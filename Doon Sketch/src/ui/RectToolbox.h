//
//  RectToolbox.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "Toolbox.h"

@interface RectToolbox : Toolbox
@property IBOutlet NSTextField *inputWidth;
@property IBOutlet NSTextField *inputHeight;
@property IBOutlet NSTextField *inputRx;
@property IBOutlet NSTextField *inputRy;
@property IBOutlet NSTextField *labelMode;
@property IBOutlet NSButton *buttonReset;

- (IBAction)didResetRounded:(id)sender;
- (IBAction)didChangeWidth:(id)sender;
- (IBAction)didChangeHeight:(id)sender;
- (IBAction)didChangeRx:(id)sender;
- (IBAction)didChangeRy:(id)sender;

@end
