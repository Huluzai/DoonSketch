//
//  SPGtkCanvas.h
//  InkscapeCore
//
//  Created by 张 光建 on 14-9-4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#ifndef __InkscapeCore__SPGtkCanvas__
#define __InkscapeCore__SPGtkCanvas__

#include <gtk/gtk.h>
#include "sp-canvas.h"
#include "sp-canvas-tile.h"
#include <gtkmm.h>
#include <gtkmm/widget.h>
#include "libnr/nr-point-l.h"
#include "libnr/nr-point.h"

struct PaintRectSetup;

#define SP_GTK_CANVAS_TYPE     ("sp-canvas-gtk")
#define IS_GTK_CANVAS(c)    ((c)->get_type()==SP_GTK_CANVAS_TYPE)
#define SP_GTK_CANVAS(obj)     ((SPGtkCanvas*)(obj))
#define SP_GTK_CANVAS_WIDGET(obj)   (SP_GTK_CANVAS(obj)->get_widget()->gobj())
#define SP_GTK_CANVAS_WINDOW(c) (SP_GTK_CANVAS_WIDGET(c)->window)

class SPGtkCanvas : public SPTilesCanvas
{
    friend class SPGtkCanvasWidget;
    
public:
    SPGtkCanvas();
    virtual ~SPGtkCanvas();
    
    std::string get_type() const {return SP_GTK_CANVAS_TYPE;}
    
    void scroll_to(double cx, double cy, unsigned int clear, bool is_scrolling = false);
    void update_now();
    int do_update();
    
    void request_update ();
    void request_redraw(int x1, int y1, int x2, int y2);
    
    Geom::Rect getViewbox() const;
    NR::IRect getViewboxIntegers() const;
    
    bool paint_rect (SPTilesCanvas *canvas, int xx0, int yy0, int xx1, int yy1);
    
    Gtk::Widget *get_widget() {return _widget;}
    
protected:
    void on_realize();
    void on_unrealize();
    
    void on_size_request(Gtk::Requisition* requisition);
    void on_size_allocate(Gtk::Allocation& allocation);
    
    bool on_button_press_event(GdkEventButton* event);
    bool on_button_release_event(GdkEventButton* event);
    
    bool on_scroll_event(GdkEventScroll* event);
    bool on_motion_notify_event(GdkEventMotion* event);
    bool on_expose_event(GdkEventExpose* event);
    bool on_key_press_event(GdkEventKey* event);
    bool on_key_release_event(GdkEventKey* event);
    bool on_enter_notify_event(GdkEventCrossing* event);
    bool on_leave_notify_event(GdkEventCrossing* event);
    bool on_focus_in_event(GdkEventFocus* event);
    bool on_focus_out_event(GdkEventFocus* event);
    
protected:
    gint do_button_event (GdkEventButton *event);
    gint do_key_event(GdkEventKey *event);
    gint do_crossing_event (GdkEventCrossing *event);
    
protected:
    void shutdown_transients();
    void remove_idle ();
    void add_idle ();
    
    void paint_single_buffer (int x0, int y0, int x1, int y1, int draw_x1, int draw_y1, int draw_x2, int draw_y2, int sw);
    int paint_rect_internal (PaintRectSetup const *setup, NRRectL this_rect);
    
    static gint idle_handler (void *data);
    
private:
    Gtk::Widget *_widget;
    guint _idle_id;
    
    NR::Point _dorg;
    NR::IPoint _iorg;
    bool is_scrolling;
    
    /* GC for temporary draw pixmap */
    GdkGC *_pixmap_gc;
};
/*
struct SPGtkCanvasClass {
    GtkWidgetClass parent_class;
};

SPGtkCanvas *sp_gtk_canvas_new();
*/
#endif /* defined(__InkscapeCore__SPGtkCanvas__) */
