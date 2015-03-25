//
//  CanvasScrollView.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/19.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "CanvasScrollView.h"
#import "sp-canvas-cocoa.h"
#import "AbsoluteValueScroller.h"
#import "FixedCanvasView.h"
#import "CanvasRulerView.h"

#import <2geom/geom.h>
#import <gdk/gdk.h>

@interface CanvasScrollView () {
    CGPoint _currentPos;
    AbsoluteValueScroller *_verticalScroller;
    AbsoluteValueScroller *_horizontalScroller;
    CanvasRulerView *_verticalRulerView;
    CanvasRulerView *_horizontalRulerView;
    BOOL _hasHorizontalRuler;
    BOOL _hasVerticalRuler;
    BOOL _hasHorizontalScroller;
    BOOL _hasVerticalScroller;
    BOOL _rulersVisible;
    FixedCanvasView *_documentView;
};

@end


@implementation CanvasScrollView

- (instancetype)initWithFrame:(NSRect)frameRect
{
    if (self = [super initWithFrame:frameRect]) {
    }
    
    return self;
}

- (void)dealloc
{
    [defaultNfc removeObserver:self];
    self.verticalScroller = nill;
    self.horizontalScroller = nill;
    self.documentView = nill;
    self.verticalRulerView = nill;
    self.horizontalRulerView = nill;
}

- (void)awakeFromNib
{
    self.verticalScroller = [[AbsoluteValueScroller alloc] initWithFrame:CGRectMake(0, 0, 20, 100)];
    self.horizontalScroller = [[AbsoluteValueScroller alloc] initWithFrame:CGRectMake(0, 0, 100, 20)];
    self.verticalRulerView = [[CanvasRulerView alloc] initWithOrientation:NSVerticalRuler];
    self.horizontalRulerView = [[CanvasRulerView alloc] initWithOrientation:NSHorizontalRuler];
    self.hasHorizontalScroller = FALSE;
    self.hasVerticalScroller = FALSE;
    self.rulersVisible = TRUE;
    self.hasVerticalRuler = TRUE;
    self.hasHorizontalRuler = TRUE;
    self.documentView = [[FixedCanvasView alloc] initWithFrame:CGRectZero];
    
    [self.horizontalRulerView setClientView:self.documentView];
    [self.verticalRulerView setClientView:self.documentView];
    
    [defaultNfc addObserver:self
                   selector:@selector(didRulerViewDragged:)
                       name:kCanvasRulerViewDraggedNotification
                     object:nill];
    
    [defaultNfc addObserver:self
                   selector:@selector(didDocumentViewChangedOrigin:)
                       name:kFixedCanvasChangedOriginNotification
                     object:nill];
}

- (BOOL)isFlipped
{
    return TRUE;
}

- (void)setRulersVisible:(BOOL)rulersVisible
{
    _rulersVisible = rulersVisible;
    self.verticalRulerView.hidden = !rulersVisible;
    self.horizontalRulerView.hidden = !rulersVisible;
    [self tile];
}

- (BOOL)rulersVisible
{
    return _rulersVisible;
}

- (void)didRulerViewDragged:(NSNotification *)aNotification
{
    CGPoint origin = self.documentView.origin;
    NSValue *val = [aNotification.userInfo objectForKey:@"offset"];
    
    if (aNotification.object == self.horizontalRulerView && val) {
        origin.x -= val.pointValue.x;
    } else if (aNotification.object == self.verticalRulerView && val) {
        origin.y -= val.pointValue.y;
    }
    self.documentView.origin = origin;
}

- (void)didDocumentViewChangedOrigin:(NSNotification *)aNotification
{
    if (aNotification.object == self.documentView) {
        self.verticalScroller.absValue = self.documentView.origin.y;
        self.horizontalScroller.absValue = self.documentView.origin.x;
        self.horizontalRulerView.originOffset = self.documentView.origin.x;
        self.verticalRulerView.originOffset = self.documentView.origin.y;
    }
}

- (void)setDocumentView:(FixedCanvasView *)documentView
{
    [_documentView removeFromSuperview];
    _documentView = documentView;
    [self addSubview:documentView];
    [self tile];
}

- (FixedCanvasView *)documentView
{
    return _documentView;
}

- (void)setHasHorizontalScroller:(BOOL)hasHorizontalScroller
{
    self.horizontalScroller.hidden = !hasHorizontalScroller;
    _hasHorizontalScroller = hasHorizontalScroller;
    [self tile];
}

- (BOOL)hasHorizontalScroller
{
    return _hasHorizontalScroller;
}

- (void)setHasVerticalScroller:(BOOL)hasVerticalScroller
{
    self.verticalScroller.hidden = !hasVerticalScroller;
    _hasVerticalScroller = hasVerticalScroller;
    [self tile];
}

- (BOOL)hasVerticalScroller
{
    return _hasVerticalScroller;
}

- (void)setHasHorizontalRuler:(BOOL)hasHorizontalRuler
{
    _hasHorizontalRuler = hasHorizontalRuler;
    [self tile];
}

- (BOOL)hasHorizontalRuler
{
    return _hasHorizontalRuler;
}

