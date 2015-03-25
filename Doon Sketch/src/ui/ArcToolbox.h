//
//  ArcToolbox.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "Toolbox.h"

@interface ArcToolbox : Toolbox
@property IBOutlet NSTextField *inputStart;
@property IBOutlet NSTextField *inputEnd;
@property IBOutlet NSTextField *labelMode;
@property IBOutlet NSSegmentedControl *segmentShape;
@property IBOutlet NSButton *buttonWhole;

- (IBAction)didChangeStart:(id)sender;
- (IBAction)didChangeEnd:(id)sender;
- (IBAction)didChangeShape:(id)sender;
- (IBAction)didResetWhole:(id)sender;

@end
