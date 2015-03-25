//
//  PaintPanel.h
//  Inxcape
//
//  Created by 张 光建 on 14-10-11.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <desktop.h>
#import <fill-or-stroke.h>
#import "StylePage.h"

@interface PaintPanel : StylePage

@property IBOutlet NSView *pselContent;

- (id)initWithDesktop:(SPDesktop *)desktop fillOrStroke:(FillOrStroke)kind;

- (id)initWithFillOrStroke:(FillOrStroke)kind;

@end
