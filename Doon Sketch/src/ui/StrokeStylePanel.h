//
//  StrokeStylePanel.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-30.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "StylePage.h"

struct SPDesktop;

@interface StrokeStylePanel : StylePage

@property CGFloat width;
@property NSUInteger join;
@property CGFloat meterLimit;
@property NSUInteger dashs;
@property CGFloat offset;
@property NSUInteger cap;

@property IBOutlet NSSegmentedControl *segmentJoins;
@property IBOutlet NSSegmentedControl *segmentCaps;
@property IBOutlet NSTextField *inputMeterLimit;
@property IBOutlet NSPopUpButton *popupDashes;
@property IBOutlet NSTextField *inputOffset;

- (IBAction)didStrokeStyleAnyToggled:(id)sender;
- (IBAction)didChangeProperty:(id)sender;

@end
