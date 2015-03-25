//
//  Calligraphy.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/5.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "Toolbox.h"

@interface CalligraphyToolbox : Toolbox

@property IBOutlet NSPopUpButton *popupPreset;
@property IBOutlet NSSlider *sliderWidth;
@property IBOutlet NSTextField *labelWidth;
@property IBOutlet NSSegmentedControl *segmentPressureAndTrace;
@property IBOutlet NSTextField *inputThinning;
@property IBOutlet NSTextField *inputAngle;
@property IBOutlet NSTextField *inputFixation;
@property IBOutlet NSTextField *inputCaps;
@property IBOutlet NSButton *buttonTile;
@property IBOutlet NSSlider *sliderTremor;
@property IBOutlet NSTextField *labelTremor;
@property IBOutlet NSSlider *sliderWiggle;
@property IBOutlet NSTextField *labelWiggle;
@property IBOutlet NSSlider *sliderMass;
@property IBOutlet NSTextField *labelMass;
@property IBOutlet NSPanel *savePanel;

@property IBOutlet NSTextField *inputProfileName;

- (IBAction)didChangePreset:(id)sender;
- (IBAction)didChangeWidth:(id)sender;
- (IBAction)didChangePressureOrTrance:(id)sender;
- (IBAction)didChangeThinning:(id)sender;
- (IBAction)didChangeAngle:(id)sender;
- (IBAction)didChangeFixation:(id)sender;
- (IBAction)didChangeCaps:(id)sender;
- (IBAction)didChangeTile:(id)sender;
- (IBAction)didChangeWiggle:(id)sender;
- (IBAction)didChangeTremor:(id)sender;
- (IBAction)didChangeMass:(id)sender;

- (IBAction)didCancelSaveProfile:(id)sender;
- (IBAction)didSaveProfile:(id)sender;

@end
