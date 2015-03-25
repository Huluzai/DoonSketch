//
//  CocoaCanvas.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <gdk/gdkkeysyms.h>
#import <Foundation/Foundation.h>
#import <gtk/gtk.h>
#import <desktop.h>
#import <display/canvas-arena.h>
#import <desktop-events.h>
#import <inkscape.h>
#import <cairo/cairo-quartz.h>
#import <cairo.h>
#import <verbs.h>
#import <helper/action.h>

#import "sp-canvas-cocoa.h"
#import "CocoaCanvasView.h"
#import "SWImageTools.h"

NSString *kCocoaCanvasViewWillStartGesture = @"CocoaCanvasViewWillStartGesture";
NSString *kCocoaCanvasViewDidEndGesture = @"CocoaCanvasViewDidEndGesture";
NSString *kCocoaCanvasViewDidMagnify = @"CocoaCanvasViewDidMagnify";
NSString *kCocoaCanvasViewDidScrollNotification = @"CocoaCanvasViewDidScrollNotification";

using namespace Inkscape;

#define INT(n)  (floor(n + 0.5))

@interface NSEvent (GdkExtentions)

- (guint)gdkEventState;
- (CGPoint)locationInCanvasView:(NSView *)canvas;

@end


@implementation NSEvent (GdkExtentions)

- (guint)gdkEventState
{
    guint state = 0;
    
    /*  1 << 0 corresponds to the left mouse button,
     1 << 1 corresponds to the right mouse button,
     1<< n, n >=2 correspond to other mouse buttons.
     */
    NSUInteger pressed = [NSEvent pressedMouseButtons];
    
    if (pressed == 1 << 0) {
        state |= GDK_BUTTON1_MASK;
    } else if (pressed == 1 << 1) {
        state |= GDK_BUTTON2_MASK;
    } else if (pressed >= 1 << 2) {
        state |= GDK_BUTTON3_MASK;
    }
    
    state |= self.modifierFlags & NSShiftKeyMask ? GDK_SHIFT_MASK : 0;
    state |= self.modifierFlags & NSAlternateKeyMask ? GDK_MOD1_MASK : 0;
    state |= self.modifierFlags & NSControlKeyMask ? GDK_CONTROL_MASK : 0;
    state |= self.modifierFlags & NSCommandKeyMask ? GDK_SUPER_MASK : 0;
    
    return state;
}

- (CGPoint)locationInCanvasView:(NSView *)canvas
{
    CGPoint loc = [canvas convertPoint:[self locationInWindow] fromView:nill];
    return loc;
}

@end


@interface CocoaCanvasView () {
    SPCocoaCanvas *_canvas;
}
@property CGPoint worldCoordinateOrigin;
@property NSBitmapImageRep *liveResizeImage;
@property CGRect liveResizeRect;
@property BOOL inLiveMagnify;

@end


@implementation CocoaCanvasView
#if 1
- (id)initWithFrame:(NSRect)frameRect
{
    if (self = [super initWithFrame:frameRect]) {
    }
    return self;
}

- (void)dealloc
{
    if (self.canvas) {
        self.canvas->unref();
    }
    self.canvas = NULL;
    
    [defaultNfc removeObserver:self];
    [self removeTrackingArea:self.trackingArea];
    self.trackingArea = nill;
}

- (void)viewDidMoveToWindow
{
    [self.window setAcceptsMouseMovedEvents:TRUE];
}

- (void)setCanvas:(SPCocoaCanvas *)canvas
{
    if (canvas) {
        canvas->ref();
    }
    
    if (_canvas) {
        _canvas->unref();
    }
    
    _canvas = canvas;
    
    if (_canvas) {
        _canvas->set_ui_delegate(self);
        
        GtkAllocation alc;
        alc.x = self.visibleRect.origin.x;
        alc.y = self.visibleRect.origin.y;
        alc.width = self.visibleRect.size.width;
        alc.height = self.visibleRect.size.height;
        _canvas->on_size_allocate(alc);
    }
}

- (SPCocoaCanvas *)canvas
{
    return _canvas;
}

- (CGPoint)convertPointFromCanvas:(CGPoint)point
{
    return point;
}

