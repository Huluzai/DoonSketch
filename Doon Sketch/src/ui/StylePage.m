//
//  StylePage.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/7.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "StylePage.h"
#import "ZGSelection.h"
#import "ZGDesktop.h"

#import <desktop.h>

@interface StylePage () {
    SPDesktop *_desktop;
}
@property ZGSelection *zSel;
@property ZGDesktop *zDt;
@end

@implementation StylePage

- (void)dealloc
{
    [defaultNfc removeObserver:self];
    self.zSel = nill;
    self.zDt = nill;
}

- (void)setTitle:(NSString *)title
{
    self.labelTitle.stringValue = title;
}

- (NSString *)title
{
    return self.labelTitle.stringValue;
}

- (void)setDesktop:(SPDesktop *)desktop
{
    self.zSel = nill;
    self.zDt = nill;
    
    if (desktop) {
        Inkscape::GC::anchor(desktop);
    }
    
    if (_desktop) {
        Inkscape::GC::release(_desktop);
    }
    _desktop = desktop;
    
    if (desktop) {
        self.zSel = [[ZGSelection alloc] initWithSPDesktop:desktop];
        self.zDt = [[ZGDesktop alloc] initWithSPDesktop:desktop];
        
        [defaultNfc addObserver:self
                       selector:@selector(didSelectionChanged:)
                           name:kZGSelectionDidChangedNotification
                         object:self.zSel];
        [defaultNfc addObserver:self
                       selector:@selector(didSelectionModified:)
                           name:kZGSelectionDidModifiedNotification
                         object:self.zSel];
        [defaultNfc addObserver:self
                       selector:@selector(didSubselectionChanged:)
                           name:kZGDesktopSubselectionChanged
                         object:self.zDt];
    }
}

- (SPDesktop *)desktop
{
    return _desktop;
}

- (void)didSelectionModified:(NSNotification *)aNotification
{}

- (void)didSelectionChanged:(NSNotification *)aNotification
{}

- (void)didSubselectionChanged:(NSNotification *)aNotification
{}

@end
