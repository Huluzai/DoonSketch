//
//  CompositeSettingsPage.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/9.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "StylePage.h"

@interface CompositeSettingsPage : StylePage
@property IBOutlet NSSlider *sliderBlur;
@property IBOutlet NSSlider *sliderOpacity;
@property IBOutlet NSTextField *inputBlur;
@property IBOutlet NSTextField *inputOpacity;

- (IBAction)didChangeBlur:(id)sender;
- (IBAction)didChangeOpacity:(id)sender;

@end
