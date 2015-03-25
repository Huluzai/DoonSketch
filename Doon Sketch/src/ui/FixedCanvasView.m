//
//  FixedCanvasView.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/24.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "FixedCanvasView.h"
#import "sp-canvas-cocoa.h"
#import "AbsoluteValueScroller.h"

#import <2geom/geom.h>
#import <gdk/gdk.h>


#define left()  (self.bounds.origin.x)
#define top()   (self.bounds.origin.y)
#define width() (self.bounds.size.width)
#define height() (self.bounds.size.height)
#define right() (left() + width())
#define bottom() (top() + height())


NSString *kFixedCanvasChangedOriginNotification = @"FixedCanvasChangedOriginNotification";


@interface FixedCanvasView () {
    CGPoint _currentPos;
};

@end


@implementation FixedCanvasView

- (void)setCanvas:(SPCocoaCanvas *)canvas
{
    [super setCanvas:canvas];
    if (canvas) {
        _currentPos = [self canvasPos];
        [self postOriginChangedNotification];
    }
}

- (void)scrollToPoint:(NSPoint)aPoint
{
    g_assert(self.canvas);
    
    CGFloat dx = aPoint.x - _currentPos.x;
    CGFloat dy = aPoint.y - _currentPos.y;
    
    dx = floor(dx);
    dy = floor(dy);
    
    if (dx == 0 && dy == 0) {
        return;
    }
    
    g_debug("respect to %.1f %.1f", aPoint.x, aPoint.y);
    
    //move the old content
    CGRect r = CGRectMake(MAX(left() + dx, left()), MAX(top() + dy, top()), width() - fabs(dx), height() - fabs(dy));
    [self scrollRect:r by:CGSizeMake(-dx, -dy)];
    
    g_debug("scroll view by %.0f %.0f to [%.1f %.1f %.1f %.1f]",
            dx, dy, self.canvasPos.x, self.canvasPos.y, self.bounds.size.width, self.bounds.size.height);
    
    CGRect rh = CGRectMake(left(), (dy > 0 ? bottom() - dy : top()), width(), fabs(dy));
    CGRect rv = CGRectMake((dx > 0 ? right() - dx : left()), top(), fabs(dx), height());
    [self setNeedsDisplayInRect:rh];
    [self setNeedsDisplayInRect:rv];
    
    _currentPos.y += dy;
    _currentPos.x += dx;
    [self postOriginChangedNotification];
}

- (CGPoint)convertPointFromCanvas:(CGPoint)point
{
    return CGPointMake(point.x - _currentPos.x, point.y - _currentPos.y);
}

- (void)postOriginChangedNotification
{
    [defaultNfc postNotificationName:kFixedCanvasChangedOriginNotification object:self];
}

- (void)setOrigin:(CGPoint)origin
{
    if (self.canvas) {
        self.canvas->scroll_to(origin.x + 0.5, origin.y + 0.5, TRUE);
    }
}

- (CGPoint)origin
{
    return _currentPos;
}

- (CGPoint)canvasPos
{
    if (self.canvas) {
        Geom::Rect vbox = self.canvas->getViewbox();
        return CGPointMake(vbox.min()[0], vbox.min()[1]);
    }
    return CGPointZero;
}

- (CGPoint)mapEventPoint:(CGPoint)point
{
    return point;
}

- (void)scrollWheel:(NSEvent *)event
{
    if (self.canvas) {
        CGPoint newPos = self.canvasPos;
        newPos.x -= event.deltaX * 5;
        newPos.y -= event.deltaY * 5;
        self.canvas->scroll_to(newPos.x + 0.5, newPos.y + 0.5, TRUE);
    }
}

- (NSView *)view
{
    return self;
}

- (void)setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    
    if (self.canvas) {
        GtkAllocation alc;
        alc.x = self.canvasPos.x;
        alc.y = self.canvasPos.y;
        alc.width = CGRectGetWidth(self.bounds);
        alc.height = CGRectGetHeight(self.bounds);
        self.canvas->on_size_allocate(alc);
    }
    
    [self updateTrackingArea];
}

- (void)drawRect:(NSRect)dirtyRect
{
    if (self.canvas) {
        const NSRect *rects = NULL;
        NSInteger count = 0;
        
        [self getRectsBeingDrawn:&rects count:&count];
        
        if (rects) {
            for (int i = 0; i < count; i++) {
                dirtyRect = rects[i];
                CGRect r = CGRectOffset(dirtyRect, _currentPos.x, _currentPos.y);
                g_debug("draw rect : %.0f %.0f %.0f %.0f",
                        CGRectGetMinX(r) + 0.5,
                        CGRectGetMinY(r) + 0.5,
                        CGRectGetMaxX(r) + 0.5,
                        CGRectGetMaxY(r) + 0.5);
                self.canvas->request_redraw(CGRectGetMinX(r) + 0.5,
                                            CGRectGetMinY(r) + 0.5,
                                            CGRectGetMaxX(r) + 0.5,
                                            CGRectGetMaxY(r) + 0.5);
            }
        }
    }
}

@end

