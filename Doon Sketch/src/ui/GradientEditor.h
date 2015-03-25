//
//  GradientEditor.h
//  Inxcape
//
//  Created by 张 光建 on 14-10-11.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

struct SPGradient;

@interface GradientEditor : NSWindowController

@property CGFloat offset;
@property (readonly) SPGradient *gradient;

@property IBOutlet NSPopUpButton *popupStops;
@property IBOutlet NSImageView *imagePreview;
@property IBOutlet NSColorWell *colorStop;
@property IBOutlet NSSlider *sliderOffset;
@property IBOutlet NSTextField *inputOffset;
@property IBOutlet NSButton *buttonAddStop;
@property IBOutlet NSButton *buttonDelStop;

- (void)loadGradient:(SPGradient *)gradient;

- (IBAction)didAddStop:(id)sender;
- (IBAction)didDeleteStop:(id)sender;
- (IBAction)didChangeStopColor:(id)sender;

@end
