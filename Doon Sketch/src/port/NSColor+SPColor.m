//
//  NSColor+SPColor.m
//  Inxcape
//
//  Created by 张 光建 on 14-10-14.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "NSColor+SPColor.h"
#import <color.h>

@implementation NSColor (SPColor)

+ (NSColor *)colorWithSPColor:(SPColor const *)color alpha:(CGFloat)alpha
{
    guint32 v = color->toRGBA32(alpha);
    return [NSColor colorWithCalibratedRed:SP_COLOR_U_TO_F((v & 0xff000000) >> 24)
                                     green:SP_COLOR_U_TO_F((v & 0x00ff0000) >> 16)
                                      blue:SP_COLOR_U_TO_F((v & 0x0000ff00) >> 8)
                                     alpha:SP_COLOR_U_TO_F((v & 0x000000ff))];
}

- (SPColor *)spColor
{
    NSColor *rgb = [self colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
    SPColor *clr = new SPColor([rgb redComponent], [rgb greenComponent], [rgb blueComponent]);
    return clr;
}

@end
