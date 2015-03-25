//
//  SPTilesCanvas.cpp
//  InkscapeCore
//
//  Created by 张 光建 on 14-9-4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <libnr/nr-pixblock.h>

#include <gtk/gtk.h>

#include <gtkmm.h>

#include "helper/sp-marshal.h"
#include <helper/recthull.h>
#include <display/sp-canvas.h>
#include "display-forward.h"
#include <2geom/matrix.h>
#include <libnr/nr-convex-hull.h>
#include "preferences.h"
#include "inkscape.h"
#include "sodipodi-ctrlrect.h"
#include "cms-system.h"
#include "display/rendermode.h"
#include "libnr/nr-blit.h"
#include "display/inkscape-cairo.h"
#include "debug/gdk-event-latency-tracker.h"
#include "desktop.h"
#include "sp-namedview.h"
#include "sp-canvas-tile.h"
#include <libnr/nr-point-l.h>

using namespace Inkscape::Debug;

enum {PROP_0, PROP_VISIBLE};

inline int sp_canvas_tile_floor(int x)
{
    return (x & (~(TILE_SIZE - 1))) / TILE_SIZE;
}

inline int sp_canvas_tile_ceil(int x)
{
    return ((x + (TILE_SIZE - 1)) & (~(TILE_SIZE - 1))) / TILE_SIZE;
}

SPTilesCanvas::~SPTilesCanvas()
{
    if (_root) {
        gtk_object_unref (GTK_OBJECT (_root));
        _root = NULL;
    }
    if (tiles) {
        g_free(tiles);
    }
}

/**
 * Callback: object initialization for SPCanvas.
 */
SPTilesCanvas::SPTilesCanvas()
{
    pick_event.type = GDK_LEAVE_NOTIFY;
    pick_event.crossing.x = 0;
    pick_event.crossing.y = 0;
    
    /* Create the root item as a special case */
    _root = SP_CANVAS_ITEM (gtk_type_new (sp_canvas_group_get_type ()));
    _root->canvas = this;
    
    gtk_object_ref (GTK_OBJECT (_root));
    gtk_object_sink (GTK_OBJECT (_root));
    
    state = 0;
    need_repick = TRUE;
    in_repick = 0;
    _dirty_tiles_num = 0;
    left_grabbed_item = 0;
    
    // See comment at in sp-canvas.h.
    gen_all_enter_events = false;

    tiles=NULL;
    tLeft=tTop=tRight=tBottom=0;
    tileH=tileV=0;
    
    forced_redraw_count = 0;
    forced_redraw_limit = -1;
    
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    enable_cms_display_adj = false;
    cms_key = new Glib::ustring("");
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
}

static bool is_descendant(SPCanvasItem const *item, SPCanvasItem const *parent)
{
    while (item) {
        if (item == parent)
            return true;
        item = item->parent;
    }
    
    return false;
}

/**
 * Helper that emits an event for an item in the canvas, be it the current
 * item, grabbed item, or focused item, as appropriate.
 */
