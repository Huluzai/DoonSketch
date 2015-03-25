//
//  SelectToolbar.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

struct SPDesktop;

@interface SelectToolbar : NSViewController

- (id)initWithDesktop:(SPDesktop *)desktop;

@property IBOutlet NSButton *buttonSelectAll;
@property IBOutlet NSButton *buttonSelectAllInAllLayers;
@property IBOutlet NSButton *buttonDeselect;

@property IBOutlet NSButton *buttonRotate90CCW;
@property IBOutlet NSButton *buttonRotate90CW;
@property IBOutlet NSButton *buttonFlipHorizontal;
@property IBOutlet NSButton *buttonFlipVertical;

@property IBOutlet NSButton *buttonSelectionToBack;
@property IBOutlet NSButton *buttonSelectionLower;
@property IBOutlet NSButton *buttonSelectionRaise;
@property IBOutlet NSButton *buttonSelectionToFront;

@property IBOutlet NSTextField *inputX;
@property IBOutlet NSTextField *inputY;
@property IBOutlet NSTextField *inputWidth;
@property IBOutlet NSTextField *inputHeight;

@property IBOutlet NSButton *buttonLock;

@property IBOutlet NSSegmentedControl *segmentAffect;

- (IBAction)didChangeAffect:(id)sender;
- (IBAction)didChangeAnyValue:(id)sender;

@end
