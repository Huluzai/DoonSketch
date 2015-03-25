//
//  ExpandableCanvasView.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-17.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ExpandableCanvasView.h"
#import "SWImageTools.h"
#import "sp-canvas-cocoa.h"
#import <gtk/gtk.h>
#import <cairo/cairo-quartz.h>
#import <cairo.h>
#import <math.h>
#import <document.h>
#import <desktop.h>
#import <desktop-handles.h>
#import <sp-item.h>
#import "CanvasClipView.h"

@interface ExpandableCanvasView () {
    CGRect _focusRect;
    cairo_surface_t *_cacheSurface;
}

@property CGPoint inkscapeOrigin;
@property CGSize minSizeLimit;

@end


@implementation ExpandableCanvasView

- (id)initWithFrame:(NSRect)frameRect
{
    if (self = [super initWithFrame:frameRect]) {
        self.inkscapeOrigin = CGPointMake(self.frame.size.width / 2, self.frame.size.height / 2);
        _cacheSurface = NULL;
        self.minSizeLimit = frameRect.size;
        self.allowsExpand = TRUE;
    }
    return self;
}

- (void)dealloc
{
    if (_cacheSurface) {
        cairo_surface_destroy(_cacheSurface);
    }
}

// How to do it when boundsSize != frameSize
- (void)setFocusRect:(CGRect)newFocus
{
    CGSize size = newFocus.size;
    CGSize oldSize = _focusRect.size;
    
    if (!CGSizeEqualToSize(size, oldSize)) {
        
        if (_cacheSurface) {
            cairo_surface_destroy(_cacheSurface);
        }
        
        _cacheSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size.width, size.height);
        cairo_t *cr = cairo_create(_cacheSurface);
        cairo_set_source_rgba(cr, 1, 1, 1, 1);
        cairo_paint(cr);
        cairo_destroy(cr);
    }

    [self expandByFocusRectIfNeed:newFocus];
    [self shrinkByFocusRectIfNeed:newFocus];
    
    if (self.canvas) {
        GtkAllocation alc;
        alc.x = 0;
        alc.y = 0;
        alc.width = newFocus.size.width;
        alc.height = newFocus.size.height;
        
        self.canvas->on_size_allocate(alc);
    }
    
    CGFloat limitWidth = self.frame.size.width - newFocus.size.width;
    CGFloat limitHeight = self.frame.size.height - newFocus.size.height;
    CGFloat baseX = CGRectGetMinX(newFocus);
    CGFloat baseY = CGRectGetMinY(newFocus);
    baseX = MIN(limitWidth, MAX(0, baseX));
    baseY = MIN(limitHeight, MAX(0, baseY));
    
    _focusRect = CGRectMake(baseX, baseY, newFocus.size.width, newFocus.size.height);
    
    [self removeTrackingArea:self.trackingArea];
    NSUInteger trackFlags = NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow;
    self.trackingArea = [[NSTrackingArea alloc] initWithRect:_focusRect
                                                     options:trackFlags
                                                       owner:self
                                                    userInfo:nill];
    [self addTrackingArea:self.trackingArea];
}

- (CGRect)focusRect
{
    return _focusRect;
}

- (cairo_surface_t *)cacheSurface
{
    return _cacheSurface;
}

- (BOOL)expandByFocusRectIfNeed:(CGRect)rect
{
    CGRect docRect = CGRectMake(0, 0, self.frame.size.width, self.frame.size.height);
    CGRect bigRect = CGRectUnion(docRect, rect);
    
    if (!self.allowsExpand) {
        return FALSE;
    }
    
    if (CGRectGetWidth(bigRect) > CGRectGetWidth(docRect)
        || CGRectGetHeight(bigRect) > CGRectGetHeight(docRect)) {
    
        // recalc inkscape origin
        self.inkscapeOrigin = CGPointMake(rect.origin.x < 0 ? self.inkscapeOrigin.x + fabs(rect.origin.x) : self.inkscapeOrigin.x,
                                          rect.origin.y < 0 ? self.inkscapeOrigin.y + fabs(rect.origin.y) : self.inkscapeOrigin.y);

        // enlarge frame
        CGRect frame = self.frame;
        frame.size = bigRect.size;
        CGRectOffset(frame, bigRect.origin.x, bigRect.origin.y);
        [self setFrame:frame];
        
        g_debug("Expand to frame : [%.0f %.0f %.0f %.0f]",
                CGRectGetMinX(self.frame), CGRectGetMinY(self.frame),
                CGRectGetWidth(self.frame), CGRectGetHeight(self.frame));
        return TRUE;
    }
    return FALSE;
}

