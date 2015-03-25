//
//  ZGAdjustment.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-20.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Foundation/Foundation.h>

enum {
    ZG_INT = 0,
    ZG_FLOAT,
    ZG_POINT,
    ZG_SIZE,
    ZG_FONT,
    ZG_COLOR,
    ZG_STRING
};
typedef NSUInteger ZGAdjustmentType;

/*
enum {
    ZG_VALUE_NOT_SET,
    ZG_VALUE_SET,
    ZG_VALUE_MULTI,
};
typedef NSUInteger ZGValueState;
*/
@class ZGNumberAdjustment;


@interface ZGAdjustment : NSViewController

@property (copy) NSString *name;
@property id target;
@property SEL selector;
@property id object;
@property id value;

@end


@interface ZGAdjustment (Override)

- (NSString *)nibName;

- (void)clearValue;

- (void)setFloatValue:(CGFloat)floatValue;
- (CGFloat)floatValue;

- (void)setPointValue:(CGPoint)pointValue;
- (CGPoint)pointValue;

- (void)setSizeValue:(CGSize)sizeValue;
- (CGSize)sizeValue;

- (void)setColorValue:(NSColor *)colorValue;
- (NSColor *)colorValue;

- (void)setFontValue:(NSFont *)fontValue;
- (NSFont *)fontValue;

- (void)setStringValue:(NSString *)stringValue;
- (NSString *)stringValue;

- (NSView *)buildControls;

@end


@protocol ZGAdjustmentDelegate <NSObject>

- (void)didAdjustmentValueChanged:(ZGAdjustment *)adjustment;

@end


@interface ZGNumberAdjustment : ZGAdjustment

@property IBOutlet NSTextField *valueEdit;
@property IBOutlet NSNumberFormatter *formatter;
@property IBOutlet NSSlider *slider;

@property CGFloat floatValue;

- (id)initWithDefaultValue:(CGFloat)value;

- (id)initWithDefaultValue:(CGFloat)value
                     lower:(CGFloat)lower
                     upper:(CGFloat)upper
                 increment:(CGFloat)increment;

@end


@interface ZGPointAdjustment : ZGAdjustment

@property IBOutlet NSNumberFormatter *xFormatter;
@property IBOutlet NSNumberFormatter *yFormatter;

@property CGFloat x;
@property CGFloat y;

- (id)initWithDefaultValue:(CGPoint)point
             inBoundingBox:(CGRect)bbox;

@end


@interface ZGSizeAdjustment : ZGAdjustment

@property IBOutlet NSNumberFormatter *widthFormatter;
@property IBOutlet NSNumberFormatter *heightFormatter;

@property CGFloat x;
@property CGFloat y;

- (id)initWithDefaultValue:(CGSize)size
                 upperSize:(CGSize)upper;

@end


@interface ZGBoolAdjustment : ZGAdjustment

@property BOOL boolValue;

- (id)initWithDefaultValue:(BOOL)value;

@end
