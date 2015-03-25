//
//  ZGEllipse.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/26.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGShape.h"

struct SPGenericEllipse;

@interface ZGEllipse : ZGShape

@property CGFloat rx;
@property CGFloat ry;
@property CGFloat cx;
@property CGFloat cy;

- (id)initWithSPEllipse:(SPGenericEllipse *)ellipse;

- (SPGenericEllipse *)spEllipse;

@end
