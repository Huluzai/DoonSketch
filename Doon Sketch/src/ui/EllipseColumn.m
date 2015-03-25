//
//  EllipseColumn.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/29.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "EllipseColumn.h"
#import "ZGDesktop.h"
#import "ZGItem.h"

#import <attributes.h>

@interface EllipseColumn ()

@end

@implementation EllipseColumn

DEFINE_ATTRIBUTE_METHODS

+ (void)initialize
{
    [self registerAttribute:SP_ATTR_CX];
    [self registerAttribute:SP_ATTR_CY];
    [self registerAttribute:SP_ATTR_RX];
    [self registerAttribute:SP_ATTR_RY];
    g_debug("EllipseColumn attributes:\n%s", self.allAttributes.description.UTF8String);
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
    
    [self.fieldCx bind:@"value" toObject:desktop.objectsController withKeyPath:@"selection.cx" options:opt];
    [self.fieldCy bind:@"value" toObject:desktop.objectsController withKeyPath:@"selection.cy" options:opt];
    [self.fieldRx bind:@"value" toObject:desktop.objectsController withKeyPath:@"selection.rx" options:opt];
    [self.fieldRy bind:@"value" toObject:desktop.objectsController withKeyPath:@"selection.ry" options:opt];
    
    return TRUE;
}

- (void)unbind
{
    [self.fieldCx unbind:@"value"];
    [self.fieldCy unbind:@"value"];
    [self.fieldRx unbind:@"value"];
    [self.fieldRy unbind:@"value"];
    [super unbind];
}

@end