int SPTilesCanvas::emit_event(GdkEvent *event)
{
    guint mask;
    double worldx, worldy;
    
    if (grabbed_item) {
        switch (event->type) {
            case GDK_ENTER_NOTIFY:
                mask = GDK_ENTER_NOTIFY_MASK;
                break;
            case GDK_LEAVE_NOTIFY:
                mask = GDK_LEAVE_NOTIFY_MASK;
                break;
            case GDK_MOTION_NOTIFY:
                mask = GDK_POINTER_MOTION_MASK;
                break;
            case GDK_BUTTON_PRESS:
            case GDK_2BUTTON_PRESS:
            case GDK_3BUTTON_PRESS:
                mask = GDK_BUTTON_PRESS_MASK;
                break;
            case GDK_BUTTON_RELEASE:
                mask = GDK_BUTTON_RELEASE_MASK;
                break;
            case GDK_KEY_PRESS:
                mask = GDK_KEY_PRESS_MASK;
                break;
            case GDK_KEY_RELEASE:
                mask = GDK_KEY_RELEASE_MASK;
                break;
            case GDK_SCROLL:
                mask = GDK_SCROLL;
                break;
            default:
                mask = 0;
                break;
        }
        
        if (!(mask & grabbed_event_mask)) return FALSE;
    }
    
    /* Convert to world coordinates -- we have two cases because of different
     * offsets of the fields in the event structures.
     */
    
    GdkEvent *ev = gdk_event_copy(event);
    
    switch (ev->type) {
        case GDK_ENTER_NOTIFY:
        case GDK_LEAVE_NOTIFY:
            window_to_world(ev->crossing.x, ev->crossing.y, &worldx, &worldy);
            ev->crossing.x = worldx;
            ev->crossing.y = worldy;
            break;
        case GDK_MOTION_NOTIFY:
        case GDK_BUTTON_PRESS:
        case GDK_2BUTTON_PRESS:
        case GDK_3BUTTON_PRESS:
        case GDK_BUTTON_RELEASE:
            window_to_world(ev->motion.x, ev->motion.y, &worldx, &worldy);
            ev->motion.x = worldx;
            ev->motion.y = worldy;
            break;
        default:
            break;
    }
    
    /* Choose where we send the event */
    
    /* canvas->current_item becomes NULL in some cases under Win32
     ** (e.g. if the pointer leaves the window).  So this is a hack that
     ** Lauris applied to SP to get around the problem.
     */
    SPCanvasItem* item = NULL;
    if (grabbed_item && !is_descendant (current_item, grabbed_item)) {
        item = grabbed_item;
    } else {
        // Make sure that current_item is up-to-date. If a snap indicator was just deleted, then
        // sp_canvas_item_dispose has been called and there is no current_item specified. We need
        // that though because otherwise we don't know where to send this event to, leading to a
        // lost event. We can't wait for idle events to have current_item updated, we need it now!
        // Otherwise, scrolling when hovering above a pre-snap indicator won't work (for example)
        // See this bug report: https://bugs.launchpad.net/inkscape/+bug/522335/comments/8
        if (need_repick && !in_repick && event->type == GDK_SCROLL) {
            // To avoid side effects, we'll only do this for scroll events, because this is the
            // only thing we want to fix here. An example of a reported side effect is that
            // otherwise selection of nodes in the node editor by dragging a rectangle using a
            // tablet will break
            need_repick = FALSE;
            pick_current_item ((GdkEvent *) event);
        }
        item = current_item;
    }
    
    if (focused_item &&
        ((event->type == GDK_KEY_PRESS) ||
         (event->type == GDK_KEY_RELEASE) ||
         (event->type == GDK_FOCUS_CHANGE))) {
            item = focused_item;
        }
    
    /* The event is propagated up the hierarchy (for if someone connected to
     * a group instead of a leaf event), and emission is stopped if a
     * handler returns TRUE, just like for GtkWidget events.
     */
    
    gint finished = FALSE;
    int looptime = 0;
    
    while (item && !finished) {
//        g_debug("loop time is : %d", looptime);
        looptime++;
        gtk_object_ref (GTK_OBJECT (item));
        gtk_signal_emit_by_name(GTK_OBJECT (item), "event", ev, &finished);
        SPCanvasItem *parent = item->parent;
        gtk_object_unref (GTK_OBJECT (item));
        item = parent;
    }
    
    gdk_event_free(ev);
    
    return finished;
}

/**
 * Helper that re-picks the current item in the canvas, based on the event's
 * coordinates and emits enter/leave events for items as appropriate.
 */
