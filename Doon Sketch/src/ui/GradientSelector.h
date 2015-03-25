//
//  GradientSelector.h
//  Inxcape
//
//  Created by 张 光建 on 14-10-11.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <widgets/gradient-selector.h>

struct SPGradient;
struct SPDesktop;

@interface GradientSelector : NSViewController

@property IBOutlet NSPopUpButton *vectorsButton;
@property IBOutlet NSButton *editButton;
@property IBOutlet NSButton *dupButton;
@property IBOutlet NSImageView *imagePreview;

@property(weak) id delegate;
@property SPGradientSelector::SelectorMode mode;
@property SPGradientUnits units;
@property SPGradientSpread spread;

// move
- (void)setVector:(SPGradient *)vector toDocument:(SPDocument *)doc;
- (SPGradient *)vector;

- (IBAction)didDuplicateClicked:(id)sender;
- (IBAction)didEditClicked:(id)sender;

@end


@interface GradientSelector (Delegate)

- (void)didGradientSelectorChanged:(GradientSelector *)gsel;

@end

