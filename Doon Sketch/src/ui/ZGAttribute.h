//
//  ZGAttribute.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-23.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <sp-object.h>
#import <sp-item.h>
#import <attributes.h>

enum {
    ZG_ATTR_FIRST = SP_PROP_PATH_EFFECT + 1,
    ZG_ATTR_POSITION = ZG_ATTR_FIRST,
    ZG_ATTR_SIZE,
    ZG_ATTR_TEXT,
    ZG_ATTR_COUNT
};


@interface ZGAttribute : NSObject

@property NSUInteger code;

- (void)setValue:(id)value ofItem:(SPItem *)item;
- (id)valueOfItem:(SPItem *)item;
- (BOOL)isValidValue:(id)value;

+ (BOOL)itemClass:(GType)classType hasAttritube:(NSUInteger)code;
+ (id)attributeWithCode:(NSUInteger)code;

@end

const NSString *attributeNameWithCode(NSUInteger attrCode);