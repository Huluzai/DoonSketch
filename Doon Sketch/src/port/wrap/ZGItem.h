//
//  ZGItem.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/26.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGObject.h"

#define DEFINE_ATTRIBUTE_METHODS  \
static NSArray *attributes = nill;  \
+ (void)registerAttribute:(unsigned int)attr  \
{   \
    if (!attributes) {  \
        attributes = [NSArray array];    \
    }   \
    g_debug("register attribute %d to %p", attr, attributes);\
    if (![self hasAttribute:attr]) {   \
        attributes = [attributes arrayByAddingObject:[NSNumber numberWithUnsignedInt:attr]];\
    }   \
}   \
+ (NSArray *)allAttributes  \
{\
    if ([super respondsToSelector:@selector(allAttributes)]) { \
        return [attributes arrayByAddingObjectsFromArray:[super allAttributes]];\
    }\
    return attributes;\
}\
+ (BOOL)hasAttribute:(unsigned int)attr\
{\
    return [attributes containsObject:[NSNumber numberWithUnsignedInteger:attr]];\
}

struct SPItem;

@interface ZGItem : ZGObject

//@property CGFloat x;
//@property CGFloat y;
//@property CGFloat width;
//@property CGFloat height;

+ (BOOL)hasAttribute:(unsigned int)attr;

+ (NSArray *)allAttributes;

+ (void)registerAttribute:(unsigned int)attr;

- (id)initWithSPItem:(SPItem *)item;

@end
