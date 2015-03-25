//
//  TextParameter.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "ZGAdjustment.h"

@class ZGDesktopStyle;

@interface TextParameter : ZGAdjustment

@property IBOutlet NSPopUpButton *fontFamilyPopUpButton;
@property IBOutlet NSComboBox *fontSizeComboBox;
@property IBOutlet NSSegmentedControl *styleSegmentControl;
@property IBOutlet NSSegmentedControl *alignSegmentControl;
@property IBOutlet NSPopUpButton *lineHeightPopUpButton;

@property BOOL fontBold;
@property BOOL fontItalic;

@property ZGDesktopStyle *desktopStyle;

- (IBAction)didChangeTextStyle:(id)sender;

@end
