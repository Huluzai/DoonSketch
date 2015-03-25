//
//  SpiralToolbox.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "Toolbox.h"

@interface SpiralToolbox : Toolbox
@property IBOutlet NSTextField *labelMode;
@property IBOutlet NSTextField *inputTurns;
@property IBOutlet NSTextField *inputDivergence;
@property IBOutlet NSTextField *inputInnerRadius;
@property IBOutlet NSButton *buttonDefaults;

- (IBAction)didChangeTurns:(id)sender;
- (IBAction)didChangeDivergence:(id)sender;
- (IBAction)didChangeInnerRadius:(id)sender;
- (IBAction)didResetDefaults:(id)sender;

@end
