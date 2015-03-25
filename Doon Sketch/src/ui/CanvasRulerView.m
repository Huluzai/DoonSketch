//
//  CanvasRulerView.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/24.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "CanvasRulerView.h"
#import "FixedCanvasView.h"

NSString *kCanvasRulerViewDraggedNotification = @"CanvasRulerViewDraggedNotification";

static const int ticksPerInch = 5;

@interface CanvasRulerView () {
    CGFloat _scale;
    CGFloat _originOffset;
    NSRulerOrientation _orientation;
}
@property BOOL dragging;
@end

@implementation CanvasRulerView

- (id)initWithOrientation:(NSRulerOrientation)orientation
{
    if (self = [super initWithFrame:CGRectZero]) {
        self.orientation = orientation;
        self.originOffset = 0;
        self.scale = 1;
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
    
    NSBezierPath *path = nill;
    
    double start = CGRectGetMinX(dirtyRect);
    double end = CGRectGetMaxX(dirtyRect);
    
    // base line
    [[NSColor blackColor] setStroke];
    path = [NSBezierPath bezierPath];
    [path setLineWidth:0.3];
    [path moveToPoint:CGPointMake(start, CGRectGetMidY(dirtyRect))];
    [path lineToPoint:CGPointMake(end, CGRectGetMidY(dirtyRect))];
    [path stroke];
    
    CGPoint clientOrg = [self.clientView convertPoint:dirtyRect.origin fromView:self];
    CGFloat clientPos = self.orientation == NSHorizontalRuler ? clientOrg.x : clientOrg.y;
    CGFloat startPos = clientPos + self.originOffset;
    CGFloat canvasPos = startPos * self.scale;
    
    int startMark = startPos / [self screenPointsPerMark] + 1;
    int endMark = (startPos + CGRectGetWidth(dirtyRect)) / [self screenPointsPerMark];
    int startFlag = startMark / [self marksPerFlag] + 1;
    
    NSFont *fnt = [NSFont systemFontOfSize:8];
    NSDictionary *attrs = [NSDictionary dictionaryWithObjectsAndKeys:fnt, NSFontAttributeName, nil];
    
    for (int i = startMark; i <= endMark; i++) {
        double pos = i * [self screenPointsPerMark] - self.originOffset;
        CGPoint localPos = self.orientation == NSHorizontalRuler ? CGPointMake(pos, 0) : CGPointMake(0, pos);
        CGPoint thisPos = [self convertPoint:localPos fromView:self.clientView];
        double top = (i % [self marksPerFlag] == 0) ? CGRectGetMinY(dirtyRect) : CGRectGetMidY(dirtyRect);
        path = [NSBezierPath bezierPath];
        [path setLineWidth:0.3f];
        [path moveToPoint:CGPointMake(thisPos.x, top)];
        [path lineToPoint:CGPointMake(thisPos.x, CGRectGetMidY(dirtyRect) + 4)];
        [[NSColor blackColor] setStroke];
        [path stroke];
        
        if (i % [self marksPerFlag] == 0) {
            int flag = self.orientation == NSVerticalRuler ? -1 : 1;    //todo:
            NSString *flagText = [NSString stringWithFormat:@"%d", i * flag * (int)([self canvasPointsPerMark] + 0.5)];
            [flagText drawAtPoint:CGPointMake(thisPos.x + 2, top) withAttributes:attrs];
        }
    }
}

- (void)setScale:(CGFloat)scale
{
    _scale = scale;
    [self setNeedsDisplay:TRUE];
}

- (CGFloat)scale
{
    return _scale;
}

- (void)setOrientation:(NSRulerOrientation)orientation
{
    _orientation = orientation;
    [self setBoundsRotation:(orientation == NSVerticalRuler ? -90 : 0)];
    [self setNeedsDisplay:TRUE];
}

- (NSRulerOrientation)orientation
{
    return _orientation;
}

- (void)setOriginOffset:(CGFloat)originOffset
{
    _originOffset = originOffset;
    [self setNeedsDisplay:TRUE];
}

- (CGFloat)originOffset
{
    return _originOffset;
}

- (BOOL)isFlipped
{
    return self.orientation == NSHorizontalRuler ? TRUE : FALSE;
}

- (int)marksPerFlag
{
    const int respectMarksPerFlag = 5;
    double pointsPerFlag = respectMarksPerFlag * [self canvasPointsPerMark];
    pointsPerFlag = MAX(1, pointsPerFlag);
    return pointsPerFlag / [self canvasPointsPerMark];
}

- (CGFloat)screenPointsPerMark
{
    CGFloat cppm = [self canvasPointsPerMark];
    return cppm * self.scale;
}

- (CGFloat)canvasPointsPerMark
{
    const CGFloat respectScreenPointsPerMark = 20;
    double r = respectScreenPointsPerMark / self.scale;
    if (r > 1) {
        r = floor(r);
    }
    return r;
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    NSDictionary *info = [NSDictionary dictionaryWithObject:[NSValue valueWithSize:CGSizeMake(theEvent.deltaX, theEvent.deltaY)] forKey:@"offset"];
    [defaultNfc postNotificationName:kCanvasRulerViewDraggedNotification object:self userInfo:info];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [[NSCursor closedHandCursor] set];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [[NSCursor openHandCursor] set];
}

- (void)resetCursorRects
{
    [self addCursorRect:self.visibleRect cursor:[NSCursor openHandCursor]];
}

@end
