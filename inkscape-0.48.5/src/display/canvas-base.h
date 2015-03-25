//
//  canvas-base.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#ifndef Inxcape_canvas_interface_h
#define Inxcape_canvas_interface_h

#include <2geom/geom.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libnr/nr-blit.h>
#include <glib/ghash.h>
//#include <glibmm.h>
/*
#define SP_TYPE_CANVAS (sp_canvas_get_type ())
#define SP_CANVAS(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_CANVAS, CanvasBase))
#define SP_CANVAS_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_CANVAS, SPCanvasBaseClass))
#define SP_IS_CANVAS(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CANVAS))
#define SP_IS_CANVAS_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_CANVAS))
*/

struct SPCanvasGroup;
struct SPCanvasItem;

struct CanvasBase
{
    bool gen_all_enter_events;
    
    int rendermode;
    
    /* Item that holds a pointer grab, or NULL if none */
    SPCanvasItem *grabbed_item;
    
    /* If non-NULL, the currently focused item */
    SPCanvasItem *focused_item;
    
    /* The item containing the mouse pointer, or NULL if none */
    SPCanvasItem *current_item;
    
    /* Item that is about to become current (used to track deletions and such) */
    SPCanvasItem *new_current_item;
    
    unsigned int need_repick : 1;
    
    int close_enough;
    
    /* Event mask specified when grabbing an item */
    guint grabbed_event_mask;
    
    virtual SPCanvasGroup *root() = 0;
    
    virtual int emit_event(GdkEvent *event) = 0;
    
    virtual void scroll_to(double cx, double cy, unsigned int clear, bool is_scrolling = false) {}
    virtual void update_now() {}
    
    virtual void request_update () {}
    virtual void request_redraw(int x1, int y1, int x2, int y2) {}
    
    virtual void force_full_redraw_after_interruptions(unsigned int count) {}
    virtual void end_forced_full_redraws() {}
    
    /**
     * Returns true if point given in world coordinates is inside window.
     */
    virtual bool world_pt_inside_window(Geom::Point const &world) = 0;
    
    /**
     * Sets world coordinates from win and canvas.
     */
    virtual void window_to_world(double winx, double winy, double *worldx, double *worldy) = 0;
    
    /**
     * Sets win coordinates from world and canvas.
     */
    virtual void world_to_window(double worldx, double worldy, double *winx, double *winy) = 0;
    
    /**
     * Converts point from win to world coordinates.
     */
    virtual Geom::Point window_to_world(Geom::Point const win) = 0;
    
    /**
     * Converts point from world to win coordinates.
     */
    virtual Geom::Point world_to_window(Geom::Point const world) = 0;
    
    virtual Geom::Rect getViewbox() const = 0;
    
    virtual NR::IRect getViewboxIntegers() const = 0;
    
    virtual std::string get_type() const = 0;
    
private:
    int reference_cnt;
    GHashTable *data_table;
    
public:
    CanvasBase();
    virtual ~CanvasBase();
    CanvasBase *ref();
    void unref();
    void set_data(char *key, void *value);
    void *get_data(char *key);
};

#if 0
struct SPCanvasBase
{
    GtkWidget widget;
    
    bool gen_all_enter_events;

    int rendermode;
    
    /* Item that holds a pointer grab, or NULL if none */
    SPCanvasItem *grabbed_item;
    
    /* If non-NULL, the currently focused item */
    SPCanvasItem *focused_item;
    
    /* The item containing the mouse pointer, or NULL if none */
    SPCanvasItem *current_item;
    
    /* Item that is about to become current (used to track deletions and such) */
    SPCanvasItem *new_current_item;

    unsigned int need_repick : 1;

    int close_enough;
    
    /* Event mask specified when grabbing an item */
    guint grabbed_event_mask;
};

struct SPCanvasBaseClass
{
    GtkWidgetClass parent_class;
    
    int (*emit_event)(SPCanvasBase *canvas);
    void (*scroll_to)(SPCanvasBase *canvas, double cx, double cy, unsigned int clear, bool is_scrolling);
    void (*update_now)(SPCanvasBase *canvas);
    void (*request_update)(SPCanvasBase *canvas);
    void (*request_redraw)(SPCanvasBase *canvas, int x1, int y1, int x2, int y2);
    void (*force_full_redraw_after_interruptions)(SPCanvasBase *canvas, unsigned int count);
    void (*end_forced_full_redraws)(SPCanvasBase *canvas);
    
    SPCanvasGroup (*get_root)(SPCanvasBase *canvas);
    bool (*world_pt_inside_window)(SPCanvasBase const *canvas, Geom::Point const &world);
    void (*window_to_world)(SPCanvasBase const *canvas, double winx, double winy, double *worldx, double *worldy);
    void (*world_to_window)(SPCanvasBase const *canvas, double worldx, double worldy, double *winx, double *winy);
    Geom::Point (*window_to_world_point)(SPCanvasBase const *canvas, Geom::Point const win);
    Geom::Point (*world_to_window_point)(SPCanvasBase const *canvas, Geom::Point const world);
    Geom::Rect (*get_view_box)(SPCanvasBase const*canvas);
    NR::IRect (*get_view_box_integers)(SPCanvasBase const *canvas);
};

GType sp_canvas_get_type(void);

SPCanvasGroup *sp_canvas_get_root(SPCanvasBase *canvas);

void sp_canvas_scroll_to(SPCanvasBase *canvas, double cx, double cy, unsigned int clear, bool is_scrolling = false);
void sp_canvas_update_now(SPCanvasBase *canvas);

void sp_canvas_request_redraw(SPCanvasBase *canvas, int x1, int y1, int x2, int y2);
void sp_canvas_force_full_redraw_after_interruptions(SPCanvasBase *canvas, unsigned int count);
void sp_canvas_end_forced_full_redraws(SPCanvasBase *canvas);

bool sp_canvas_world_pt_inside_window(SPCanvasBase const *canvas, Geom::Point const &world);

void sp_canvas_window_to_world(SPCanvasBase const *canvas, double winx, double winy, double *worldx, double *worldy);
void sp_canvas_world_to_window(SPCanvasBase const *canvas, double worldx, double worldy, double *winx, double *winy);

Geom::Point sp_canvas_window_to_world(SPCanvasBase const *canvas, Geom::Point const win);
Geom::Point sp_canvas_world_to_window(SPCanvasBase const *canvas, Geom::Point const world);

Geom::Rect sp_canvas_get_view_box(SPCanvasBase const *canvas);
NR::IRect sp_canvas_get_view_box_integers(SPCanvasBase const *canvas);
#endif
#endif
