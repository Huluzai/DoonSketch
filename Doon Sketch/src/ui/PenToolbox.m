//
//  PenToolbox.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/5.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "PenToolbox.h"

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
#import <sp-spiral.h>
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
#import <xml/repr.h>
#import <xml/node-event-vector.h>
#import <mod360.h>

@implementation PenToolbox

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self addAdvanceShapeOptions];
    guint freehandMode = GlbPrefs->getInt("/tools/freehand/pen/freehand-mode", 0);
    [self.segmentMode setSelectedSegment:freehandMode];
    [self.popupShape selectItemAtIndex:(GlbPrefs->getInt("/tools/freehand/pen/shape", 0))];
}

- (NSString *)toolboxNibName
{
    return @"PenToolbox";
}

- (void)addAdvanceShapeOptions
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    self.popupShape.enabled = prefs->getInt("/tools/freehand/pen/shape", 1);
}

- (IBAction)didChangeShape:(id)sender
{
    gint shape = (gint)self.popupShape.selectedTag;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/freehand/pen/shape", shape);
}

- (IBAction)didChangeMode:(id)sender
{
    [self didFreehandModeChanged:(int)self.segmentMode.selectedSegment];
}

@end
