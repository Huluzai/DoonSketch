//
//  GeomAttrbutesPage.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-20.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <sp-star.h>
#import <sp-rect.h>
#import <sp-line.h>
#import <verbs.h>
#import "GeomAttributesPage.h"
#import "ZGAttribute.h"

@implementation GeomAttributesPage

- (id)initWithDesktop:(SPDesktop*)desktop
{
    if (self = [super initWithDesktop:desktop]) {
        [self prepareForCommonAttributes];
    }
    return self;
}

- (void)prepareForCommonAttributes
{
    [self addAdjustmentWithAttribute:ZG_ATTR_POSITION];
    [self addAdjustmentWithAttribute:ZG_ATTR_SIZE];
    [self addAdjustmentWithAttribute:SP_ATTR_RX];
    [self addAdjustmentWithAttribute:SP_ATTR_RY];
    [self addAdjustmentWithAttribute:SP_ATTR_SODIPODI_START];
    [self addAdjustmentWithAttribute:SP_ATTR_SODIPODI_END];
    [self addAdjustmentWithAttribute:SP_ATTR_SODIPODI_OPEN];
    [self addAdjustmentWithAttribute:ZG_ATTR_TEXT];
    [self sizeToFit];
}

- (void)prepareForItem:(SPItem *)item
{
    
}

@end
