//
//  ZGStyle.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/5.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <style.h>

struct SPDesktop;
struct SPDocument;
@class ZGDesktop;

@interface ZGDesktopStyle : NSObject

@property (copy) NSString *fontFamily;
@property CGFloat fontSize;
@property BOOL fontBold;
@property BOOL fontItalic;
@property NSTextAlignment textAlign;
@property CGFloat lineHeight;
@property CGFloat letterSpacing;
@property CGFloat wordSpacing;
@property CGFloat verticalShift;
@property CGFloat horizontalShift;
@property CGFloat characterRotation;
@property BOOL textOrientation;
@property BOOL superscript;
@property BOOL subscript;

- (id)initWithDesktop:(SPDesktop *)desktop;

+ (NSArray *)availableFontSizes;

@end
