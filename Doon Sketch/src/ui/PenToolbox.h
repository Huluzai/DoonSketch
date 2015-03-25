//
//  PenToolbox.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/5.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "Toolbox.h"

@interface PenToolbox : Toolbox

@property IBOutlet NSSegmentedControl *segmentMode;
@property IBOutlet NSPopUpButton *popupShape;

- (IBAction)didChangeMode:(id)sender;
- (IBAction)didChangeShape:(id)sender;
@end
