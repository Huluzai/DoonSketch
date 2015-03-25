//
//  AbsoluteValueScroller.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/24.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AbsoluteValueScroller : NSScroller

@property CGFloat absLower;
@property CGFloat absUpper;
@property CGFloat absValue;

@end
