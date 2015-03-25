//
//  SPGtkCanvas.cpp
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
#include <glib.h>
#include <gtkmm.h>
#include <gtkmm/widget.h>
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
#include "sp-canvas-gtk.h"
#include "sp-canvas.h"

//#define DEBUG_REDRAW    (1)

// GTK_CHECK_VERSION returns false on failure
#define HAS_GDK_EVENT_REQUEST_MOTIONS GTK_CHECK_VERSION(2, 12, 0)

// gtk_check_version returns non-NULL on failure
static bool const HAS_BROKEN_MOTION_HINTS =
true || gtk_check_version(2, 12, 0) != NULL || !HAS_GDK_EVENT_REQUEST_MOTIONS;

static gint const sp_canvas_update_priority = G_PRIORITY_HIGH_IDLE;


class SPGtkCanvasWidget : public Gtk::Widget
{
public:
    SPGtkCanvasWidget (SPGtkCanvas *canvas) : _canvas(canvas) {};
    
    void on_realize() {
        if (_canvas) _canvas->on_realize();
        bool b = GDK_IS_WINDOW(SP_GTK_CANVAS_WINDOW(_canvas));
        g_debug("gdk window is %s created", b ? "" : "not");
    }
    void on_unrealize() { if (_canvas) _canvas->on_unrealize(); }
    
    void on_size_request(Gtk::Requisition* requisition) { if (_canvas) _canvas->on_size_request(requisition); }
    void on_size_allocate(Gtk::Allocation& allocation) { if (_canvas) _canvas->on_size_allocate(allocation); }
    
    bool on_button_press_event(GdkEventButton* event) { if (_canvas) return _canvas->on_button_press_event(event); return false;}
    bool on_button_release_event(GdkEventButton* event) { if (_canvas) return _canvas->on_button_release_event(event); return false; }
    
    bool on_scroll_event(GdkEventScroll* event) { if (_canvas) return _canvas->on_scroll_event(event); return false; }
    bool on_motion_notify_event(GdkEventMotion* event) { if (_canvas) return _canvas->on_motion_notify_event(event);  return false;}
    bool on_expose_event(GdkEventExpose* event) { if (_canvas) return _canvas->on_expose_event(event);  return false;}
    bool on_key_press_event(GdkEventKey* event) { if (_canvas) return _canvas->on_key_press_event(event);  return false;}
    bool on_key_release_event(GdkEventKey* event) { if (_canvas) return _canvas->on_key_release_event(event);  return false;}
    bool on_enter_notify_event(GdkEventCrossing* event) { if (_canvas) return _canvas->on_enter_notify_event(event);  return false;}
    bool on_leave_notify_event(GdkEventCrossing* event) { if (_canvas) return _canvas->on_leave_notify_event(event);  return false;}
    bool on_focus_in_event(GdkEventFocus* event) { if (_canvas) return _canvas->on_focus_in_event(event);  return false;}
    bool on_focus_out_event(GdkEventFocus* event) { if (_canvas) return _canvas->on_focus_out_event(event);  return false;}
    
    SPGtkCanvas *_canvas;
};

SPGtkCanvas::SPGtkCanvas()
{
    _widget = new SPGtkCanvasWidget(this);
    is_scrolling = FALSE;
    _pixmap_gc = NULL;
}

SPGtkCanvas::~SPGtkCanvas()
{
    if (_widget) {
        delete _widget;
    }
}

struct PaintRectSetup {
    SPGtkCanvas* canvas;
    NRRectL big_rect;
    GTimeVal start_time;
    int max_pixels;
    Geom::Point mouse_loc;
};

