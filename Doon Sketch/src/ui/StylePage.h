//
//  StylePage.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/7.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

struct SPDesktop;
@class ZGSelection;
@class ZGDesktop;

@interface StylePage : NSViewController

@property SPDesktop *desktop;
@property NSString *title;

@property IBOutlet NSTextField *labelTitle;

- (void)didSelectionModified:(NSNotification *)aNotification;
- (void)didSelectionChanged:(NSNotification *)aNotification;
- (void)didSubselectionChanged:(NSNotification *)aNotification;

- (ZGSelection *)zSel;
- (ZGDesktop *)zDt;

@end
