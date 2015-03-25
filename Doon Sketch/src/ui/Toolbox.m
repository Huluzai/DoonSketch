//
//  Toolbox.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/2.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "Toolbox.h"
#import "ZGSelection.h"
#import "ZGDesktop.h"

#import <verbs.h>
#import <desktop.h>
#import <document.h>
#import <helper/action.h>
#import <preferences.h>
#import <sp-item.h>
#import <selection.h>
#import <desktop-handles.h>
#import <sp-item-transform.h>
#import <desktop-style.h>
#import <display/canvas-base.h>
#import <selection-chemistry.h>
#import <sp-rect.h>
#import <sp-star.h>
#import <sp-text.h>
#import <style.h>
#import <sp-ellipse.h>
#import <shape-editor.h>
#import <pen-context.h>
#import <text-context.h>
#import <text-editing.h>
#import <tweak-context.h>
#import <spray-context.h>
#import <ui/tool/node-tool.h>
#import <ui/tool/control-point-selection.h>
#import <ui/tool/multi-path-manipulator.h>
#import <sp-flowtext.h>
#import <tools-switch.h>

@interface Toolbox () {
    SPDesktop *_desktop;
}
@property SPDesktop *desktop;
@property ZGSelection *zSel;
@property ZGDesktop *zDt;
@end

@implementation Toolbox

- (id)initWithSPDesktop:(SPDesktop *)desktop
{
    if (self = [super initWithNibName:self.toolboxNibName bundle:nill]) {
        self.desktop = desktop;
        self.zSel = [[ZGSelection alloc] initWithSPDesktop:desktop];
        self.zDt = [[ZGDesktop alloc] initWithSPDesktop:self.desktop];
    }
    return self;
}

- (void)viewDidLoad
{
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

- (void)setDesktop:(SPDesktop *)desktop
{
    if (desktop) {
        Inkscape::GC::anchor(desktop);
    }
    
    if (_desktop) {
        Inkscape::GC::release(_desktop);
    }
    _desktop = desktop;
}

- (SPDesktop *)desktop
{
    return _desktop;
}

- (void)dealloc
{
    [defaultNfc removeObserver:self];
    self.zSel = nill;
    self.zDt = nill;
    self.desktop = NULL;
}

+ (id)toolboxFromGPointer:(gpointer)pointer
{
    return (__bridge Toolbox *)pointer;
}

- (gpointer)gpointer
{
    return (__bridge gpointer)self;
}

- (NSString *)toolboxNibName
{
    return @"";
}

- (void)didFreehandModeChanged:(int)mode
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring node = tools_isactive(self.desktop, TOOLS_FREEHAND_PEN) ? "/tools/freehand/pen" : "/tools/freehand/pencil";
    
    prefs->setInt(node + "/freehand-mode", mode);
    
    SPDesktop *desktop = self.desktop;
    
    // in pen tool we have more options than in pencil tool; if one of them was chosen, we do any
    // preparatory work here
    if (SP_IS_PEN_CONTEXT(desktop->event_context)) {
        SPPenContext *pc = SP_PEN_CONTEXT(desktop->event_context);
        sp_pen_context_set_polyline_mode(pc);
    }
}

- (void)didSelectionChanged:(NSNotification *)aNotification
{}

- (void)didSubselectionChanged:(NSNotification *)aNotification
{}

- (void)didSelectionModified:(NSNotification *)aNotificaiton
{}

@end