- (BOOL)isFlipped
{
#ifndef COCOA_CANVAS_USE_BUFFER
    return TRUE;
#else
    return TRUE;
#endif
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (CGPoint)mapEventPoint:(CGPoint)point
{
    if (self.canvas) {
        Geom::Rect r = self.canvas->getViewbox();
        return CGPointMake(point.x - r.min()[0], point.y - r.min()[1]);
    }
    return CGPointZero;
}

- (void)viewDidMoveToSuperview
{
    NSScrollView *scroll = [self enclosingScrollView];
    
    if (scroll) {
        [scroll.contentView setPostsBoundsChangedNotifications:YES];
        [scroll.contentView setPostsFrameChangedNotifications:YES];
        [defaultNfc addObserver:self
                       selector:@selector(didChangeVisibleRect:)
                           name:NSViewBoundsDidChangeNotification
                         object:scroll.contentView];
        [defaultNfc addObserver:self
                       selector:@selector(didChangeVisibleRect:)
                           name:NSViewFrameDidChangeNotification
                         object:scroll.contentView];
        /*
        [defaultNfc addObserver:self
                       selector:@selector(scrollViewDidEndLiveMagnifyNotification:)
                           name:NSScrollViewDidEndLiveMagnifyNotification
                         object:nill];
        [defaultNfc addObserver:self
                       selector:@selector(scrollViewWillStartLiveMagnifyNotification:)
                           name:NSScrollViewWillStartLiveMagnifyNotification
                         object:nill];
         */
        [self didChangeVisibleRect:nill];
    }
}

- (void)updateTrackingArea
{
    [self removeTrackingArea:self.trackingArea];
    NSUInteger trackFlags = NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow;
    self.trackingArea = [[NSTrackingArea alloc] initWithRect:self.visibleRect
                                                     options:trackFlags
                                                       owner:self
                                                    userInfo:nill];
    [self addTrackingArea:self.trackingArea];
}

- (void)didChangeVisibleRect:(NSNotification *)nf
{
    if ((!nf || nf.object == self.superview) && self.canvas) {
        GtkAllocation alc;
        alc.x = INT(self.visibleRect.origin.x);
        alc.y = INT(self.visibleRect.origin.y);
        alc.width = INT(self.visibleRect.size.width);
        alc.height = INT(self.visibleRect.size.height);
        
        g_debug("change visible : %d %d %d %d", alc.x, alc.y, alc.width, alc.height);
        
        Geom::Rect box = self.canvas->getViewbox();
        
        self.canvas->on_size_allocate(alc);
        
        CGPoint point = CGPointMake(alc.x, alc.y);
        CGRect rh, rv;
        
        if (point.x > box.min()[0]) {
            rh.origin.x = box.max()[0];
            rh.origin.y = point.y;
            rh.size.width = point.x - box.min()[0];
            rh.size.height = box.height();
        } else {
            rh.origin.x = point.x;
            rh.origin.y = point.y;
            rh.size.width = box.min()[0] - point.x;
            rh.size.height = box.height();
        }
        
        if (point.y > box.min()[1]) {
            rv.origin.y = box.max()[1];
            rv.origin.x = point.x;
            rv.size.height = point.y - box.min()[1];
            rv.size.width = box.width();
        } else {
            rv.origin.y = point.y;
            rv.origin.x = point.x;
            rv.size.height = box.min()[1] - point.y;
            rv.size.width = box.width();
        }
        
        GdkEventExpose e;
        e.window = NULL;
        e.count = 2;
        e.region = gdk_region_new();
        GdkRectangle grh;
        grh.x = rh.origin.x; grh.y = rh.origin.y; grh.width = rh.size.width; grh.height = rh.size.height;
        GdkRectangle grv;
        grv.x = rv.origin.x; grv.y = rv.origin.y; grv.width = rv.size.width; grv.height = rv.size.height;
        gdk_region_union_with_rect(e.region, &grh);
        gdk_region_union_with_rect(e.region, &grv);
        self.canvas->on_expose_event(&e);
        
        gdk_region_destroy(e.region);
        
        e.count = 0;
        e.region = gdk_region_new();
        
        //size
        if (alc.width > box.width()) {
            grh.x = box.max()[0];
            grh.y = alc.y;
            grh.width = alc.width - box.width();
            grh.height = alc.height;
            e.count ++;
            gdk_region_union_with_rect(e.region, &grh);
        }
        if (alc.height > box.height()) {
            grv.x = alc.x;
            grv.y = box.max()[1];
            grv.width = alc.width;
            grv.height = alc.height - box.height();
            e.count++;
            gdk_region_union_with_rect(e.region, &grv);
        }
        self.canvas->on_expose_event(&e);
    }
}

- (void)scrollToPoint:(CGPoint)point
{
    CGRect rh, rv;
    
    if (self.canvas) {
        //calc exposed rects
        Geom::Rect box = self.canvas->getViewbox();
        if (point.x > box.min()[0]) {
            rh.origin.x = box.max()[0];
            rh.origin.y = point.y;
            rh.size.width = point.x - box.min()[0];
            rh.size.height = box.height();
        } else {
            rh.origin.x = point.x;
            rh.origin.y = point.y;
            rh.size.width = box.min()[0] - point.x;
            rh.size.height = box.height();
        }
        
        if (point.y > box.min()[1]) {
            rv.origin.y = box.max()[1];
            rv.origin.x = point.x;
            rv.size.height = point.y - box.min()[1];
            rv.size.width = box.width();
        } else {
            rv.origin.y = point.y;
            rv.origin.x = point.x;
            rv.size.height = box.min()[1] - point.y;
            rv.size.width = box.width();
        }
    }
    
    if ([self.superview isKindOfClass:[NSClipView class]]) {
        [(NSClipView *)self.superview scrollToPoint:[self.superview convertPoint:point fromView:self]];
    } else {
        [self scrollPoint:point];
    }
    /*
    if (self.canvas) {
        GdkEventExpose e;
        e.window = NULL;
        e.count = 2;
        e.region = gdk_region_new();
        GdkRectangle grh;
        grh.x = rh.origin.x; grh.y = rh.origin.y; grh.width = rh.size.width; grh.height = rh.size.height;
        GdkRectangle grv;
        grv.x = rv.origin.x; grv.y = rv.origin.y; grv.width = rv.size.width; grv.height = rv.size.height;
        gdk_region_union_with_rect(e.region, &grh);
        gdk_region_union_with_rect(e.region, &grv);
        self.canvas->on_expose_event(&e);
    }*/
}

- (void)requestUpdateRect:(CGRect)dirtyRect
{
    g_debug("canvas request update %.0f %.0f %.0f %.0f",
            dirtyRect.origin.x, dirtyRect.origin.y, dirtyRect.size.width, dirtyRect.size.height);
    /*
    if (self.canvas) {
        self.canvas->request_redraw(INT(CGRectGetMinX(dirtyRect)),
                                    INT(CGRectGetMinY(dirtyRect)),
                                    INT(CGRectGetMaxX(dirtyRect)),
                                    INT(CGRectGetMaxY(dirtyRect)));
    }*/
//    [self setNeedsDisplayInRect:dirtyRect];
    [self displayRect:dirtyRect];
}

- (void)prepareContentInRect:(NSRect)rect
{
    [super prepareContentInRect:rect];
}

- (NSView *)view
{
    return self;
}

#if 1

- (void)mouseDown:(NSEvent *)theEvent
{
    CGPoint loc = [theEvent locationInCanvasView:self];
    GTimeVal now;
    
    loc = [self mapEventPoint:loc];
    
    g_get_current_time(&now);
    
    GdkEventButton e;
    e.type = theEvent.clickCount > 1 ? GDK_2BUTTON_PRESS : GDK_BUTTON_PRESS;
    e.x = loc.x;
    e.y = loc.y;
    e.window = NULL;    // TODO :
    e.axes = NULL;
    e.button = 1;
    e.time = (guint)(now.tv_sec * 1000 + now.tv_usec / 1000);
    e.send_event = TRUE;
    e.device = NULL;    // TODO : need ?
    e.x_root = 0;
    e.y_root = 0;
    e.state = [theEvent gdkEventState];
    
    if (self.canvas) {
        self.canvas->on_button_press_event(&e);
    }
}

- (void)mouseUp:(NSEvent *)theEvent
{
    CGPoint loc = [theEvent locationInCanvasView:self];
    GTimeVal now;
    
    loc = [self mapEventPoint:loc];
    
    g_get_current_time(&now);
    
    GdkEventButton e;
    e.type = GDK_BUTTON_RELEASE;
    e.x = loc.x;
    e.y = loc.y;
    e.window = NULL;    // TODO :
    e.axes = NULL;
    e.button = 1;
    e.time = (guint)(now.tv_sec * 1000 + now.tv_usec / 1000);
    e.send_event = TRUE;
    e.device = NULL;    // TODO : need ?
    e.x_root = 0;
    e.y_root = 0;
    e.state = [theEvent gdkEventState];
    
    if (self.canvas) {
        self.canvas->on_button_release_event(&e);
    }
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    CGPoint loc = [theEvent locationInCanvasView:self];
    GTimeVal now;
    
    loc = [self mapEventPoint:loc];
    
    g_get_current_time(&now);
    
    GdkEventMotion e;
    e.type = GDK_MOTION_NOTIFY;
    e.x = loc.x;
    e.y = loc.y;
    e.window = NULL;    // TODO :
    e.axes = NULL;
    e.time = (guint)(now.tv_sec * 1000 + now.tv_usec / 1000);
    e.send_event = TRUE;
    e.device = NULL;    // TODO : need ?
    e.x_root = 0;
    e.y_root = 0;
    e.is_hint = 0;
    e.state = [theEvent gdkEventState];
    
    if (self.canvas) {
        self.canvas->on_motion_notify_event(&e);
    }
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    [self mouseDragged:theEvent];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    CGPoint loc = [theEvent locationInCanvasView:self];
    GTimeVal now;
    
    loc = [self mapEventPoint:loc];
    
    g_get_current_time(&now);
    
    GdkEventCrossing e;
    e.type = GDK_ENTER_NOTIFY;
    e.x = loc.x;
    e.y = loc.y;
    e.window = NULL;    // TODO :
    e.subwindow = NULL;
    e.time = (guint)(now.tv_sec * 1000 + now.tv_usec / 1000);
    e.send_event = TRUE;
    e.x_root = 0;
    e.y_root = 0;
    e.mode = GDK_CROSSING_NORMAL;
    e.detail = GDK_NOTIFY_UNKNOWN;
    e.focus = TRUE;
    e.state = [theEvent gdkEventState];
    
    if (self.canvas) {
        self.canvas->on_enter_notify_event(&e);
    }
}

- (void)mouseExited:(NSEvent *)theEvent
{
    CGPoint loc = [theEvent locationInCanvasView:self];
    GTimeVal now;
    
    loc = [self mapEventPoint:loc];
    
    g_get_current_time(&now);
    
    GdkEventCrossing e;
    e.type = GDK_LEAVE_NOTIFY;
    e.x = loc.x;
    e.y = loc.y;
    e.window = NULL;    // TODO :
    e.subwindow = NULL;
    e.time = (guint)(now.tv_sec * 1000 + now.tv_usec / 1000);
    e.send_event = TRUE;
    e.x_root = 0;
    e.y_root = 0;
    e.mode = GDK_CROSSING_NORMAL;
    e.detail = GDK_NOTIFY_UNKNOWN;
    e.focus = TRUE;
    e.state = [theEvent gdkEventState];
    
    if (self.canvas) {
        self.canvas->on_leave_notify_event(&e);
    }
}

- (void)keyDown:(NSEvent *)theEvent
{
    GTimeVal now;
    GdkEventKey e;

    g_get_current_time(&now);
    e.type = GDK_KEY_PRESS;
    e.window = NULL;
    e.time = (guint)(now.tv_sec * 1000 + now.tv_usec / 1000);
    e.state = [theEvent gdkEventState];
    e.keyval = ([theEvent keyCode]);
    e.string = (gchar *)[[theEvent characters] UTF8String];    //deprecated
    e.length = (guint)[[theEvent characters] length];
    e.send_event = NO;
    e.hardware_keycode = [theEvent keyCode];
    e.is_modifier = 0;  //?
    e.group = 0;    //?
    
    if (self.canvas) {
        self.canvas->on_key_press_event(&e);
    }
}

- (void)keyUp:(NSEvent *)theEvent
{
    GTimeVal now;
    GdkEventKey e;
    
    g_get_current_time(&now);
    e.type = GDK_KEY_RELEASE;
    e.window = NULL;
    e.time = (guint)(now.tv_sec * 1000 + now.tv_usec / 1000);
    e.state = [theEvent gdkEventState];
    e.keyval = ([theEvent keyCode]);
    e.string = (gchar *)[[theEvent characters] UTF8String];    //deprecated
    e.length = (guint)[[theEvent characters] length];
    e.send_event = NO;
    e.hardware_keycode = [theEvent keyCode];
    e.is_modifier = 0;
    e.group = 0;
    
    if (self.canvas) {
        self.canvas->on_key_release_event(&e);
    }
}

#endif
#pragma mark - gesture

- (void)beginGestureWithEvent:(NSEvent *)event
{
    NSDictionary *usrInfo = [NSDictionary dictionaryWithObject:event forKey:@"event"];
    [defaultNfc postNotificationName:kCocoaCanvasViewWillStartGesture
                              object:self
                            userInfo:usrInfo];
}

- (void)endGestureWithEvent:(NSEvent *)event
{
    self.liveResizeImage = nill;
    
    NSDictionary *usrInfo = [NSDictionary dictionaryWithObject:event forKey:@"event"];
    [defaultNfc postNotificationName:kCocoaCanvasViewDidEndGesture
                              object:self
                            userInfo:usrInfo];
    
    //    self.inLiveMagnify = FALSE;
}

- (void)magnifyWithEvent:(NSEvent *)event
{
    /*    if (!self.inLiveMagnify) {
     self.inLiveMagnify = TRUE;
     
     self.liveResizeRect = self.visibleRect;
     self.liveResizeImage = [self bitmapImageRepForCachingDisplayInRect:self.liveResizeRect];
     [self cacheDisplayInRect:self.liveResizeRect toBitmapImageRep:self.liveResizeImage];
     }
     */
    g_debug("magnify with event %p magnification = %.2f", event, event.magnification);
    NSDictionary *usrInfo = [NSDictionary dictionaryWithObject:event forKey:@"event"];
    [defaultNfc postNotificationName:kCocoaCanvasViewDidMagnify
                              object:self
                            userInfo:usrInfo];
}

#ifdef COCOA_CANVAS_PAINT_WITH_NO_LOCKFOCUS
- (void)setNeedsDisplayInRect:(NSRect)dirtyRect
{
    [super setNeedsDisplayInRect:dirtyRect];
    if (self.canvas) {
        self.canvas->request_redraw(INT(dirtyRect.origin.x),
                                    INT(dirtyRect.origin.y),
                                    INT(CGRectGetMaxX(dirtyRect)),
                                    INT(CGRectGetMaxY(dirtyRect)));
    }
}
#endif

- (CGPoint)mouseLocation
{
    return [self convertPoint:[self.window mouseLocationOutsideOfEventStream] fromView:nill];
//    return CGPointZero;
}

- (void)viewWillDraw
{
    [super viewWillDraw];
    
}

- (void)drawRect:(NSRect)dirtyRect
{
#ifndef COCOA_CANVAS_PAINT_WITH_NO_LOCKFOCUS
    if (self.canvas) {
        double wy = [self isFlipped] ? CGRectGetMaxY(dirtyRect) : CGRectGetMinY(dirtyRect);
        double x, y;
        self.canvas->window_to_world(dirtyRect.origin.x, wy, &x, &y);
        self.canvas->request_redraw(x, y, x + CGRectGetWidth(dirtyRect), y + CGRectGetHeight(dirtyRect));
    }
#else
    if (self.liveResizeImage && self.inLiveMagnify) {
        [self.liveResizeImage drawAtPoint:self.liveResizeRect.origin];
        g_debug("in live resizing .");
    } else if (self.canvas) {
#if 1
        /*
        const NSRect *rects = NULL;
        NSInteger count = 0;
        
        [self getRectsBeingDrawn:&rects count:&count];
        if (rects && self.canvas) {
            for (NSInteger i = 0; i < count; i++) {
                NSRect dirtyRect = rects[i];
                
                g_debug("will draw rect : %.0f %.0f %.0f %.0f",
                        dirtyRect.origin.x,
                        dirtyRect.origin.y,
                        CGRectGetWidth(dirtyRect),
                        CGRectGetHeight(dirtyRect));
                
                self.canvas->request_redraw(INT(dirtyRect.origin.x),
                                            INT(dirtyRect.origin.y),
                                            INT(CGRectGetMaxX(dirtyRect)),
                                            INT(CGRectGetMaxY(dirtyRect)));
            }
            //        free((void *)rects);
        }*/
        if (self.canvas) {
            g_debug("will draw rect : %.0f %.0f %.0f %.0f",
                    dirtyRect.origin.x,
                    dirtyRect.origin.y,
                    CGRectGetWidth(dirtyRect),
                    CGRectGetHeight(dirtyRect));
            self.canvas->paint_rect(NULL,
                                    CGRectGetMinX(dirtyRect),
                                    CGRectGetMinY(dirtyRect),
                                    CGRectGetMaxX(dirtyRect),
                                    CGRectGetMaxY(dirtyRect));
        }
#else
        self.canvas->request_redraw(INT(dirtyRect.origin.x),
                                    INT(dirtyRect.origin.y),
                                    INT(CGRectGetMaxX(dirtyRect)),
                                    INT(CGRectGetMaxY(dirtyRect)));
#endif
    }
#endif
//    [[[NSColor greenColor] colorWithAlphaComponent:0.99] setFill];
//    [[NSGraphicsContext currentContext] setCompositingOperation:NSCompositeSourceOver];
}


#pragma mark - live resize
/*
- (void)scrollViewDidEndLiveMagnifyNotification:(NSNotification *)nc
{
    self.liveResizeImage = nill;
}

- (void)scrollViewWillStartLiveMagnifyNotification:(NSNotification *)nc
{
    self.liveResizeRect = self.visibleRect;
    self.liveResizeImage = [self bitmapImageRepForCachingDisplayInRect:self.liveResizeRect];
    [self cacheDisplayInRect:self.liveResizeRect toBitmapImageRep:self.liveResizeImage];
}
*/

#endif

@end
