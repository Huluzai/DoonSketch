//
//  ObjectsListView.m
//  Inxcape
//
//  Created by 张 光建 on 15/1/4.
//  Copyright (c) 2015年 Doonsoft. All rights reserved.
//

#import "ObjectsView.h"

@implementation ObjectsView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [super mouseDown:theEvent];
    if ([self.delegate respondsToSelector:@selector(objectsView:didClickTableRow:)]) {
        NSPoint pos = [self convertPoint:theEvent.locationInWindow fromView:nill];
        [(id)self.delegate objectsView:self didClickTableRow:[self rowAtPoint:pos]];
    }
}

- (void)mouseUp:(NSEvent *)theEvent
{
    if ([self.delegate respondsToSelector:@selector(objectsView:didClickTableRow:)]) {
        NSPoint pos = [self convertPoint:theEvent.locationInWindow fromView:nill];
        [(id)self.delegate objectsView:self didClickTableRow:[self rowAtPoint:pos]];
    }
}

@end