void SPGtkCanvas::paint_single_buffer (int x0, int y0, int x1, int y1, int draw_x1, int draw_y1, int draw_x2, int draw_y2, int sw)
{
    GtkWidget *widget = SP_GTK_CANVAS_WIDGET(this);
    SPGtkCanvas *canvas = this;
    SPCanvasBuf buf;
    
    if (canvas->rendermode != Inkscape::RENDERMODE_OUTLINE) {
        buf.buf = nr_pixelstore_256K_new (FALSE, 0);
    } else {
        buf.buf = nr_pixelstore_1M_new (FALSE, 0);
    }
    
    // Mark the region clean
    mark_rect(x0, y0, x1, y1, 0);
    
    buf.buf_rowstride = sw * 4;
    buf.rect.x0 = x0;
    buf.rect.y0 = y0;
    buf.rect.x1 = x1;
    buf.rect.y1 = y1;
    buf.visible_rect.x0 = draw_x1;
    buf.visible_rect.y0 = draw_y1;
    buf.visible_rect.x1 = draw_x2;
    buf.visible_rect.y1 = draw_y2;
    GdkColor *color = &widget->style->bg[GTK_STATE_NORMAL];
    buf.bg_color = (((color->red & 0xff00) << 8)
                    | (color->green & 0xff00)
                    | (color->blue >> 8));
    buf.is_empty = true;
    
    buf.ct = nr_create_cairo_context_canvasbuf (&(buf.visible_rect), &buf);
    
    if (SP_CANVAS_ITEM(canvas->root())->flags & SP_CANVAS_ITEM_VISIBLE) {
        SP_CANVAS_ITEM_GET_CLASS (canvas->root())->render (SP_CANVAS_ITEM(canvas->root()), &buf);
    }
    
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    cmsHTRANSFORM transf = 0;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool fromDisplay = prefs->getBool( "/options/displayprofile/from_display");
    if ( fromDisplay ) {
        transf = Inkscape::CMSSystem::getDisplayPer( *(canvas->cms_key) );
    } else {
        transf = Inkscape::CMSSystem::getDisplayTransform();
    }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    
    if (buf.is_empty) {
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        if ( transf && canvas->enable_cms_display_adj ) {
            Inkscape::CMSSystem::doTransform(transf, &buf.bg_color, &buf.bg_color, 1);
        }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        gdk_rgb_gc_set_foreground (canvas->_pixmap_gc, buf.bg_color);
        gdk_draw_rectangle (SP_GTK_CANVAS_WINDOW (canvas),
                            canvas->_pixmap_gc,
                            TRUE,
                            x0 - canvas->_iorg[NR::X], y0 - canvas->_iorg[NR::Y],
                            x1 - x0, y1 - y0);
    } else {
        
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        if ( transf && canvas->enable_cms_display_adj ) {
            for ( gint yy = 0; yy < (y1 - y0); yy++ ) {
                guchar* p = buf.buf + (buf.buf_rowstride * yy);
                Inkscape::CMSSystem::doTransform( transf, p, p, (x1 - x0) );
            }
        }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        
        // Now we only need to output the prepared pixmap to the actual screen, and this define chooses one
        // of the two ways to do it. The cairo way is direct and straightforward, but unfortunately
        // noticeably slower. I asked Carl Worth but he was unable so far to suggest any specific reason
        // for this slowness. So, for now we use the oldish method: squeeze out 32bpp buffer to 24bpp and
        // use gdk_draw_rgb_image_dithalign, for unfortunately gdk can only handle 24 bpp, which cairo
        // cannot handle at all. Still, this way is currently faster even despite the blit with squeeze.
        
        ///#define CANVAS_OUTPUT_VIA_CAIRO
        
#ifdef CANVAS_OUTPUT_VIA_CAIRO
        
        buf.cst = cairo_image_surface_create_for_data (
                                                       buf.buf,
                                                       CAIRO_FORMAT_ARGB32,  // unpacked, i.e. 32 bits! one byte is unused
                                                       _x1 - _x0, _y1 - _y0,
                                                       buf.buf_rowstride
                                                       );
        
        cairo_t *window_ct = gdk_cairo_create(SP_GTK_CANVAS_WINDOW (canvas));
        cairo_set_source_surface (window_ct, buf.cst, _x0 - canvas->x0, _y0 - canvas->y0);
        cairo_paint (window_ct);
        cairo_destroy (window_ct);
        cairo_surface_finish (buf.cst);
        cairo_surface_destroy (buf.cst);
#else
        
        NRPixBlock b3;
        nr_pixblock_setup_fast (&b3, NR_PIXBLOCK_MODE_R8G8B8, x0, y0, x1, y1, TRUE);
        
        NRPixBlock b4;
        nr_pixblock_setup_extern (&b4, NR_PIXBLOCK_MODE_R8G8B8A8P, x0, y0, x1, y1,
                                  buf.buf,
                                  buf.buf_rowstride,
                                  FALSE, FALSE);
        
        // this does the 32->24 squishing, using an assembler routine:
        nr_blit_pixblock_pixblock (&b3, &b4);
        
        gdk_draw_rgb_image_dithalign (SP_GTK_CANVAS_WINDOW (canvas),
                                      canvas->_pixmap_gc,
                                      x0 - canvas->_iorg[NR::X], y0 - canvas->_iorg[NR::Y],
                                      x1 - x0, y1 - y0,
                                      GDK_RGB_DITHER_MAX,
                                      NR_PIXBLOCK_PX(&b3),
                                      sw * 3,
                                      x0 - canvas->_iorg[NR::X], y0 - canvas->_iorg[NR::Y]);
        
        nr_pixblock_release (&b3);
        nr_pixblock_release (&b4);
#endif
    }
    
    cairo_surface_t *cst = cairo_get_target(buf.ct);
    cairo_destroy (buf.ct);
    cairo_surface_finish (cst);
    cairo_surface_destroy (cst);
    
    if (canvas->rendermode != Inkscape::RENDERMODE_OUTLINE) {
        nr_pixelstore_256K_free (buf.buf);
    } else {
        nr_pixelstore_1M_free (buf.buf);
    }
}

