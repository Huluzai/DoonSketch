//
//  ZGShape.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/26.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGShape.h"
#import "ZGObject-Private.h"
#import "ZGEllipse.h"
#import "ZGRect.h"

#import <sp-rect.h>
#import <sp-ellipse.h>
#import <sp-shape.h>

@implementation ZGShape

- (id)initWithSPShape:(SPShape *)shape
{
    if (SP_IS_GENERICELLIPSE(shape)) {
        return [[ZGEllipse alloc] initWithSPEllipse:SP_GENERICELLIPSE(shape)];
    }
    
    if (SP_IS_RECT(shape)) {
        return [[ZGRect alloc] initWithSPRect:SP_RECT(shape)];
    }
    
    return [super initWithSPObject:SP_OBJECT(shape) takeCopy:FALSE];
}

@end
