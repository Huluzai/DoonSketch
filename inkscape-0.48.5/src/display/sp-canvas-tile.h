//
//  SPTilesCanvas.h
//  InkscapeCore
//
//  Created by 张 光建 on 14-9-4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#ifndef __InkscapeCore__SPTilesCanvas__
#define __InkscapeCore__SPTilesCanvas__

#include "canvas-base.h"
#include <gtk/gtk.h>
#include "sp-canvas.h"
#include "display-forward.h"
#include <libnr/nr-point-l.h>

// Tiles are a way to minimize the number of redraws, eliminating too small redraws.
// The canvas stores a 2D array of ints, each representing a TILE_SIZExTILE_SIZE pixels tile.
// If any part of it is dirtied, the entire tile is dirtied (its int is nonzero) and repainted.
#define TILE_SIZE 16

struct SPTilesCanvas : public CanvasBase
{
    SPTilesCanvas();
    virtual ~SPTilesCanvas();
    
    void force_full_redraw_after_interruptions(unsigned int count);
    void end_forced_full_redraws();
    
    virtual int paint ();
    virtual bool paint_rect (SPTilesCanvas *canvas, int xx0, int yy0, int xx1, int yy1) {return false;}
    
    SPCanvasGroup * root () {return SP_CANVAS_GROUP(_root);}
    
    void dirty_rect(int nl, int nt, int nr, int nb);
    bool needs_redraw() const {return tiles ? _dirty_tiles_num > 0 : false;}
    
    //
    virtual void scroll_to(double cx, double cy, unsigned int clear, bool is_scrolling = false) {}
    virtual void update_now() {}
    
    virtual void request_update () {}
    
    /**
     * Returns true if point given in world coordinates is inside window.
     */
    virtual bool world_pt_inside_window(Geom::Point const &world);
    
    /**
     * Sets world coordinates from win and canvas.
     */
    virtual void window_to_world(double winx, double winy, double *worldx, double *worldy);
    
    /**
     * Sets win coordinates from world and canvas.
     */
    virtual void world_to_window(double worldx, double worldy, double *winx, double *winy);
    
    /**
     * Converts point from win to world coordinates.
     */
    virtual Geom::Point window_to_world(Geom::Point const win);
    
    /**
     * Converts point from world to win coordinates.
     */
    virtual Geom::Point world_to_window(Geom::Point const world);
    
    int emit_event(GdkEvent *event);
    
public:
    
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    bool enable_cms_display_adj;
    Glib::ustring* cms_key;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    
    int forced_redraw_count;
    int forced_redraw_limit;
    
protected:
    
    /* Last known modifier state, for deferred repick when a button is down */
    int state;
    
    /* Event on which selection of current item is based */
    GdkEvent pick_event;
    
    unsigned int need_update : 1;
    
    /* Area that needs redrawing, stored as a microtile array */
    int    tLeft,tTop,tRight,tBottom;
    int    tileH,tileV;
    uint8_t *tiles;
    
    GdkColor bg_color;
    
protected:
    void resize_tiles(int nl, int nt, int nr, int nb);
    void sp_canvas_dirty_rect(int nl, int nt, int nr, int nb);
    void mark_rect(int nl, int nt, int nr, int nb, uint8_t val);
    int pick_current_item (GdkEvent *event);
    NR::IPoint integerOrigin();
    
private:
    
    SPCanvasItem * _root;
    int _dirty_tiles_num;
    /* For use by internal pick_current_item() function */
    unsigned int left_grabbed_item : 1;
    /* For use by internal pick_current_item() function */
    unsigned int in_repick : 1;
};


#endif /* defined(__InkscapeCore__SPTilesCanvas__) */