- (void)setHasVerticalRuler:(BOOL)hasVerticalRuler
{
    _hasVerticalRuler = hasVerticalRuler;
    [self tile];
}

- (BOOL)hasVerticalRuler
{
    return _hasVerticalRuler;
}

- (void)setVerticalScroller:(AbsoluteValueScroller *)verticalScroller
{
    [_verticalScroller removeFromSuperview];
    [_verticalScroller removeObserver:self forKeyPath:@"absValue"];
    
    _verticalScroller = verticalScroller;
    
    if (verticalScroller) {
        verticalScroller.enabled = TRUE;
        
        [self addSubview:verticalScroller];
        [verticalScroller addObserver:self
                           forKeyPath:@"absValue"
                              options:NSKeyValueObservingOptionNew
                              context:NULL];
    }
    [self tile];
}

- (NSScroller *)verticalScroller
{
    return _verticalScroller;
}

- (void)setHorizontalScroller:(AbsoluteValueScroller *)horizontalScroller
{
    [_horizontalScroller removeFromSuperview];
    [_horizontalScroller removeObserver:self forKeyPath:@"absValue"];

    _horizontalScroller = horizontalScroller;
    
    if (horizontalScroller) {
        horizontalScroller.enabled = TRUE;
        
        [self addSubview:horizontalScroller];
        [horizontalScroller addObserver:self
                             forKeyPath:@"absValue"
                                options:NSKeyValueObservingOptionNew
                                context:NULL];
    }
    
    [self tile];
}

- (NSScroller *)horizontalScroller
{
    return _horizontalScroller;
}

- (void)setHorizontalRulerView:(CanvasRulerView *)horizontalRulerView
{
    [_horizontalRulerView removeFromSuperview];
    
    _horizontalRulerView = horizontalRulerView;
    if (horizontalRulerView) {
        [self addSubview:horizontalRulerView];
    }
    [self tile];
}

- (CanvasRulerView *)horizontalRulerView
{
    return _horizontalRulerView;
}

- (void)setVerticalRulerView:(CanvasRulerView *)verticalRulerView
{
    [_verticalRulerView removeFromSuperview];
    
    _verticalRulerView = verticalRulerView;
    if (verticalRulerView) {
        [self addSubview:verticalRulerView];
    }
    [self tile];
}

- (CanvasRulerView *)verticalRulerView
{
    return _verticalRulerView;
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    if ([keyPath isEqualTo:@"absValue"]) {
        if (object == self.horizontalScroller) {
            self.documentView.origin = CGPointMake(self.horizontalScroller.absValue, self.documentView.origin.y);
        } else if (object == self.verticalScroller) {
            self.documentView.origin = CGPointMake(self.documentView.origin.x, self.verticalScroller.absValue);
        }
    }
}

- (void)tile
{
    const CGFloat scrollerWidth = [NSScroller scrollerWidthForControlSize:NSRegularControlSize scrollerStyle:NSScrollerStyleLegacy];
    const CGFloat rulerWidth = 20;
    const BOOL showVR = self.hasVerticalRuler && self.rulersVisible;
    const BOOL showHR = self.hasHorizontalRuler && self.rulersVisible;
    
    CGRect r = self.bounds;
    r.origin.y = CGRectGetMaxY(r) - scrollerWidth;
    r.size.height = scrollerWidth;
    self.horizontalScroller.frame = r;
    
    r = self.bounds;
    r.origin.x = CGRectGetMaxX(r) - scrollerWidth;
    r.size.width = scrollerWidth;
    self.verticalScroller.frame = r;
    
    self.horizontalScroller.autoresizingMask = NSViewWidthSizable | NSViewMaxXMargin | NSViewMinXMargin | NSViewMinYMargin;
    self.verticalScroller.autoresizingMask = NSViewHeightSizable | NSViewMaxYMargin | NSViewMinYMargin | NSViewMinXMargin;
    
    r = self.bounds;
    r.size.height = rulerWidth;
    self.horizontalRulerView.frame = r;
    self.horizontalRulerView.autoresizingMask = NSViewWidthSizable | NSViewMaxYMargin;
    
    r = self.bounds;
    r.size.width = rulerWidth;
    self.verticalRulerView.frame = r;
    self.verticalRulerView.autoresizingMask = NSViewHeightSizable | NSViewMaxXMargin;
    
    r = self.bounds;
    r.origin.x += showVR ? rulerWidth : 0;
    r.origin.y += showHR ? rulerWidth : 0;
    r.size.width -= (self.hasVerticalScroller ? scrollerWidth : 0) + (showVR ? rulerWidth : 0);
    r.size.height -= (self.hasHorizontalScroller ? scrollerWidth : 0) + (showHR ? rulerWidth : 0);
    self.documentView.frame = r;
    self.documentView.autoresizingMask = NSViewHeightSizable | NSViewWidthSizable | NSViewMinYMargin | NSViewMinXMargin | NSViewMaxXMargin | NSViewMaxYMargin;
    
    [self setFrameSize:self.frame.size];
}

- (CGPoint)convertPointFromCanvas:(CGPoint)point
{
    return CGPointMake(point.x - _currentPos.x, point.y - _currentPos.y);
}

@end
