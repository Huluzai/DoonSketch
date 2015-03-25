//
//  NodeToolbox.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/2.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

struct SPDesktop;

@interface NodeToolbox : NSViewController

@property IBOutlet NSTextField *inputX;
@property IBOutlet NSTextField *inputY;
@property IBOutlet NSSegmentedControl *segmentShowPaths;
@property IBOutlet NSSegmentedControl *segmentShowHandles;
@property IBOutlet NSButton *checkLpeedit;

- (id)initWithSPDesktop:(SPDesktop *)desktop;

- (IBAction)didButtonAdd:(id)sender;
- (IBAction)didButtonDelete:(id)sender;
- (IBAction)didButtonDeleteSegment:(id)sender;
- (IBAction)didButtonBreak:(id)sender;
- (IBAction)didButtonJoin:(id)sender;
- (IBAction)didButtonJoinSegment:(id)sender;
- (IBAction)didButtonToLine:(id)sender;
- (IBAction)didButtonToCureve:(id)sender;
- (IBAction)didButtonCusp:(id)sender;
- (IBAction)didButtonSmooth:(id)sender;
- (IBAction)didButtonSymmetrical:(id)sender;
- (IBAction)didButtonAuto:(id)sender;
- (IBAction)didButtonNextLPEparam:(id)sender;
- (IBAction)didSegmentToggled:(id)sender;

@end
