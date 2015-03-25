//
//  ZGAdjustment+Attribute.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-23.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGAdjustment+Attribute.h"
#import "TextParameter.h"

static ZGAdjustment* createPositionAdjustment();
static ZGAdjustment* createSizeAdjustment();
static ZGAdjustment* createRectRxAdjustment();
static ZGAdjustment* createRectRyAdjustment();
static ZGAdjustment* createArcOpenAdjustment();
static ZGAdjustment* createArcStartAdjustment();
static ZGAdjustment* createArcEndAdjustment();
static ZGAdjustment* createTextAdjustment();

@implementation ZGAdjustment(Attribute)

+ (ZGAdjustment *)adjustmentWithAttribute:(NSUInteger)code
{
    static ZGAdjustment *adjs[ZG_ATTR_COUNT];
    static BOOL initialized = NO;
    
    if (!initialized) {
        memset(adjs, 0, sizeof(adjs));
        adjs[ZG_ATTR_POSITION] = createPositionAdjustment();
        adjs[ZG_ATTR_SIZE] = createSizeAdjustment();
        adjs[ZG_ATTR_TEXT] = createTextAdjustment();
        adjs[SP_ATTR_RX] = createRectRxAdjustment();
        adjs[SP_ATTR_RY] = createRectRyAdjustment();
        adjs[SP_ATTR_SODIPODI_START] = createArcStartAdjustment();
        adjs[SP_ATTR_SODIPODI_END] = createArcEndAdjustment();
        adjs[SP_ATTR_SODIPODI_OPEN] = createArcOpenAdjustment();
        
        for (int i = 0; i < ZG_ATTR_COUNT; i++) {
            if (adjs[i]) {
                [adjs[i] loadView];
            }
        }
        initialized = YES;
    }
    
    return adjs[code];
}


@end


static ZGAdjustment* createPositionAdjustment()
{
    ZGAdjustment *adj = [[ZGPointAdjustment alloc] initWithDefaultValue:CGPointZero
                                                          inBoundingBox:CGRectZero];
    adj.name = @"Position";
    return adj;
}

static ZGAdjustment* createSizeAdjustment()
{
    ZGAdjustment *adj = [[ZGSizeAdjustment alloc] initWithDefaultValue:CGSizeZero
                                                             upperSize:CGSizeZero];
    adj.name = @"Size";
    return adj;
}

static ZGAdjustment* createRectRxAdjustment()
{
    ZGAdjustment *adj = [[ZGNumberAdjustment alloc] initWithDefaultValue:0];
    adj.name = @"Rx";
    return adj;
}

static ZGAdjustment* createRectRyAdjustment()
{
    ZGAdjustment *adj = [[ZGNumberAdjustment alloc] initWithDefaultValue:0];
    adj.name = @"Ry";
    return adj;
}

static ZGAdjustment* createArcStartAdjustment()
{
    ZGAdjustment *adj = [[ZGNumberAdjustment alloc] initWithDefaultValue:0];
    adj.name = @"Start";
    return adj;
}

static ZGAdjustment* createArcEndAdjustment()
{
    ZGAdjustment *adj = [[ZGNumberAdjustment alloc] initWithDefaultValue:0];
    adj.name = @"End";
    return adj;
}

static ZGAdjustment* createArcOpenAdjustment()
{
    ZGAdjustment *adj = [[ZGBoolAdjustment alloc] initWithDefaultValue:YES];
    adj.name = @"Close";
    return adj;
}

static ZGAdjustment* createTextAdjustment()
{
    ZGAdjustment *adj = [[TextParameter alloc] init];
    adj.name = @"Text";
    return adj;
}