int SPTilesCanvas::pick_current_item (GdkEvent *event)
{
    int button_down = 0;
    double x, y;
    
    if (!_root) // canvas may have already be destroyed by closing desktop durring interrupted display!
        return FALSE;
    
    int retval = FALSE;
    
    if (gen_all_enter_events == false) {
        // If a button is down, we'll perform enter and leave events on the
        // current item, but not enter on any other item.  This is more or
        // less like X pointer grabbing for canvas items.
        //
        button_down = state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK |
                               GDK_BUTTON3_MASK | GDK_BUTTON4_MASK | GDK_BUTTON5_MASK);
        
        if (!button_down) left_grabbed_item = FALSE;
    }
    
    /* Save the event in the canvas.  This is used to synthesize enter and
     * leave events in case the current item changes.  It is also used to
     * re-pick the current item if the current one gets deleted.  Also,
     * synthesize an enter event.
     */
    if (event != &pick_event) {
        if ((event->type == GDK_MOTION_NOTIFY) || (event->type == GDK_BUTTON_RELEASE)) {
            /* these fields have the same offsets in both types of events */
            
            pick_event.crossing.type       = GDK_ENTER_NOTIFY;
            pick_event.crossing.window     = event->motion.window;
            pick_event.crossing.send_event = event->motion.send_event;
            pick_event.crossing.subwindow  = NULL;
            pick_event.crossing.x          = event->motion.x;
            pick_event.crossing.y          = event->motion.y;
            pick_event.crossing.mode       = GDK_CROSSING_NORMAL;
            pick_event.crossing.detail     = GDK_NOTIFY_NONLINEAR;
            pick_event.crossing.focus      = FALSE;
            pick_event.crossing.state      = event->motion.state;
            
            /* these fields don't have the same offsets in both types of events */
            
            if (event->type == GDK_MOTION_NOTIFY) {
                pick_event.crossing.x_root = event->motion.x_root;
                pick_event.crossing.y_root = event->motion.y_root;
            } else {
                pick_event.crossing.x_root = event->button.x_root;
                pick_event.crossing.y_root = event->button.y_root;
            }
        } else {
            
            pick_event = *event;
        }
    }
    
    /* Don't do anything else if this is a recursive call */
    if (in_repick) return retval;
    
    /* LeaveNotify means that there is no current item, so we don't look for one */
    if (pick_event.type != GDK_LEAVE_NOTIFY) {
        /* these fields don't have the same offsets in both types of events */
        
        if (pick_event.type == GDK_ENTER_NOTIFY) {
            x = pick_event.crossing.x;
            y = pick_event.crossing.y;
        } else {
            x = pick_event.motion.x;
            y = pick_event.motion.y;
        }
        
        /* world coords */
        double t0, t1;
        window_to_world(x, y, &t0, &t1);
        x = t0, y = t1;
        
        /* find the closest item */
        if (_root->flags & SP_CANVAS_ITEM_VISIBLE) {
            sp_canvas_item_invoke_point (_root, Geom::Point(x, y), &new_current_item);
        } else {
            new_current_item = NULL;
        }
    } else {
        new_current_item = NULL;
    }
    
    if ((new_current_item == current_item) && !left_grabbed_item) {
        return retval; /* current item did not change */
    }
    
    /* Synthesize events for old and new current items */
    
    if ((new_current_item != current_item)
        && (current_item != NULL)
        && !left_grabbed_item) {
        GdkEvent new_event;
        SPCanvasItem *item;
        
        item = current_item;
        
        new_event = pick_event;
        new_event.type = GDK_LEAVE_NOTIFY;
        
        new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
        new_event.crossing.subwindow = NULL;
        in_repick = TRUE;
        retval = emit_event (&new_event);
        in_repick = FALSE;
    }
    
    if (gen_all_enter_events == false) {
        // new_current_item may have been set to NULL during the call to
        // emit_event() above
        if ((new_current_item != current_item) && button_down) {
            left_grabbed_item = TRUE;
            return retval;
        }
    }
    
    /* Handle the rest of cases */
    
    left_grabbed_item = FALSE;
    current_item = new_current_item;
    
    if (current_item) {
        g_debug("Current item rect [%.0f, %.0f, %.0f, %.0f]", current_item->x1, current_item->x2, current_item->y1, current_item->y2);
    } else {
        g_debug("Current item is set to NULL");
    }
    
    if (current_item != NULL) {
        GdkEvent new_event;
        
        new_event = pick_event;
        new_event.type = GDK_ENTER_NOTIFY;
        new_event.crossing.detail = GDK_NOTIFY_ANCESTOR;
        new_event.crossing.subwindow = NULL;
        retval = emit_event (&new_event);
    }
    
    return retval;
}

/**
 * Force a full redraw after a specified number of interrupted redraws
 */
void SPTilesCanvas::force_full_redraw_after_interruptions(unsigned int count)
{
    forced_redraw_limit = count;
    forced_redraw_count = 0;
}

