//
//  StylePagesManager.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/9.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "StylePagesManager.h"
#import "StrokeStylePanel.h"
#import "PaintPanel.h"
#import "ZGDesktop.h"
#import "ZGBox.h"
#import "CompositeSettingsPage.h"
#import "MessagePage.h"

//#import <desktop.h>

@interface StylePagesManager () {
    ZGDesktop *_desktop;
}
@property StrokeStylePanel *lineStylePage;
@property PaintPanel *fillPaintPage;
@property PaintPanel *strokePaintPage;
@property CompositeSettingsPage *compositePage;
@property MessagePage *msgPage;
@property ZGVBox *vbox;
@end

@implementation StylePagesManager

- (void)dealloc
{
    self.vbox = nill;
    self.fillPaintPage = nill;
    self.strokePaintPage = nill;
    self.lineStylePage = nill;
    self.desktop = nill;
    self.compositePage = nill;
    self.msgPage = nill;
}

- (void)loadPages
{
    self.msgPage = [[MessagePage alloc] initWithNibName:@"MessagePage" bundle:nill];
    self.lineStylePage = [[StrokeStylePanel alloc] initWithNibName:@"StrokeStylePanel" bundle:nill];
    self.fillPaintPage = [[PaintPanel alloc] initWithFillOrStroke:FILL];
    self.strokePaintPage = [[PaintPanel alloc] initWithFillOrStroke:STROKE];
    self.compositePage = [[CompositeSettingsPage alloc] initWithNibName:@"CompositeSettingsPage" bundle:nill];
    self.lineStylePage.desktop = self.desktop.spDesktop;
    self.fillPaintPage.desktop = self.desktop.spDesktop;
    self.strokePaintPage.desktop = self.desktop.spDesktop;
    self.compositePage.desktop = self.desktop.spDesktop;

    g_assert(self.view);
    self.vbox = [[ZGVBox alloc] initWithView:self.view];
    self.vbox.space = 3;
    [self.vbox packStart:self.compositePage.view];
    [self.vbox packStart:self.lineStylePage.view];
    [self.vbox packStart:self.strokePaintPage.view];
    [self.vbox packStart:self.fillPaintPage.view];
    [self.vbox packStart:self.msgPage.view];
    
    self.lineStylePage.title = @"Line Style";
    self.strokePaintPage.title = @"Stroke Paint Style";
    self.fillPaintPage.title = @"Fill Paint Style";
    self.compositePage.title = @"Composite Style";
    
    [self.vbox sizeToFit];
    [self.view scrollRectToVisible:[self.msgPage.view frame]];
}

- (MessagePage *)messagePage
{
    return self.msgPage;
}

- (void)setDesktop:(ZGDesktop *)desktop
{
    SPDesktop *dt = NULL;
    if (desktop && desktop.spDesktop) {
        _desktop = desktop;
        dt = desktop.spDesktop;
    } else {
        _desktop = nill;
    }
    self.lineStylePage.desktop = dt;
    self.fillPaintPage.desktop = dt;
    self.strokePaintPage.desktop = dt;
    self.compositePage.desktop = dt;
}

- (ZGDesktop *)desktop
{
    return _desktop;
}

@end