- (BOOL)shrinkByFocusRectIfNeed:(CGRect)rect
{
    g_return_val_if_fail(self.canvas, FALSE);
    
    if (!self.allowsExpand) {
        return FALSE;
    }
    
    // Minimal rect
    CGRect bigRect = CGRectMake(self.inkscapeOrigin.x - self.minSizeLimit.width / 2,
                                self.inkscapeOrigin.y - self.minSizeLimit.height / 2,
                                self.minSizeLimit.width,
                                self.minSizeLimit.height);
    
    // Focus rect
    bigRect = CGRectUnion(bigRect, rect);
    
    // objects frame
    SPItem *docitem = NULL;
    Geom::OptRect d;
    
    if (self.desktop && sp_desktop_document(self.desktop)
        && (docitem = SP_ITEM (sp_document_root (self.desktop->doc())))
        && (d = sp_item_bbox_desktop(docitem)) && (d->minExtent() >= 0.1)) {
        /* Note that the second condition here indicates that
         ** there are no items in the drawing.
         */
        double zoom = self.desktop->current_zoom();
        CGPoint point = CGPointMake(d->min()[Geom::X] * zoom, d->min()[Geom::Y] * zoom);
        point = [self convertPointFromInkscape:point];
        CGRect objsRect = CGRectMake(point.x, point.y, d->width() * zoom, d->height() * zoom);
        
        bigRect = CGRectUnion(bigRect, objsRect);
    }
    
    BOOL doShrink = FALSE;
    CGRect newRect = self.frame;
    CGPoint newOrg = self.inkscapeOrigin;
    CGFloat dx = bigRect.size.width - self.frame.size.width;
    
    if (dx < 0) {
        newOrg.x -= bigRect.origin.x;
        if (bigRect.origin.x == 0) {
            newRect.origin.x += fabs(dx);
        }
        newRect.size.width = bigRect.size.width;
        doShrink = TRUE;
    }
    
    CGFloat dy = bigRect.size.height - self.frame.size.height;
    
    if (dy < 0) {
        newOrg.y -= bigRect.origin.y;
        if (bigRect.origin.y == 0) {
            newRect.origin.y += fabs(dy);
        }
        newRect.size.height = bigRect.size.height;
        doShrink = TRUE;
    }
    
    if (doShrink) {
        self.inkscapeOrigin = newOrg;
        [self setFrame:newRect];
        g_debug("shrink frame to [%.0f %.0f %.0f %.0f]",
                self.frame.origin.x,
                self.frame.origin.y,
                self.frame.size.width,
                self.frame.size.height);
    }
    
    return doShrink;
}

- (CGPoint)willScrollToPoint:(CGPoint)newOrigin
{/*
    static bool inLoop = false;
    
    if (!inLoop) {
        inLoop = true;
        CGRect newFocus = CGRectMake(newOrigin.x,
                                     newOrigin.y,
                                     self.focusRect.size.width,
                                     self.focusRect.size.height);
        
        [self expandByFocusRectIfNeed:newFocus];
        [self shrinkByFocusRectIfNeed:newFocus];
        
        CGFloat limitWidth = self.frame.size.width - newFocus.size.width;
        CGFloat limitHeight = self.frame.size.height - newFocus.size.height;
        CGFloat baseX = CGRectGetMinX(newFocus);
        CGFloat baseY = CGRectGetMinY(newFocus);
        baseX = MIN(limitWidth, MAX(0, baseX));
        baseY = MIN(limitHeight, MAX(0, baseY));
        
        inLoop = false;
        return CGPointMake(baseX,baseY);
    }*/
    return newOrigin;
}

- (CGPoint)mapEventPoint:(CGPoint)point
{
    return CGPointMake(point.x - self.focusRect.origin.x, point.y - self.focusRect.origin.y);
}

- (CGPoint)convertPointToInkscape:(CGPoint)point
{
    return CGPointMake(point.x - self.inkscapeOrigin.x, point.y - self.inkscapeOrigin.y);
}

- (CGPoint)convertPointFromInkscape:(CGPoint)point
{
    return CGPointMake(point.x + self.inkscapeOrigin.x, point.y + self.inkscapeOrigin.y);
}

- (void)didChangeBounds:(NSNotification *)nfc
{}

- (void)drawRect:(NSRect)dirtyRect
{
    g_return_if_fail(self.canvas && _cacheSurface);
    
    g_return_if_fail(CGRectIntersectsRect(dirtyRect, self.focusRect));
    
    g_debug("Draw dirty rect (%.0f, %.0f, %.0f, %.0f)",
            dirtyRect.origin.x, dirtyRect.origin.y, dirtyRect.size.width, dirtyRect.size.height);
    
    dirtyRect = CGRectIntersection(dirtyRect, self.focusRect);
    
    CGFloat width = CGRectGetWidth(dirtyRect);
    CGFloat height = CGRectGetHeight(dirtyRect);
    CGFloat x = dirtyRect.origin.x;
    CGFloat y = dirtyRect.origin.y;
    CGPoint org = CGPointMake(x - CGRectGetMinX(self.focusRect), y - CGRectGetMinY(self.focusRect));
    CGPoint ikPoint = [self convertPointToInkscape:CGPointMake(x, y)];
    
//    self.canvas->paint_rect(self.canvas, ikPoint.x, ikPoint.y, ikPoint.x + width, ikPoint.y + height);
    
    CGContextRef gc = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    cairo_surface_t *dst = cairo_quartz_surface_create_for_cg_context(gc, self.frame.size.width, self.frame.size.height);
    cairo_t *ct = cairo_create(dst);

    cairo_surface_t *src = cairo_surface_create_for_rectangle(_cacheSurface, org.x, org.y, width, height);

    cairo_set_source_surface(ct, src, dirtyRect.origin.x, dirtyRect.origin.y);
    cairo_paint(ct);
    cairo_surface_destroy(src);

#ifdef xxDEBUG
    
    cairo_set_line_width(ct, 2);
    cairo_set_source_rgb(ct, 1., 0., 0.);
    
    cairo_move_to(ct, x, y);
    cairo_line_to(ct, x + width, y);
    cairo_line_to(ct, x + width, y + height);
    cairo_line_to(ct, x, y + height);
    cairo_line_to(ct, x, y);
    cairo_close_path(ct);
    
    cairo_stroke(ct);

#endif
    
    cairo_surface_flush(dst);
    cairo_surface_destroy(dst);
    cairo_destroy(ct);
}

@end
