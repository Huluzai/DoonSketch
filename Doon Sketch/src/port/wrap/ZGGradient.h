//
//  ZGGradient.h
//  Inxcape
//
//  Created by 张 光建 on 14-10-14.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ZGObject.h"

struct SPGradient;

@interface ZGGradient : ZGObject
// no retain
- (id)initWithSPGradient:(SPGradient *)gr;
- (SPGradient *)spGradient;

+ (NSImage *)gradientImage:(SPGradient *)gradient;

@end


@interface ZGGradient (Delegate)

- (void)didGradientReleased:(ZGGradient *)gr;

@end
