//
//  ZGNumberFormatter.m
//  Study
//
//  Created by 张 光建 on 14-9-26.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGNumberFormatter.h"
#import "define.h"

@implementation ZGNumberFormatter

- (NSString *)editingStringForObjectValue:(id)anObject
{
    if ([anObject isKindOfClass:[NSNumber class]]
        && (ZGFloatEqualToFloat([anObject floatValue], ZGFLOAT_NOT_SET)
            || (ZGFloatEqualToFloat([anObject floatValue], ZGFLOAT_MULTIPLE)))) {
            return @"";
    }
    return [super editingStringForObjectValue:anObject];
}

- (NSString *)stringForObjectValue:(id)obj
{
    if ([obj isKindOfClass:[NSNumber class]]) {
        if (ZGFloatEqualToFloat([obj floatValue], ZGFLOAT_NOT_SET)) {
            return @"Not set";
        } else if (ZGFloatEqualToFloat([obj floatValue], ZGFLOAT_MULTIPLE)) {
            return @"Multiple";
        }
    }
    
    return [super stringForObjectValue:obj];
}

@end