/**
 * Paint the given rect, recursively subdividing the region until it is the size of a single
 * buffer.
 *
 * @return true if the drawing completes
 */
int SPGtkCanvas::paint_rect_internal (PaintRectSetup const *setup, NRRectL this_rect)
{
    GTimeVal now;
    g_get_current_time (&now);
    
    glong elapsed = (now.tv_sec - setup->start_time.tv_sec) * 1000000
    + (now.tv_usec - setup->start_time.tv_usec);
    
    // Allow only very fast buffers to be run together;
    // as soon as the total redraw time exceeds 1ms, cancel;
    // this returns control to the idle loop and allows Inkscape to process user input
    // (potentially interrupting the redraw); as soon as Inkscape has some more idle time,
    // it will get back and finish painting what remains to paint.
    if (elapsed > 1000) {
        
        // Interrupting redraw isn't always good.
        // For example, when you drag one node of a big path, only the buffer containing
        // the mouse cursor will be redrawn again and again, and the rest of the path
        // will remain stale because Inkscape never has enough idle time to redraw all
        // of the screen. To work around this, such operations set a forced_redraw_limit > 0.
        // If this limit is set, and if we have aborted redraw more times than is allowed,
        // interrupting is blocked and we're forced to redraw full screen once
        // (after which we can again interrupt forced_redraw_limit times).
        if (setup->canvas->forced_redraw_limit < 0 ||
            setup->canvas->forced_redraw_count < setup->canvas->forced_redraw_limit) {
            
            if (setup->canvas->forced_redraw_limit != -1) {
                setup->canvas->forced_redraw_count++;
            }
            
            return false;
        }
    }
    
    // Find the optimal buffer dimensions
    int bw = this_rect.x1 - this_rect.x0;
    int bh = this_rect.y1 - this_rect.y0;
    if ((bw < 1) || (bh < 1))
        return 0;
    
    if (bw * bh < setup->max_pixels) {
        // We are small enough
        paint_single_buffer (this_rect.x0, this_rect.y0,
                             this_rect.x1, this_rect.y1,
                             setup->big_rect.x0, setup->big_rect.y0,
                             setup->big_rect.x1, setup->big_rect.y1, bw);
        return 1;
    }
    
    NRRectL lo = this_rect;
    NRRectL hi = this_rect;
    
    /*
     This test determines the redraw strategy:
     
     bw < bh (strips mode) splits across the smaller dimension of the rect and therefore (on
     horizontally-stretched windows) results in redrawing in horizontal strips (from cursor point, in
     both directions if the cursor is in the middle). This is traditional for Inkscape since old days,
     and seems to be faster for drawings with many smaller objects at zoom-out.
     
     bw > bh (chunks mode) splits across the larger dimension of the rect and therefore paints in
     almost-square chunks, again from the cursor point. It's sometimes faster for drawings with few slow
     (e.g. blurred) objects crossing the entire screen. It also appears to be somewhat psychologically
     faster.
     
     The default for now is the strips mode.
     */
    if (bw < bh || bh < 2 * TILE_SIZE) {
        // to correctly calculate the mean of two ints, we need to sum them into a larger int type
        gint32 mid = (gint32)((long long) this_rect.x0 + (long long) this_rect.x1) / 2;
        // Make sure that mid lies on a tile boundary
        mid = (mid / TILE_SIZE) * TILE_SIZE;
        
        lo.x1 = mid;
        hi.x0 = mid;
        
        if (setup->mouse_loc[Geom::X] < mid) {
            // Always paint towards the mouse first
            return paint_rect_internal(setup, lo)
            && paint_rect_internal(setup, hi);
        } else {
            return paint_rect_internal(setup, hi)
            && paint_rect_internal(setup, lo);
        }
    } else {
        // to correctly calculate the mean of two ints, we need to sum them into a larger int type
        gint32 mid = (gint32)((long long) this_rect.y0 + (long long) this_rect.y1) / 2;
        // Make sure that mid lies on a tile boundary
        mid = (mid / TILE_SIZE) * TILE_SIZE;
        
        lo.y1 = mid;
        hi.y0 = mid;
        
        if (setup->mouse_loc[Geom::Y] < mid) {
            // Always paint towards the mouse first
            return paint_rect_internal(setup, lo)
            && paint_rect_internal(setup, hi);
        } else {
            return paint_rect_internal(setup, hi)
            && paint_rect_internal(setup, lo);
        }
    }
}


