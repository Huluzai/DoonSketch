//
//  ZGBox.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-20.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGBox.h"

@interface ZGBox ()
@property CGPoint forewardPos;
@property CGPoint backwardPos;
@property NSMutableArray *fwChildren;
@property NSMutableArray *bwChildren;
@end

@implementation ZGBox

- (id)initWithView:(NSView *)anView
{
    if (self = [super initWithView:anView]) {
        self.forewardPos = CGPointZero;
        self.backwardPos = CGPointMake(CGRectGetMaxX(anView.bounds), CGRectGetMaxY(anView.bounds));
        self.fwChildren = [NSMutableArray array];
        self.bwChildren = [NSMutableArray array];
        self.space = 8;
    }
    return self;
}

- (void)dealloc
{
    self.fwChildren = nill;
    self.bwChildren = nill;
}

- (void)relayout
{
    self.forewardPos = CGPointZero;
    self.backwardPos = CGPointMake(CGRectGetMaxX(self.targetView.bounds),
                                   CGRectGetMaxY(self.targetView.bounds));
    
    for (NSView *child in self.fwChildren) {
        [self packStart:child];
    }
    
    for (NSView *child in self.bwChildren) {
        [self packEnd:child];
    }
    
    NSView *headView = [self.fwChildren firstObject];
    if (!headView) {
        [self.bwChildren lastObject];
    }
}

- (void)packStart:(NSView *)anView
{
    if ([anView isHidden]) {
        return;
    }
    
    [anView setFrameOrigin:self.forewardPos];
    
    CGPoint pt = self.forewardPos;
    pt.x += [self increment].width * (anView.frame.size.width + self.space);
    pt.y += [self increment].height * (anView.frame.size.height + self.space);
    self.forewardPos = pt;
    
    if ([self.fwChildren indexOfObject:anView] == NSNotFound) {
        [super add:anView];
        [self.fwChildren addObject:anView];
    }
}

- (void)packEnd:(NSView *)anView
{
    if ([anView isHidden]) {
        return;
    }
    
    CGPoint pos = self.backwardPos;
    pos.x -= [self increment].width * anView.frame.size.width;
    pos.y -= [self increment].height * anView.frame.size.height;
    
    [anView setFrameOrigin:pos];
    
    pos.x -= [self increment].width * self.space;
    pos.y -= [self increment].height * self.space;
    self.backwardPos = pos;
    
    if ([self.bwChildren indexOfObject:anView] == NSNotFound) {
        [self add:anView];
        [self.bwChildren addObject:anView];
    }
}

- (void)sizeToFit
{
    CGRect childrenFrame = CGRectZero;
    
    for (NSView *child in self.fwChildren) {
        if (![child isHidden]) {
            childrenFrame = CGRectUnion(child.frame, childrenFrame);
        }
    }
    
    for (NSView *child in self.bwChildren) {
        if (![child isHidden]) {
            childrenFrame = CGRectUnion(childrenFrame, child.frame);
        }
    }
    
    [self.targetView setFrameSize:childrenFrame.size];
}

@end


@implementation ZGHBox

- (CGSize)increment
{
    return CGSizeMake(1, 0);
}

- (void)packStart:(NSView *)view
{
    [super packStart:view];
    CGRect frame = view.frame;
    frame.origin.y = (self.targetView.bounds.size.height - frame.size.height) / 2;
    [view setFrame:frame];
}

- (void)packEnd:(NSView *)view
{
    [super packEnd:view];
    CGRect frame = view.frame;
    frame.origin.y = (self.targetView.bounds.size.height - frame.size.height) / 2;
    [view setFrame:frame];
}

@end


@implementation ZGVBox

- (CGSize)increment
{
    return CGSizeMake(0, 1);
}

@end
