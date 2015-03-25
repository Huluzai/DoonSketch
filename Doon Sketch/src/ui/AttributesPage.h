//
//  AttributesViewController.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-19.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

struct SPDesktop;


@interface AttributesPage : NSObject

@property SPDesktop *desktop;

@property (readonly) NSView *view;

- (id)initWithDesktop:(SPDesktop *)desktop;

- (void)addAdjustmentWithAttribute:(NSUInteger)attrCode;

// add in a row
- (void)addBuddyAdjustments:(NSArray *)adjs;

// Todo: add other types support

- (void)sizeToFit;

- (void)updateValues;

@end