bool SPGtkCanvas::paint_rect(SPTilesCanvas */*canvas*/, int xx0, int yy0, int xx1, int yy1)
{
    g_return_val_if_fail (!need_update, false);
    
    NRRectL rect;
    rect.x0 = xx0;
    rect.x1 = xx1;
    rect.y0 = yy0;
    rect.y1 = yy1;
    
    // Clip rect-to-draw by the current visible area
    rect.x0 = MAX (rect.x0, _iorg[NR::X]);
    rect.y0 = MAX (rect.y0, _iorg[NR::Y]);
    rect.x1 = MIN (rect.x1, _iorg[NR::X]/*draw_x1*/ + SP_GTK_CANVAS_WIDGET(this)->allocation.width);
    rect.y1 = MIN (rect.y1, _iorg[NR::Y]/*draw_y1*/ + SP_GTK_CANVAS_WIDGET(this)->allocation.height);
    
#ifdef DEBUG_REDRAW
    // paint the area to redraw yellow
    gdk_rgb_gc_set_foreground (_pixmap_gc, 0xFFFF00);
    gdk_draw_rectangle (SP_GTK_CANVAS_WINDOW (this),
                        _pixmap_gc,
                        TRUE,
                        rect.x0, rect.y0,
                        rect.x1 - rect.x0, rect.y1 - rect.y0);
#endif
    
    PaintRectSetup setup;
    
    setup.canvas = this;
    setup.big_rect = rect;
    
    // Save the mouse location
    gint x, y;
    gdk_window_get_pointer (SP_GTK_CANVAS_WINDOW(this), &x, &y, NULL);
    setup.mouse_loc = window_to_world (Geom::Point(x,y));
    
    if (rendermode != Inkscape::RENDERMODE_OUTLINE) {
        // use 256K as a compromise to not slow down gradients
        // 256K is the cached buffer and we need 4 channels
        setup.max_pixels = 65536; // 256K/4
    } else {
        // paths only, so 1M works faster
        // 1M is the cached buffer and we need 4 channels
        setup.max_pixels = 262144;
    }
    
    // Start the clock
    g_get_current_time(&(setup.start_time));
    
    // Go
    return paint_rect_internal(&setup, rect);
}

