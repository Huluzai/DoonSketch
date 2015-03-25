//
//  GeomPropertiesPage.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/26.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "AttributeColumn.h"

@class ZGDesktop;

@interface GeomPropertiesPage : NSViewController

@property IBOutlet AttributeColumn *positionColumn;
@property IBOutlet AttributeColumn *roundedColumn;
@property IBOutlet AttributeColumn *starColumn;
@property IBOutlet AttributeColumn *ellipseColumn;

@property ZGDesktop *desktop;

- (void)addColumn:(AttributeColumn *)column;
- (void)removeColumn:(AttributeColumn *)column atIndex:(NSUInteger)index;
- (void)insertColumn:(AttributeColumn *)column atIndex:(NSUInteger)index;
- (NSArray *)allColumns;

@end
