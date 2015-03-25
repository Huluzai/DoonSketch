//
//  ActiveInfoPanel.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/10.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

//#define USE_PANEL

@interface ActiveInfoPanel :
#ifdef USE_PANEL
NSWindowController
#else
NSViewController
#endif

@property IBOutlet NSTextField *textField;

@property NSTextAlignment xAlign;
@property NSTextAlignment yAlign;

- (void)showContent:(NSString *)str forView:(NSView *)view inWindow:(NSWindow *)parent forTime:(NSTimeInterval)showTime;
- (void)showContent:(NSString *)str atPoint:(CGPoint)point inWindow:(NSWindow *)parent forTime:(NSTimeInterval)showTime;
- (void)showContent:(NSString *)str atPoint:(CGPoint)point forView:(NSView *)view inWindow:(NSWindow *)parent forTime:(NSTimeInterval)showTime;
- (void)dismiss;

@end