void SPGtkCanvas::scroll_to(double cx, double cy, unsigned int clear, bool is_scrolling)
{
    int ix = (int) round(cx); // ix and iy are the new canvas coordinates (integer screen pixels)
    int iy = (int) round(cy); // cx might be negative, so (int)(cx + 0.5) will not do!
    int dx = ix - _iorg[NR::X]; // dx and dy specify the displacement (scroll) of the
    int dy = iy - _iorg[NR::Y]; // canvas w.r.t its previous position
    
    _dorg[NR::X] = cx; // here the 'd' stands for double, not delta!
    _dorg[NR::Y] = cy;
    _iorg[NR::X] = ix;
    _iorg[NR::Y] = iy;
    
    resize_tiles (_iorg[NR::X], _iorg[NR::Y], _iorg[NR::X]+_widget->get_allocation().get_width(), _iorg[NR::Y]+_widget->get_allocation().get_height());
    
    if (!clear) {
        // scrolling without zoom; redraw only the newly exposed areas
        if ((dx != 0) || (dy != 0)) {
            this->is_scrolling = is_scrolling;
            if (_widget->is_realized()) {
                gdk_window_scroll (SP_GTK_CANVAS_WINDOW(this), -dx, -dy);
            }
        }
    } else {
        // scrolling as part of zoom; do nothing here - the next do_update will perform full redraw
    }
}

void SPGtkCanvas::update_now()
{
    if (!(need_update || needs_redraw()))
        return;
    
    do_update ();
}

int SPGtkCanvas::do_update()
{
    if (!root() || !_pixmap_gc) // canvas may have already be destroyed by closing desktop during interrupted display!
        return TRUE;
    
    /* Cause the update if necessary */
    if (need_update) {
        sp_canvas_item_invoke_update ((SPCanvasItem*)root(), Geom::identity(), 0);
        need_update = FALSE;
    }
    
    /* Paint if able to */
    return paint ();
}

void SPGtkCanvas::request_redraw(int x0, int y0, int x1, int y1)
{
    NRRectL bbox;
    NRRectL visible;
    NRRectL clip;
    
    if ((x0 >= x1) || (y0 >= y1)) return;
    
    bbox.x0 = x0;
    bbox.y0 = y0;
    bbox.x1 = x1;
    bbox.y1 = y1;
    
    visible.x0 = _iorg[NR::X];
    visible.y0 = _iorg[NR::Y];
    visible.x1 = visible.x0 + SP_GTK_CANVAS_WIDGET(this)->allocation.width;
    visible.y1 = visible.y0 + SP_GTK_CANVAS_WIDGET(this)->allocation.height;
    
    nr_rect_l_intersect (&clip, &bbox, &visible);
    
    dirty_rect(clip.x0, clip.y0, clip.x1, clip.y1);
    add_idle ();
}

Geom::Rect SPGtkCanvas::getViewbox() const
{
    GtkWidget const *w = SP_GTK_CANVAS_WIDGET(this);
    return Geom::Rect(Geom::Point(_dorg[NR::X], _dorg[NR::Y]),
                      Geom::Point(_dorg[NR::X] + w->allocation.width, _dorg[NR::Y] + w->allocation.height));
}

void SPGtkCanvas::on_realize()
{
    GtkWidget *widget = SP_GTK_CANVAS_WIDGET(this);
    
    GdkWindowAttr attributes;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gdk_rgb_get_visual ();
    attributes.colormap = gdk_rgb_get_cmap ();
    attributes.event_mask = (gtk_widget_get_events (widget) |
                             GDK_EXPOSURE_MASK |
                             GDK_BUTTON_PRESS_MASK |
                             GDK_BUTTON_RELEASE_MASK |
                             GDK_POINTER_MOTION_MASK |
                             ( HAS_BROKEN_MOTION_HINTS ?
                              0 : GDK_POINTER_MOTION_HINT_MASK ) |
                             GDK_PROXIMITY_IN_MASK |
                             GDK_PROXIMITY_OUT_MASK |
                             GDK_KEY_PRESS_MASK |
                             GDK_KEY_RELEASE_MASK |
                             GDK_ENTER_NOTIFY_MASK |
                             GDK_LEAVE_NOTIFY_MASK |
                             GDK_FOCUS_CHANGE_MASK);
    gint attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
    
    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if ( prefs->getBool("/options/useextinput/value", true) )
        gtk_widget_set_events(widget, attributes.event_mask);
    
    widget->style = gtk_style_attach (widget->style, widget->window);
    
    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
    
    _pixmap_gc = gdk_gc_new (widget->window);
}

