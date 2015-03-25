//
//  PencilToolbox.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "PencilToolbox.h"

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

@implementation PencilToolbox

- (void)dealloc
{
    
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self addAdvanceShapeOptions];
    self.sliderSmoothing.doubleValue = GlbPrefs->getDouble("/tools/freehand/pencil/tolerance", 3.0);
    guint freehandMode = GlbPrefs->getInt("/tools/freehand/pencil/freehand-mode", 0);
    [self.segmentMode setSelectedSegment:freehandMode];
    [self.popupShape selectItemAtIndex:(GlbPrefs->getInt("/tools/freehand/pencil/shape", 0))];
}

- (NSString *)toolboxNibName
{
    return @"PencilToolbox";
}

- (IBAction)didChangeMode:(id)sender
{
    [self didFreehandModeChanged:(int)self.segmentMode.selectedSegment];
}

- (IBAction)didChangeSmoothing:(id)sender
{
    ENTER_ONCE_CODE
    // in turn, prevent listener from responding
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/tools/freehand/pencil/tolerance", self.sliderSmoothing.doubleValue);
    self.labelSmoothing.intValue = (int)self.sliderSmoothing.intValue;
    LEAVE_ONCE_CODE
}

- (void)addAdvanceShapeOptions
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    self.popupShape.enabled = prefs->getInt("/tools/freehand/pencil/shape", 1);
}

- (IBAction)didChangeShape:(id)sender
{
    gint shape = (gint)self.popupShape.selectedTag;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/freehand/pencil/shape", shape);
}

- (IBAction)didResetDefaults:(id)sender
{
    // fixme: make settable
    gdouble tolerance = 4;
    
    self.sliderSmoothing.doubleValue = tolerance;
    [self didChangeSmoothing:nill];
}

@end
