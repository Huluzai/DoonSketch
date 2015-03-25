//
//  AbsoluteValueScroller.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/24.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "AbsoluteValueScroller.h"

@interface AbsoluteValueScroller () {
    CGFloat _absLower;
    CGFloat _absUpper;
    CGFloat _absValue;
}

@end

@implementation AbsoluteValueScroller

- (void)updateValue
{
    self.doubleValue = (_absValue - _absLower) / (_absUpper - _absLower);
}

- (void)setAbsLower:(CGFloat)absLower
{
    _absLower = absLower;
    [self updateValue];
}

- (CGFloat)absLower
{
    return _absLower;
}

- (void)setAbsUpper:(CGFloat)absUpper
{
    _absUpper = absUpper;
    [self updateValue];
}

- (CGFloat)absUpper
{
    return _absUpper;
}

- (void)setAbsValue:(CGFloat)absValue
{
    static BOOL _busy = FALSE;
    if (_busy) {
        return;
    }
    _busy = TRUE;
    _absValue = absValue;
    [self updateValue];
    _busy = FALSE;
}

- (CGFloat)absValue
{
    return _absValue;
}

- (void)setDoubleValue:(double)doubleValue
{
    [super setDoubleValue:doubleValue];
    self.absValue = self.doubleValue * (_absUpper - _absLower) + _absLower;
}

@end