/**
 * Convenience function to remove the idle handler of a canvas.
 */
void SPGtkCanvas::remove_idle ()
{
    if (_idle_id) {
        gtk_idle_remove (_idle_id);
        _idle_id = 0;
    }
}

void SPGtkCanvas::add_idle ()
{
    if (_idle_id != 0)
        return;
    
    _idle_id = gtk_idle_add_priority (sp_canvas_update_priority, SPGtkCanvas::idle_handler, this);
}

/**
 * Idle handler for the canvas that deals with pending updates and redraws.
 */
gint SPGtkCanvas::idle_handler (void *data)
{
    GDK_THREADS_ENTER ();
    
    SPGtkCanvas *canvas = (SPGtkCanvas *)data;
    
    int const ret = canvas->do_update ();
    
    if (ret) {
        /* Reset idle id */
        canvas->_idle_id = 0;
    }
    
    GDK_THREADS_LEAVE ();
    
    return !ret;
}

/*
 * Removes the transient state of the canvas (idle handler, grabs).
 */
void SPGtkCanvas::shutdown_transients ()
{
    /* We turn off the need_redraw flag, since if the canvas is mapped again
     * it will request a redraw anyways.  We do not turn off the need_update
     * flag, though, because updates are not queued when the canvas remaps
     * itself.
     */
    mark_rect(_iorg[NR::X], _iorg[NR::Y], _widget->get_allocation().get_width(), _widget->get_allocation().get_height(), 0);
    
    if ( tiles ) g_free(tiles);
    tiles=NULL;
    tLeft=tTop=tRight=tBottom=0;
    tileH=tileV=0;
    
    if (grabbed_item) {
        grabbed_item = NULL;
        gdk_pointer_ungrab (GDK_CURRENT_TIME);
    }
    
    remove_idle ();
}

void SPGtkCanvas::on_unrealize()
{
    SPGtkCanvas *canvas = this;
    
    canvas->current_item = NULL;
    canvas->grabbed_item = NULL;
    canvas->focused_item = NULL;
    
    shutdown_transients ();
    
    gdk_gc_destroy (canvas->_pixmap_gc);
    canvas->_pixmap_gc = NULL;
}

void SPGtkCanvas::on_size_request(Gtk::Requisition* requisition)
{
    requisition->width = 256;
    requisition->height = 256;
}

void SPGtkCanvas::on_size_allocate(Gtk::Allocation& allocation)
{
    /* Schedule redraw of new region */
    resize_tiles(_iorg[NR::X],_iorg[NR::Y],_iorg[NR::X]+allocation.get_width(),_iorg[NR::Y]+allocation.get_height());
    if (allocation.get_width() > _widget->get_allocation().get_width()) {
        request_redraw (_iorg[NR::X] + _widget->get_allocation().get_width(),
                        0,
                        _iorg[NR::X] + allocation.get_width(),
                        _iorg[NR::Y] + allocation.get_height());
    }
    if (allocation.get_height() > _widget->get_allocation().get_height()) {
        request_redraw (0,
                        _iorg[NR::Y] + _widget->get_allocation().get_height(),
                        _iorg[NR::X] + allocation.get_width(),
                        _iorg[NR::Y] + allocation.get_height());
    }
    
    _widget->set_allocation(allocation);
    
    if (_widget->is_realized()) {
        _widget->get_window()->move_resize(_widget->get_allocation().get_x(),
                                           _widget->get_allocation().get_y(),
                                           _widget->get_allocation().get_width(),
                                           _widget->get_allocation().get_height());
    }
}

/**
 * Button event handler for the canvas.
 */
