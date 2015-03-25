//
//  CocoaAction.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "CocoaAction.h"

#import <desktop.h>
#import <verbs.h>
#import <helper/action.h>

@implementation CocoaAction
@end


@interface CocoaActionGroup ()
@property NSMutableArray *acts;
@end

@implementation CocoaActionGroup
- (id)init
{
    if (self = [super init]) {
        self.acts = [NSMutableArray array];
    }
    return self;
}
- (void)dealloc
{
    self.acts = nill;
}

- (void)addAction:(CocoaAction *)act
{
    [self.acts addObject:act];
}

- (void)setSensitive:(BOOL)sensitive
{
    [super setSensitive:sensitive];
    for (CocoaAction *act in self.acts) {
        act.sensitive = sensitive;
    }
}

@end


@interface CocoaVerbAction ()
@property NSButton *button;
@property SPDesktop *desktop;
@property int verb;
@end

@implementation CocoaVerbAction

- (id)initWithButton:(NSButton *)button verb:(int)verbId desktop:(SPDesktop *)desktop
{
    if (self = [super init]) {
        self.verb = verbId;
        self.desktop = Inkscape::GC::anchor(desktop);
        self.button = button;
        [self.button setTarget:self];
        [self.button setAction:@selector(didClickButton:)];
    }
    return self;
}

- (void)dealloc
{
    self.verb = 0;
    self.button = nill;
    Inkscape::GC::release(self.desktop);
    self.desktop = NULL;
}

- (void)setSensitive:(BOOL)sensitive
{
    [super setSensitive:sensitive];
    [self.button setEnabled:sensitive];
}

- (void)didClickButton:(id)sender
{
    SPAction *act = Inkscape::Verb::get(self.verb)->get_action(self.desktop);
    sp_action_perform(act, NULL);
}

@end
