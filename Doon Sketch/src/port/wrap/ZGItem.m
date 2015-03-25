//
//  ZGItem.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/26.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGItem.h"
#import "ZGObject-Private.h"
#import "ZGShape.h"

#import <sp-shape.h>
#import <attributes.h>

@implementation ZGItem

static NSArray *attributes = nill;  \
+ (void)registerAttribute:(unsigned int)attr  \
{   \
    if (!attributes) {  \
        attributes = [NSArray array];    \
    }   \
    if (![self hasAttribute:attr]) {   \
        attributes = [attributes arrayByAddingObject:[NSNumber numberWithUnsignedInt:attr]];\
    }   \
}   \
+ (NSArray *)allAttributes  \
{\
    return attributes;\
}\
+ (BOOL)hasAttribute:(unsigned int)attr\
{\
    return [attributes containsObject:[NSNumber numberWithUnsignedInteger:attr]];\
}

+ (void)initialize
{
//    [self registerAttribute:SP_ATTR_WIDTH];
//    [self registerAttribute:SP_ATTR_HEIGHT];
//    [self registerAttribute:SP_ATTR_X];
//    [self registerAttribute:SP_ATTR_Y];
}

- (id)initWithSPItem:(SPItem *)item
{
    if (SP_IS_SHAPE(item)) {
        return [[ZGShape alloc] initWithSPShape:SP_SHAPE(item)];
    }

    return [super initWithSPObject:SP_OBJECT(item) takeCopy:FALSE];
}

@end
