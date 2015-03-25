//
//  AttributeColumn.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/27.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class ZGDesktop;
@class ZGItem;

@interface AttributeColumn : NSViewController

@property ZGDesktop *desktop;

+ (BOOL)hasAttribute:(unsigned int)attr;
+ (NSArray *)allAttributes;
+ (void)registerAttribute:(unsigned int)attr;
- (BOOL)suitableForItem:(ZGItem *)zItem;
@end

@interface AttributeColumn (Override)
- (BOOL)bindWithDesktop:(ZGDesktop *)desktop;
- (void)unbind;
@end