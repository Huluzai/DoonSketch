//
//  ActiveInfoPanel.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/10.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ActiveInfoPanel.h"

@implementation ActiveInfoPanel

- (id)init
{
#ifdef USE_PANEL
    return [super initWithWindowNibName:@"ActiveInfoPanel"];
#else
    return [super initWithNibName:@"ActiveInfoBox" bundle:nill];
#endif
}

- (void)showContent:(NSString *)str atPoint:(CGPoint)point forView:(NSView *)view inWindow:(NSWindow *)parent forTime:(NSTimeInterval)showTime
{
    CGSize parentSize = CGSizeZero;
    
#ifndef USE_PANEL
    view = view ? view : parent.contentView;
#endif
    
    if (view) {
        parentSize = view.frame.size;
#ifdef USE_PANEL
        point = [view convertPoint:point toView:nill];
#else
        point = [view convertPoint:point toView:parent.contentView];
#endif
    } else {
        parentSize = parent.frame.size;
    }
    
    [self.textField setStringValue:str];
    [self.textField sizeToFit];
#ifdef USE_PANEL
    CGRect contentFrame = CGRectInset(self.textField.frame, -3, -3);
    CGRect frame = [self.window frameRectForContentRect:contentFrame];
#else
    CGRect frame = self.view.frame;
#endif
    CGFloat w = frame.size.width;
    CGFloat h = frame.size.height;
    CGFloat leftDis, rightDis, topDis, bottomDis;
    
    switch (self.xAlign) {
        case NSLeftTextAlignment:
            leftDis = 0;
            rightDis = w;
            break;
        case NSRightTextAlignment:
            leftDis = w;
            rightDis = 0;
            break;
        case NSCenterTextAlignment:
            leftDis = rightDis = w / 2;
            break;
        default:
            break;
    }
    
    switch (self.yAlign) {
        case NSLeftTextAlignment:
            topDis = h;
            bottomDis = 0;
            break;
        case NSRightTextAlignment:
            topDis = 0;
            bottomDis = h;
            break;
        case NSCenterTextAlignment:
            topDis = bottomDis = h / 2;
        default:
            break;
    }
    
    point.x = MIN(parentSize.width - rightDis, MAX(leftDis, point.x - leftDis));
    point.y = MIN(parentSize.height - bottomDis, MAX(topDis, point.y - topDis));
    
    frame.origin = point;
    
#ifdef USE_PANEL
    frame = [parent convertRectToScreen:frame];
    
    [[self.window parentWindow] removeChildWindow:self.window];
    [parent addChildWindow:self.window ordered:NSWindowAbove];
    [self showWindow:nill];
    [self.window setFrame:frame display:NO animate:NO];
    [[self.window animator] setAlphaValue:0.8];
    [self.window setIgnoresMouseEvents:YES];
#else
    [self.view removeFromSuperview];
    [view addSubview:self.view positioned:NSWindowAbove relativeTo:nill];
    [self.view setFrame:frame];
    [[self.view animator] setHidden:NO];
#endif
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(dismiss) object:nill];
    if (showTime != 0) {
        [self performSelector:@selector(dismiss) withObject:nill afterDelay:showTime];
    }
}

- (void)showContent:(NSString *)str forView:(NSView *)view inWindow:(NSWindow *)parent  forTime:(NSTimeInterval)showTime
{
    CGPoint point = CGPointZero;
    if (view) {
        point = CGPointMake(CGRectGetMidX(view.frame), CGRectGetMidY(view.frame));
    } else {
        CGRect r = parent.frame;
        r.origin = CGPointZero;
        point = CGPointMake(CGRectGetMidX(r), CGRectGetMidY(r));
    }
    [self showContent:str atPoint:point forView:view inWindow:parent forTime:showTime];
}

- (void)showContent:(NSString *)str atPoint:(CGPoint)point inWindow:(NSWindow *)parent forTime:(NSTimeInterval)showTime
{
    [self showContent:str atPoint:point forView:nill inWindow:parent forTime:showTime];
}

- (void)dismiss
{
#ifdef USE_PANEL
    [[self.window animator] setAlphaValue:0];
    [self performSelector:@selector(close) withObject:nill afterDelay:1];
#else
    [[self.view animator] setHidden:YES];
#endif
}

@end
