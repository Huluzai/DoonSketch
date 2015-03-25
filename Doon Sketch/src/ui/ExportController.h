//
//  ExportController.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/9.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

struct SPDocument;
@class ZGDesktop;

@interface ExportController : NSWindowController
@property ZGDesktop *zDt;
@property(weak) id delegate;
@property CGFloat dpi;
@property NSInteger width;
@property NSInteger height;

@property IBOutlet NSView *viewOption;
@property IBOutlet NSProgressIndicator *proExport;
@property IBOutlet NSTextField *labelProgMsg;
@property IBOutlet NSTextField *inputDpi;
@property IBOutlet NSTextField *inputWidth;
@property IBOutlet NSTextField *inputHeight;

- (void)exportDocument:(SPDocument *)doc toFile:(NSURL*)anUrl;
- (void)prepareOptionView;

- (IBAction)cancel:(id)sender;

@end

@interface ExportController (Delegate)
- (void)exportControllerDidComplete:(ExportController *)ec;
@end
