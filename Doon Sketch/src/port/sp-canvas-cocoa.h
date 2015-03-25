//
//  sp-canvas-cocoa.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#ifndef Inxcape_sp_canvas_cocoa_h
#define Inxcape_sp_canvas_cocoa_h

#include <display/sp-canvas-tile.h>
#include <gtk/gtk.h>
#include <cocoa/cocoa.h>

#define SP_COCOA_CANVAS_TYPE     ("sp-canvas-cocoa")
//#define COCOA_CANVAS_USE_BUFFER
//#define COCOA_CANVAS_PAINT_WITH_NO_LOCKFOCUS

@class CocoaCanvasView;
@class CanvasController;
@class IdleUpdater;

struct PaintRectSetup;


@protocol CanvasControl

- (void)didCanvasScroll:(CGPoint)moveVector;
- (void)didCanvasUpdateRect:(CGRect)dirtyRect;
- (void)didCanvasResize:(CGSize)size;
- (void)setNeedsUpdate:(BOOL)needsUpdate;

- (void)requestUpdateRect:(CGRect)dirtyRect;
- (NSView *)view;
- (void)scrollToPoint:(CGPoint)point;

- (CGPoint)mouseLocation;
- (CGPoint)convertPointFromCanvas:(CGPoint)point;

@end


struct SPCocoaCanvas : public SPTilesCanvas {
  
public:
    SPCocoaCanvas();
    virtual ~SPCocoaCanvas();
    
    void test() const;
    
    std::string get_type() const {return SP_COCOA_CANVAS_TYPE;}
    
    void scroll_to(double cx, double cy, unsigned int clear, bool is_scrolling = false);
    
    void update_now();
    int do_update();
    
    void request_update ();
    void request_redraw(int x1, int y1, int x2, int y2);
    
    Geom::Rect getViewbox() const;
    NR::IRect getViewboxIntegers() const;
    
#ifdef COCOA_CANVAS_USE_BUFFER
    NSBitmapImageRep *get_image_buffer() const {return _image_buffer;}
#endif
    
    void shutdown_transients ();
    
    void set_ui_delegate(id<CanvasControl> ui);
    id<CanvasControl> get_ui_delegate() const;
    
    virtual bool paint_rect (SPTilesCanvas */*canvas*/, int xx0, int yy0, int xx1, int yy1);
    void get_dirty_rects(const CGRect **rects, int *count);

public:

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
    
protected:
    void paint_single_buffer (int x0, int y0, int x1, int y1, int draw_x1, int draw_y1, int draw_x2, int draw_y2, int sw);
    int paint_rect_internal (PaintRectSetup const *setup, NRRectL this_rect);
    void do_scroll(double cx, double cy);
    
public:
    void on_size_allocate(GtkAllocation & allocation);
    bool on_expose_event(GdkEventExpose* event);
    bool on_button_press_event(GdkEventButton* event);
    bool on_button_release_event(GdkEventButton* event);
    bool on_motion_notify_event(GdkEventMotion* event);
    bool on_enter_notify_event(GdkEventCrossing* event);
    bool on_leave_notify_event(GdkEventCrossing* event);
    bool on_key_press_event(GdkEventKey* event);
    bool on_key_release_event(GdkEventKey* event);
    
    gint do_key_event(GdkEventKey *event);
    gint do_button_event (GdkEventButton *event);
    gint do_crossing_event (GdkEventCrossing *event);
    
    int idle_id;
    
protected:
    __weak id<CanvasControl> _ui;
#ifdef COCOA_CANVAS_USE_BUFFER
    NSBitmapImageRep *_image_buffer;
    NSBitmapImageRep *_scroll_buffer;
#endif
    IdleUpdater *_idle_updater;
    BOOL _is_scrolling;
    CGRect _allocateRect;
};

/*
struct SPCocoaCanvasClass {
    GtkWidgetClass parent_class;
};

SPCocoaCanvas *sp_cocoa_canvas_new();

*/
#endif