/**
 * End forced full redraw requests
 */
void SPTilesCanvas::end_forced_full_redraws()
{
    forced_redraw_limit = -1;
}

/**
 * Helper that repaints the areas in the canvas that need it.
 *
 * @return true if all the dirty parts have been redrawn
 */
int SPTilesCanvas::paint ()
{
    if (need_update) {
        sp_canvas_item_invoke_update (_root, Geom::identity(), 0);
        need_update = FALSE;
    }
    
    Gdk::Region to_paint;
    
    for (int j=tTop; j<tBottom; j++) {
        for (int i=tLeft; i<tRight; i++) {
            int tile_index = (i - tLeft) + (j - tTop)*tileH;
            
            if ( tiles[tile_index] ) { // if this tile is dirtied (nonzero)
                to_paint.union_with_rect(Gdk::Rectangle(i*TILE_SIZE, j*TILE_SIZE,
                                                        TILE_SIZE, TILE_SIZE));
            }
            
        }
    }
    
    if (!to_paint.empty()) {
        static glong mas_elapsed = 0;
        static glong max_loop = 0;
        glong elapsed;
        GTimeVal start, tmp, now;
        bool complete = TRUE;
        
        g_get_current_time (&start);
        
        Glib::ArrayHandle<Gdk::Rectangle> rect = to_paint.get_rectangles();
        typedef Glib::ArrayHandle<Gdk::Rectangle>::const_iterator Iter;
        for (Iter i=rect.begin(); i != rect.end(); ++i) {
            int x0 = (*i).get_x();
            int y0 = (*i).get_y();
            int x1 = x0 + (*i).get_width();
            int y1 = y0 + (*i).get_height();
            
            g_get_current_time (&now);
            
            elapsed = (now.tv_sec - start.tv_sec) * 1000000 + (now.tv_usec - start.tv_usec);
            if (0/*elapsed > 5000*/) {
                complete = FALSE;
                break;
            }
            
            bool r = paint_rect(this, x0, y0, x1, y1);
            
            g_get_current_time (&tmp);
            elapsed = (tmp.tv_sec - now.tv_sec) * 1000000 + (tmp.tv_usec - now.tv_usec);
            if (elapsed > max_loop) {
                max_loop = elapsed;
            }
            
            if (!r) {
                // Aborted
                complete = FALSE;
                break;
            };
        }

        g_get_current_time (&now);
        
        elapsed = (now.tv_sec - start.tv_sec) * 1000000 + (now.tv_usec - start.tv_usec);
        if (elapsed > mas_elapsed) {
            mas_elapsed = elapsed;
        }
        g_debug("paint time consumed=%8d \tmax=%8d \t max_loop=%8d", elapsed, mas_elapsed, max_loop);
        
        if (!complete) {
            return FALSE;
        }
    }
    
    // we've had a full unaborted redraw, reset the full redraw counter
    if (forced_redraw_limit != -1) {
        forced_redraw_count = 0;
    }
    
    return TRUE;
}

/**
 * Returns true if point given in world coordinates is inside window.
 */
bool SPTilesCanvas::world_pt_inside_window(Geom::Point const &world)
{
    return ( ( integerOrigin()[NR::X] <= world[Geom::X] )  &&
            ( integerOrigin()[NR::Y] <= world[Geom::Y] )  &&
            ( world[Geom::X] < integerOrigin()[NR::X] + getViewbox().width() )  &&
            ( world[Geom::Y] < integerOrigin()[NR::Y] + getViewbox().height() ) );
}

/**
 * Sets world coordinates from win and canvas.
 */
void SPTilesCanvas::window_to_world(double winx, double winy, double *worldx, double *worldy)
{
    if (worldx) *worldx = integerOrigin()[NR::X] + winx;
    if (worldy) *worldy = integerOrigin()[NR::Y] + winy;
}

/**
 * Sets win coordinates from world and canvas.
 */
void SPTilesCanvas::world_to_window(double worldx, double worldy, double *winx, double *winy)
{
    if (winx) *winx = worldx - integerOrigin()[NR::X];
    if (winy) *winy = worldy - integerOrigin()[NR::Y];
}

