//
//  ZGAdjustment.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-20.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGAdjustment.h"
#import "ZGBox.h"
#import <glib.h>

@interface ZGLabel : NSTextField
@end

@implementation ZGLabel

- (id)initWithFrame:(NSRect)frameRect
{
    if (self = [super initWithFrame:frameRect]) {
        [self setEditable:NO];
        [self setSelectable:NO];
        [self setBezeled:NO];
    }
    return self;
}

@end


@interface ZGAdjustment ()
- (void)didChangeValue:(id)sender;
@end

@implementation ZGAdjustment

- (id)init
{
    return [super initWithNibName:[self nibName] bundle:[NSBundle mainBundle]];
}

- (void)awakeFromNib
{}

- (void)didChangeValue:(id)sender
{
    [self.target performSelector:self.selector withObject:sender];
}

- (void)clearValue
{}

@end


@interface ZGNumberAdjustment () {
    CGFloat _floatValue;
}

@property CGFloat lower;
@property CGFloat upper;
@property CGFloat stepIncrement;
@property CGFloat pageIncrement;
@property CGFloat defaultValue;

@end


@implementation ZGNumberAdjustment

- (id)initWithDefaultValue:(CGFloat)value
{
    return [self initWithDefaultValue:value
                                lower:NSIntegerMax
                                upper:NSIntegerMin
                            increment:NSIntegerMax];
}

- (id)initWithDefaultValue:(CGFloat)value
                     lower:(CGFloat)lower
                     upper:(CGFloat)upper
                 increment:(CGFloat)increment
{
    if (self = [super init]) {
        self.defaultValue = value;
        self.lower = lower;
        self.upper = upper;
        self.stepIncrement = increment;
    }
    return self;
}

- (void)awakeFromNib
{
    if (self.lower < self.upper) {
        [self.formatter setMinimum:[NSNumber numberWithFloat:self.lower]];
        [self.formatter setMaximum:[NSNumber numberWithFloat:self.upper]];
        [self.slider setMinValue:self.lower];
        [self.slider setMaxValue:self.upper];
    }
}

- (void)clearValue
{
    [self willChangeValueForKey:@"floatValue"];
    _floatValue = ZGFLOAT_NOT_SET;
    [self didChangeValueForKey:@"floatValue"];
}

- (void)setValue:(id)value
{
    [self willChangeValueForKey:@"floatValue"];
    if (ZGFloatEqualToFloat(self.floatValue, ZGFLOAT_NOT_SET)) {
        _floatValue = [(NSNumber *)value floatValue];
    } else if (!ZGFloatEqualToFloat(self.floatValue, [(NSNumber *)value floatValue])) {
        _floatValue = ZGFLOAT_MULTIPLE;
    }
    [self didChangeValueForKey:@"floatValue"];
}

- (id)value
{
    return [NSNumber numberWithFloat:self.floatValue];
}

- (NSString *)nibName
{
    return @"NumberAdjustment";
}
 
- (CGFloat)floatValue
{
    return _floatValue;
}

- (void)setFloatValue:(CGFloat)value
{
    if (!ZGFloatEqualToFloat(_floatValue, value)) {
        _floatValue = value;
        [self didChangeValue:self];
    }
}

@end


@interface ZGPointAdjustment () {
    CGPoint _pointValue;
}
@property CGRect bounds;
@property CGPoint defaultValue;
@property CGPoint pointValue;
@end


@implementation ZGPointAdjustment

- (id)initWithDefaultValue:(CGPoint)point
             inBoundingBox:(CGRect)bbox;
{
    if (self = [super init]) {
        self.bounds = bbox;
        self.defaultValue = point;
    }
    return self;
}

- (NSString *)nibName
{
    return @"PointAdjustment";
}

- (void)awakeFromNib
{
    if (!CGRectEqualToRect(self.bounds, CGRectZero)) {
        [self.xFormatter setMinimum:[NSNumber numberWithFloat:self.bounds.origin.x]];
        [self.xFormatter setMaximum:[NSNumber numberWithFloat:CGRectGetMaxX(self.bounds)]];
        
        [self.yFormatter setMinimum:[NSNumber numberWithFloat:self.bounds.origin.y]];
        [self.yFormatter setMaximum:[NSNumber numberWithFloat:CGRectGetMaxY(self.bounds)]];
    }
}

- (void)setX:(CGFloat)x
{
    self.pointValue = CGPointMake(x, self.pointValue.y);
}

- (CGFloat)x
{
    return self.pointValue.x;
}

- (void)setY:(CGFloat)y
{
    self.pointValue = CGPointMake(self.pointValue.x, y);
}

- (CGFloat)y
{
    return self.pointValue.y;
}

- (void)setValue:(id)value
{
    CGPoint newSize = [(NSValue *)value pointValue];
    CGFloat x, y;
    
    [self willChangeValueForKey:@"pointValue"];
    [self willChangeValueForKey:@"x"];
    [self willChangeValueForKey:@"y"];
    
    if (ZGFloatEqualToFloat(self.x, ZGFLOAT_NOT_SET)) {
        x = newSize.x;
    } else if (!ZGFloatEqualToFloat(self.x, newSize.x)) {
        x = ZGFLOAT_MULTIPLE;
    }
    
    if (ZGFloatEqualToFloat(self.y, ZGFLOAT_NOT_SET)) {
        y = newSize.y;
    } else if (!ZGFloatEqualToFloat(self.y, newSize.y)) {
        y = ZGFLOAT_MULTIPLE;
    }
    
    _pointValue = CGPointMake(x, y);
    
    [self didChangeValueForKey:@"pointValue"];
    [self didChangeValueForKey:@"x"];
    [self didChangeValueForKey:@"y"];
}

