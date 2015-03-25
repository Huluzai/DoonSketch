//
//  PaintbucketToolbox.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/6.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "Toolbox.h"

@interface PaintbucketToolbox : Toolbox

@property IBOutlet NSPopUpButton *popupChannels;
@property IBOutlet NSSlider *sliderThreshold;
@property IBOutlet NSTextField *inputOffset;
@property IBOutlet NSPopUpButton *popupAutoGap;
@property IBOutlet NSButton *buttonDefaults;
@property IBOutlet NSTextField *labelThreshold;

- (IBAction)didChangeChannel:(id)sender;
- (IBAction)didChangeThreshold:(id)sender;
- (IBAction)didChangeOffset:(id)sender;
- (IBAction)didChangeAutoGap:(id)sender;
- (IBAction)didResetDefaults:(id)sender;

@end
