//
//  AttributeColumn.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/29.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "AttributeColumn.h"
#import "ZGItem.h"
#import "ZGDesktop.h"

@interface AttributeColumn () {
    ZGDesktop *_desktop;
}

@end

@implementation AttributeColumn

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

- (BOOL)suitableForItem:(ZGItem *)zItem
{
    for (NSNumber *attr in [self class].allAttributes) {
        if (![[zItem class] hasAttribute:attr.unsignedIntValue]) {
            return FALSE;
        }
    }
    return TRUE;
}

- (BOOL)bindWithDesktop:(ZGDesktop *)desktop
{
    [desktop.objectsController addObserver:self
                                forKeyPath:@"selectedObjects"
                                   options:NSKeyValueObservingOptionNew
                                   context:nill];
    return TRUE;
}

- (void)unbind
{
    [self.desktop.objectsController removeObserver:self forKeyPath:@"selectedObjects"];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    BOOL suitable = FALSE;
    
    for (ZGItem *zItem in self.desktop.objectsController.selectedObjects) {
        if ([self suitableForItem:zItem]) {
            suitable = TRUE;
            break;
        }
    }
    [self.view setHidden:!suitable];
}

- (void)setDesktop:(ZGDesktop *)desktop
{
    [self unbind];
    
    _desktop = desktop;
    if (desktop) {
        [self bindWithDesktop:desktop];
    }
}

- (ZGDesktop *)desktop
{
    return _desktop;
}

@end
