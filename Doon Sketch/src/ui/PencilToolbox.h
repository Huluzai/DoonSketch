//
//  PencilToolbox.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "Toolbox.h"

@interface PencilToolbox : Toolbox

@property IBOutlet NSSegmentedControl *segmentMode;
@property IBOutlet NSSlider *sliderSmoothing;
@property IBOutlet NSTextField *labelSmoothing;
@property IBOutlet NSButton *buttonDefaults;
@property IBOutlet NSPopUpButton *popupShape;

- (IBAction)didChangeMode:(id)sender;
- (IBAction)didChangeSmoothing:(id)sender;
- (IBAction)didChangeShape:(id)sender;
- (IBAction)didResetDefaults:(id)sender;

@end
