//
//  StarToolbox.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "Toolbox.h"

@interface StarToolbox : Toolbox

@property IBOutlet NSSegmentedControl *segmentSidesFlat;
@property IBOutlet NSTextField *inputCorners;
@property IBOutlet NSTextField *inputSpokeRatio;
@property IBOutlet NSTextField *inputRounded;
@property IBOutlet NSTextField *inputRandomized;
@property IBOutlet NSTextField *labelMode;

- (IBAction)didSidesflatChanged:(id)sender;
- (IBAction)didMagnitudeChanged:(id)sender;
- (IBAction)didProportionChanged:(id)sender;
- (IBAction)didRoundedChanged:(id)sender;
- (IBAction)didResetDefaults:(id)sender;
- (IBAction)didRandomizedChanged:(id)sender;

@end
