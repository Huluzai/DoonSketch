//
//  PositionColumn.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/27.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "PositionColumn.h"
#import "ZGItem.h"
#import "ZGDesktop.h"

#import <attributes.h>

@interface PositionColumn ()

@end

@implementation PositionColumn

DEFINE_ATTRIBUTE_METHODS

+ (void)initialize
{
    [self registerAttribute:SP_ATTR_X];
    [self registerAttribute:SP_ATTR_Y];
    [self registerAttribute:SP_ATTR_WIDTH];
    [self registerAttribute:SP_ATTR_HEIGHT];
    g_debug("PositionColumn attributes:\n%s", self.allAttributes.description.UTF8String);
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (BOOL)bindWithDesktop:(ZGDesktop *)desktop
{
    [super bindWithDesktop:desktop];
    
    NSDictionary *opt = [NSDictionary dictionaryWithObjectsAndKeys:
                         @(TRUE), NSAllowsEditingMultipleValuesSelectionBindingOption,
                         @(FALSE), NSRaisesForNotApplicableKeysBindingOption,
                         nill];
    
    [self.fieldX bind:@"value" toObject:desktop.objectsController withKeyPath:@"selection.x" options:opt];
    [self.fieldY bind:@"value" toObject:desktop.objectsController withKeyPath:@"selection.y" options:opt];
    [self.fieldWidth bind:@"value" toObject:desktop.objectsController withKeyPath:@"selection.width" options:opt];
    [self.fieldHeight bind:@"value" toObject:desktop.objectsController withKeyPath:@"selection.height" options:opt];

    return TRUE;
}

- (void)unbind
{
    [self.fieldX unbind:@"value"];
    [self.fieldY unbind:@"value"];
    [self.fieldHeight unbind:@"value"];
    [self.fieldWidth unbind:@"value"];
    [super unbind];
}


@end