- (id)value
{
    return [NSValue valueWithPoint:self.pointValue];
}

- (void)setPointValue:(CGPoint)newSize
{
    if (!ZGPointEqualToPoint(newSize, _pointValue)) {
        [self willChangeValueForKey:@"x"];
        [self willChangeValueForKey:@"y"];
        _pointValue = newSize;
        [self didChangeValueForKey:@"x"];
        [self didChangeValueForKey:@"y"];
        [self didChangeValue:self];
    }
}

- (CGPoint)pointValue
{
    return _pointValue;
}

- (void)clearValue
{
    [self willChangeValueForKey:@"pointValue"];
    [self willChangeValueForKey:@"x"];
    [self willChangeValueForKey:@"y"];
    _pointValue = CGPointMake(ZGFLOAT_NOT_SET, ZGFLOAT_NOT_SET);
    [self didChangeValueForKey:@"x"];
    [self didChangeValueForKey:@"y"];
    [self didChangeValueForKey:@"pointValue"];
}

@end


@interface ZGSizeAdjustment () {
    CGSize _sizeValue;
}
@property CGSize defaultValue;
@property CGSize upperSize;
@property CGSize sizeValue;
@end

@implementation ZGSizeAdjustment

- (id)initWithDefaultValue:(CGSize)size
                 upperSize:(CGSize)upperSize;
{
    if (self = [super init]) {
        self.upperSize = upperSize;
        self.defaultValue = size;
    }
    return self;
}

- (NSString *)nibName
{
    return @"SizeAdjustment";
}

- (void)awakeFromNib
{
    [self.widthFormatter setMinimum:[NSNumber numberWithFloat:1.f]];
    [self.heightFormatter setMinimum:[NSNumber numberWithFloat:1.f]];
    
    if (!ZGSizeEqualToSize(self.upperSize, CGSizeZero)) {
        [self.widthFormatter setMaximum:[NSNumber numberWithFloat:self.upperSize.width]];
        [self.heightFormatter setMaximum:[NSNumber numberWithFloat:self.upperSize.height]];
    }
}

- (void)setX:(CGFloat)x
{
    self.sizeValue = CGSizeMake(x, self.sizeValue.height);
}

- (CGFloat)x
{
    return self.sizeValue.width;
}

- (void)setY:(CGFloat)y
{
    self.sizeValue = CGSizeMake(self.sizeValue.width, y);
}

- (CGFloat)y
{
    return self.sizeValue.height;
}

- (void)clearValue
{
    [self willChangeValueForKey:@"sizeValue"];
    [self willChangeValueForKey:@"x"];
    [self willChangeValueForKey:@"y"];
    _sizeValue = CGSizeMake(ZGFLOAT_NOT_SET, ZGFLOAT_NOT_SET);
    [self didChangeValueForKey:@"x"];
    [self didChangeValueForKey:@"y"];
    [self didChangeValueForKey:@"sizeValue"];
}

- (void)setSizeValue:(CGSize)newSize
{
    if (!ZGSizeEqualToSize(newSize, _sizeValue)) {
        
        [self willChangeValueForKey:@"x"];
        [self willChangeValueForKey:@"y"];
        _sizeValue = newSize;
        [self didChangeValueForKey:@"x"];
        [self didChangeValueForKey:@"y"];
        
        [self didChangeValue:self];
    }
}

- (CGSize)sizeValue
{
    return _sizeValue;
}

- (void)setValue:(id)value
{
    CGSize newSize = [(NSValue *)value sizeValue];
    CGFloat x, y;
    
    [self willChangeValueForKey:@"sizeValue"];
    [self willChangeValueForKey:@"x"];
    [self willChangeValueForKey:@"y"];
    
    if (ZGFloatEqualToFloat(self.x, ZGFLOAT_NOT_SET)) {
        x = newSize.width;
    } else if (!ZGFloatEqualToFloat(self.x, newSize.width)) {
        x = ZGFLOAT_MULTIPLE;
    }
    
    if (ZGFloatEqualToFloat(self.y, ZGFLOAT_NOT_SET)) {
        y = newSize.height;
    } else if (!ZGFloatEqualToFloat(self.y, newSize.height)) {
        y = ZGFLOAT_MULTIPLE;
    }
    
    _sizeValue = CGSizeMake(x, y);
    
    [self didChangeValueForKey:@"sizeValue"];
    [self didChangeValueForKey:@"x"];
    [self didChangeValueForKey:@"y"];
}

- (id)value
{
    return [NSValue valueWithSize:self.sizeValue];
}

@end


@interface ZGBoolAdjustment () {
    BOOL _boolValue;
}
@property BOOL defaultValue;

@end

@implementation ZGBoolAdjustment

- (id)initWithDefaultValue:(BOOL)value
{
    if (self = [super init]) {
        self.boolValue = value;
        self.defaultValue = value;
    }
    return self;
}

- (void)clearValue
{
    [self willChangeValueForKey:@"boolValue"];
    _boolValue = self.defaultValue;
    [self didChangeValueForKey:@"boolValue"];
}

- (NSString *)nibName
{
    return @"BoolAdjustment";
}

- (void)setValue:(id)value
{
    [self willChangeValueForKey:@"boolValue"];
    _boolValue = [value boolValue];
    [self didChangeValueForKey:@"boolValue"];
}

- (id)value
{
    return [NSNumber numberWithBool:self.boolValue];
}

- (void)setBoolValue:(BOOL)boolValue
{
    if (_boolValue != boolValue) {
        _boolValue = boolValue;
        [self didChangeValue:self];
    }
}

- (BOOL)boolValue
{
    return _boolValue;
}

@end