gint SPGtkCanvas::do_button_event (GdkEventButton *event)
{
    SPGtkCanvas *canvas = this;
    
    int retval = FALSE;
    
    /* dispatch normally regardless of the event's window if an item
     has a pointer grab in effect */
    if (!canvas->grabbed_item &&
        event->window != SP_GTK_CANVAS_WINDOW (canvas))
        return retval;
    
    int mask;
    switch (event->button) {
        case 1:
            mask = GDK_BUTTON1_MASK;
            break;
        case 2:
            mask = GDK_BUTTON2_MASK;
            break;
        case 3:
            mask = GDK_BUTTON3_MASK;
            break;
        case 4:
            mask = GDK_BUTTON4_MASK;
            break;
        case 5:
            mask = GDK_BUTTON5_MASK;
            break;
        default:
            mask = 0;
    }
    
    switch (event->type) {
        case GDK_BUTTON_PRESS:
        case GDK_2BUTTON_PRESS:
        case GDK_3BUTTON_PRESS:
            /* Pick the current item as if the button were not pressed, and
             * then process the event.
             */
            canvas->state = event->state;
            pick_current_item ((GdkEvent *) event);
            canvas->state ^= mask;
            retval = emit_event ((GdkEvent *) event);
            break;
            
        case GDK_BUTTON_RELEASE:
            /* Process the event as if the button were pressed, then repick
             * after the button has been released
             */
            canvas->state = event->state;
            retval = emit_event ((GdkEvent *) event);
            event->state ^= mask;
            canvas->state = event->state;
            pick_current_item ((GdkEvent *) event);
            event->state ^= mask;
            
            break;
            
        default:
            g_assert_not_reached ();
    }
    
    return retval;
}

bool SPGtkCanvas::on_button_press_event(GdkEventButton* event)
{
    return do_button_event(event);
}

bool SPGtkCanvas::on_button_release_event(GdkEventButton* event)
{
    return do_button_event(event);
}

bool SPGtkCanvas::on_scroll_event(GdkEventScroll* event)
{
    return emit_event ((GdkEvent *) event);
}

static void track_latency(GdkEvent const *event) {
    Inkscape::Debug::GdkEventLatencyTracker &tracker = Inkscape::Debug::GdkEventLatencyTracker::default_tracker();
    boost::optional<double> latency = tracker.process(event);
    if (latency && *latency > 2.0) {
        //g_warning("Event latency reached %f sec (%1.4f)", *latency, tracker.getSkew());
    }
}

static inline void request_motions(GdkWindow *w, GdkEventMotion *event) {
    gdk_window_get_pointer(w, NULL, NULL, NULL);
#if HAS_GDK_EVENT_REQUEST_MOTIONS
    gdk_event_request_motions(event);
#endif
}

bool SPGtkCanvas::on_motion_notify_event(GdkEventMotion* event)
{
    int status;
    SPGtkCanvas *canvas = this;
    
    track_latency((GdkEvent *)event);
    
    if (event->window != SP_GTK_CANVAS_WIDGET (canvas)->window)
        return FALSE;
    
    if (canvas->_pixmap_gc == NULL) // canvas being deleted
        return FALSE;
    
    canvas->state = event->state;
    pick_current_item ((GdkEvent *) event);
    status = emit_event ((GdkEvent *) event);
    if (event->is_hint) {
        request_motions(SP_GTK_CANVAS_WINDOW(this), event);
    }
    
    return status;
}

bool SPGtkCanvas::on_expose_event(GdkEventExpose* event)
{
    SPGtkCanvas *canvas = this;
    
    if ((event->window != SP_GTK_CANVAS_WINDOW (canvas)))
        return FALSE;
    
    int n_rects;
    GdkRectangle *rects;
    gdk_region_get_rectangles (event->region, &rects, &n_rects);
    
    for (int i = 0; i < n_rects; i++) {
        NRRectL rect;
        
        rect.x0 = rects[i].x + canvas->_iorg[NR::X];
        rect.y0 = rects[i].y + canvas->_iorg[NR::Y];
        rect.x1 = rect.x0 + rects[i].width;
        rect.y1 = rect.y0 + rects[i].height;
        
        request_redraw (rect.x0, rect.y0, rect.x1, rect.y1);
    }
    
    if (n_rects > 0)
        g_free (rects);
    
    return FALSE;
}

