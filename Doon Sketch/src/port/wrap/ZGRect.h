//
//  ZGRect.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/26.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGShape.h"

struct SPRect;

@interface ZGRect : ZGShape

@property CGFloat rx;
@property CGFloat ry;
@property CGFloat x;
@property CGFloat y;
@property CGFloat width;
@property CGFloat height;

- (id)initWithSPRect:(SPRect *)rect;

- (SPRect *)spRect;

@end