/**
 * Converts point from win to world coordinates.
 */
Geom::Point SPTilesCanvas::window_to_world(Geom::Point const win)
{
    return Geom::Point(integerOrigin()[NR::X] + win[0], integerOrigin()[NR::Y] + win[1]);
}

/**
 * Converts point from world to win coordinates.
 */
Geom::Point SPTilesCanvas::world_to_window(Geom::Point const world)
{
    return Geom::Point(world[0] - integerOrigin()[NR::X], world[1] - integerOrigin()[NR::Y]);
}

NR::IPoint SPTilesCanvas::integerOrigin()
{
    Geom::Rect r = getViewbox();
    return NR::IPoint(round(r.left()), round(r.top()));
}

/**
 * Helper that allocates a new tile array for the canvas, copying overlapping tiles from the old array
 */
void SPTilesCanvas:: resize_tiles(int nl, int nt, int nr, int nb)
{
    _dirty_tiles_num = 0;
    
    if ( nl >= nr || nt >= nb ) {
        if ( tiles ) g_free(tiles);
        tLeft=tTop=tRight=tBottom=0;
        tileH=tileV=0;
        tiles=NULL;
        return;
    }
    int tl=sp_canvas_tile_floor(nl);
    int tt=sp_canvas_tile_floor(nt);
    int tr=sp_canvas_tile_ceil(nr);
    int tb=sp_canvas_tile_ceil(nb);
    
    int nh = tr-tl, nv = tb-tt;
    uint8_t* ntiles = (uint8_t*)g_malloc(nh*nv*sizeof(uint8_t));
    for (int i=tl; i<tr; i++) {
        for (int j=tt; j<tb; j++) {
            int ind = (i-tl) + (j-tt)*nh;
            if ( i >= tLeft && i < tRight && j >= tTop && j < tBottom ) {
                ntiles[ind]=tiles[(i-tLeft)+(j-tTop)*tileH]; // copy from the old tile
                _dirty_tiles_num += ntiles[ind] > 0 ? 1 : 0;
            } else {
                ntiles[ind]=0; // newly exposed areas get 0
                _dirty_tiles_num += ntiles[ind] > 0 ? 1 : 0;
            }
        }
    }
    if ( tiles ) g_free(tiles);
    tiles=ntiles;
    tLeft=tl;
    tTop=tt;
    tRight=tr;
    tBottom=tb;
    tileH=nh;
    tileV=nv;
    
    g_debug("dirty tiles after resize : %d", _dirty_tiles_num);
}

/*
 * Helper that queues a canvas rectangle for redraw
 */
void SPTilesCanvas::dirty_rect(int nl, int nt, int nr, int nb) {
//    g_debug("dirty rect : %d %d %d %d", nl, nt, nr, nb);
    mark_rect(nl, nt, nr, nb, 1);
}


/**
 * Helper that marks specific canvas rectangle as clean (val == 0) or dirty (otherwise)
 */
void SPTilesCanvas::mark_rect(int nl, int nt, int nr, int nb, uint8_t val)
{
    if ( nl >= nr || nt >= nb ) {
//        g_warning("can't mark a negtive rect: %d %d %d %d", nl, nt, nr, nb);
//        return;
    }
    int tl=sp_canvas_tile_floor(nl);
    int tt=sp_canvas_tile_floor(nt);
    int tr=sp_canvas_tile_ceil(nr);
    int tb=sp_canvas_tile_ceil(nb);
    if ( tl >= tRight || tr <= tLeft || tt >= tBottom || tb <= tTop ) return;
    if ( tl < tLeft ) tl=tLeft;
    if ( tr > tRight ) tr=tRight;
    if ( tt < tTop ) tt=tTop;
    if ( tb > tBottom ) tb=tBottom;
    
    for (int i=tl; i<tr; i++) {
        for (int j=tt; j<tb; j++) {
            int index = (i-tLeft)+(j-tTop)*tileH;
            uint8_t old = tiles[index];
            tiles[index] = val;
            _dirty_tiles_num += val - old;
        }
    }
    
    g_debug("dirty tiles rest : %d", _dirty_tiles_num);
}