gint SPGtkCanvas::do_key_event(GdkEventKey *event)
{
    return emit_event ((GdkEvent *) event);
}

bool SPGtkCanvas::on_key_press_event(GdkEventKey* event)
{
    return do_key_event(event);
}

bool SPGtkCanvas::on_key_release_event(GdkEventKey* event)
{
    return do_key_event(event);
}

/**
 * Crossing event handler for the canvas.
 */
gint SPGtkCanvas::do_crossing_event (GdkEventCrossing *event)
{
    SPGtkCanvas *canvas = this;
    
    if (event->window != SP_GTK_CANVAS_WINDOW(this))
        return FALSE;
    
    canvas->state = event->state;
    return pick_current_item ((GdkEvent *) event);
}

bool SPGtkCanvas::on_enter_notify_event(GdkEventCrossing* event)
{
    return do_crossing_event(event);
}

bool SPGtkCanvas::on_leave_notify_event(GdkEventCrossing* event)
{
    return do_crossing_event(event);
}

bool SPGtkCanvas::on_focus_in_event(GdkEventFocus* event)
{
    GTK_WIDGET_SET_FLAGS (SP_GTK_CANVAS_WIDGET(this), GTK_HAS_FOCUS);
    
    SPGtkCanvas *canvas = this;
    
    if (canvas->focused_item) {
        return emit_event ((GdkEvent *) event);
    } else {
        return FALSE;
    }
}

bool SPGtkCanvas::on_focus_out_event(GdkEventFocus* event)
{
    GTK_WIDGET_UNSET_FLAGS (SP_GTK_CANVAS_WIDGET(this), GTK_HAS_FOCUS);
    
    SPGtkCanvas *canvas = this;
    
    if (canvas->focused_item)
        return emit_event ((GdkEvent *) event);
    else
        return FALSE;
}

/**
 * Update callback for canvas widget.
 */
void SPGtkCanvas::request_update ()
{
    need_update = TRUE;
    add_idle ();
}

/**
 * Return canvas window coordinates as IRect (a rectangle defined by integers).
 */
NR::IRect SPGtkCanvas::getViewboxIntegers() const
{
    GtkWidget const *w = SP_GTK_CANVAS_WIDGET(this);
    return NR::IRect(_iorg, 
                     NR::IPoint(_iorg[NR::X] + w->allocation.width, _iorg[NR::Y] + w->allocation.height));
}

#if 0
static GObjectClass *parent_class = NULL;

/**
 * Callback: object initialization for SPCanvas.
 */
static void sp_gtk_canvas_init (SPGtkCanvas *canvas)
{
    canvas->init();
}

static void sp_gtk_canvas_finalize (GObject *object)
{
    SPGtkCanvas *canvas = (SPGtkCanvas *)object;
//    canvas->shutdown_transients ();
    canvas->destroy();
    if (parent_class->finalize) {
        parent_class->finalize(G_OBJECT(canvas));
    }
}

/**
 * Initializes the SPObject vtable.
 */
static void
sp_object_class_init(SPGtkCanvas *klass)
{
    GObjectClass *object_class;
    object_class = (GObjectClass *) klass;
    parent_class = (GObjectClass *) g_type_class_ref(GTK_TYPE_WIDGET);
    object_class->finalize = sp_gtk_canvas_finalize;
}

/**
 * Registers the SPCanvas class if necessary, and returns the type ID
 * associated to it.
 *
 * \return The type ID of the SPCanvas class.
 **/
GType sp_gtk_canvas_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPGtkCanvasClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_object_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPGtkCanvas),
            0, // n_preallocs
            (GInstanceInitFunc)sp_gtk_canvas_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_WIDGET, "SPGtkCanvas", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

SPGtkCanvas *sp_gtk_canvas_new()
{
    SPGtkCanvas *canvas = (SPGtkCanvas *)gtk_type_new(sp_gtk_canvas_get_type());
    canvas->init();
    return canvas;
}
#endif
