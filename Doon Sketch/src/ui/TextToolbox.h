//
//  TextToolbox.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/5.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "Toolbox.h"

@class ZGDesktopStyle;

@interface TextToolbox : Toolbox

@property ZGDesktopStyle *zStyle;

@property IBOutlet NSPopUpButton *popupFamilys;
@property IBOutlet NSComboBox *comboSizes;
@property IBOutlet NSSegmentedControl *segmentStyle;
@property IBOutlet NSSegmentedControl *segmentAlign;
@property IBOutlet NSSegmentedControl *segmentScript;
@property IBOutlet NSTextField *inputLineSpace;
@property IBOutlet NSTextField *inputletterSapce;
@property IBOutlet NSTextField *inputWordSpace;
@property IBOutlet NSTextField *inputVerticalShift;
@property IBOutlet NSTextField *inputRotation;
@property IBOutlet NSButton *buttonVerticalOrHorizontal;


- (IBAction)didChangeStyle:(id)sender;
- (IBAction)didChangeScript:(id)sender;
/*
- (IBAction)didChangeFamily:(id)sender;
- (IBAction)didChangeSize:(id)sender;
- (IBAction)didChangeAlign:(id)sender;
- (IBAction)didChangeLineSpace:(id)sender;
- (IBAction)didChangeLetterSpace:(id)sender;
- (IBAction)didChangeWordSpace:(id)sender;
- (IBAction)didChangeVerticalShift:(id)sender;
- (IBAction)didChangeRotation:(id)sender;
- (IBAction)didChangeVerticalOrHorizontal:(id)sender;
*/
@end
