//
//  CanvasController.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-17.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "CanvasController.h"
#import "sp-canvas-cocoa.h"
#import "ExpandableCanvasView.h"
#import "CanvasClipView.h"

#import <glib.h>

#define theCanvasView       ((ExpandableCanvasView *)(self.scrollView.documentView))
#define theClipView         ((CanvasClipView*)self.scrollView.contentView)
#define theCanvas           (_canvas)

#define RequestUpdateNotification   (@"RequestUpdateNotification")


@interface CanvasController () {
    NSScrollView *_scrollView;
    SPCocoaCanvas *_canvas;
}
- (void)didRequestUpdate:(NSNotification *)notification;
@end


@implementation CanvasController


- (id)init
{
    if (self = [super init]) {
        _canvas = new SPCocoaCanvas();
        _canvas->_controller = self;
        _canvas->init();
        [defaultNfc addObserver:self selector:@selector(didRequestUpdate:) name:RequestUpdateNotification object:nill];
    }
    return self;
}

- (void)dealloc
{
    [defaultNfc removeObserver:self];
    g_object_unref(_canvas);
}

- (void)setDesktop:(SPDesktop*)desktop
{
    [theCanvasView setDesktop:desktop];
}

- (SPDesktop *)desktop
{
    return [theCanvasView desktop];
}

- (void)setScrollView:(NSScrollView *)scrollView
{
    [defaultNfc removeObserver:self
                          name:NSViewBoundsDidChangeNotification
                        object:_scrollView.contentView];
    
    [scrollView.contentView setPostsBoundsChangedNotifications:YES];
    
    [defaultNfc addObserver:self
                   selector:@selector(didClipViewBoundsChanged:)
                       name:NSViewBoundsDidChangeNotification
                     object:scrollView.contentView];
    
    // Align
    ExpandableCanvasView *canvasView = (ExpandableCanvasView *)scrollView.documentView;
    CanvasClipView *clip = scrollView.contentView;
    
    canvasView.focusRect = [clip documentVisibleRect];
    
    CGPoint inkPt = [theCanvasView convertPointToInkscape:canvasView.focusRect.origin];
    theCanvas->do_scroll(inkPt.x, inkPt.y);
    
    _scrollView = scrollView;
    canvasView.canvas = _canvas;
    
    clip.scrollDelegate = self;
}

- (NSScrollView *)scrollView
{
    return _scrollView;
}

- (SPCocoaCanvas *)canvas
{
    return _canvas;
}

- (void)didClipViewBoundsChanged:(NSNotification *)nf
{
    theCanvasView.focusRect = [theClipView documentVisibleRect];
    CGPoint point = [theCanvasView convertPointToInkscape:theCanvasView.focusRect.origin];
    theCanvas->do_scroll(point.x, point.y);
}

- (void)scrollToPoint:(NSPoint)point
{
    [theClipView scrollToPoint:[theCanvasView convertPointFromInkscape:point]];
}

- (void)requestUpdateWhenIdle
{
    NSNotificationQueue *nq = [NSNotificationQueue defaultQueue];
    NSNotification *n = [NSNotification notificationWithName:RequestUpdateNotification object:nill];
    [nq enqueueNotification:n postingStyle:NSPostWhenIdle];
}

- (void)didRequestUpdate:(NSNotification *)notification
{
    theCanvas->do_update();
}

- (CGPoint)clipView:(CanvasClipView *)clipView willScrollToPoint:(CGPoint)newOrigin
{
    return [theCanvasView willScrollToPoint:newOrigin];
}

- (cairo_surface_t *)cacheSurface
{
    return theCanvasView.cacheSurface;
}

- (void)markDirtyRect:(NSRect)dirtyRect
{
    dirtyRect = CGRectOffset(dirtyRect,
                             theCanvasView.focusRect.origin.x,
                             theCanvasView.focusRect.origin.y);
    [theCanvasView setNeedsDisplayInRect:dirtyRect];
}

@end
