//
//  NSColor+SPColor.h
//  Inxcape
//
//  Created by 张 光建 on 14-10-14.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

struct SPColor;

@interface NSColor (SPColor)

+ (NSColor *)colorWithSPColor:(SPColor const *)color alpha:(CGFloat)alpha;

- (SPColor *)spColor;

@end
