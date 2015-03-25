//
//  PaintbucketToolbox.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/6.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "PaintbucketToolbox.h"

#include <desktop.h>
#include <desktop-handles.h>
#include <desktop-style.h>
#include <document-private.h>
#include <helper/units.h>
#include <inkscape.h>
#include <interface.h>
#include <pen-context.h>
#include <preferences.h>
#include <selection-chemistry.h>
#include <selection.h>
#include <sp-flowtext.h>
#include <sp-text.h>
#include <style.h>
#include <svg/css-ostringstream.h>
#include <text-context.h>
#include <text-editing.h>
#include <verbs.h>
#include <xml/attribute-record.h>
#include <xml/node-event-vector.h>
#include <xml/repr.h>
#include <preferences.h>
#include <flood-context.h>

@implementation PaintbucketToolbox

- (NSString *)toolboxNibName
{
    return @"PaintbucketToolbox";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    int ch = GlbPrefs->getInt("/tools/paintbucket/channels", 0);
    [self.popupChannels selectItemAtIndex:ch];
    double thr = GlbPrefs->getDouble("/tools/paintbucket/threshold", 5);
    self.labelThreshold.stringValue = [NSString stringWithFormat:@"Threshold:%d", (int)thr];
    self.sliderThreshold.doubleValue = thr;
    double offset = GlbPrefs->getDouble("/tools/paintbucket/offset", 0);
    self.inputOffset.doubleValue = offset;
    int ag = GlbPrefs->getBool("/tools/paintbucket/autogap");
    [self.popupAutoGap selectItemAtIndex:ag];
}

- (IBAction)didChangeChannel:(id)sender
{
    gint channels = (gint)[self.popupChannels indexOfSelectedItem];
    flood_channels_set_channels(channels);
}

- (IBAction)didChangeThreshold:(id)sender
{
    GlbPrefs->setInt("/tools/paintbucket/threshold", self.sliderThreshold.intValue);
    self.labelThreshold.stringValue = [NSString stringWithFormat:@"Threshold:%d", self.sliderThreshold.intValue];
}

- (IBAction)didChangeAutoGap:(id)sender
{
    GlbPrefs->setBool("/tools/paintbucket/autogap", [self.popupAutoGap indexOfSelectedItem]);
}

- (IBAction)didChangeOffset:(id)sender
{
    SPUnit const *unit = sp_unit_get_by_abbreviation("px");
    Inkscape::Preferences *prefs = GlbPrefs;
    
    // Don't adjust the offset value because we're saving the
    // unit and it'll be correctly handled on load.
    prefs->setDouble("/tools/paintbucket/offset", self.inputOffset.doubleValue);
    prefs->setString("/tools/paintbucket/offsetunits", sp_unit_get_abbreviation(unit));
}

- (IBAction)didResetDefaults:(id)sender
{
    // FIXME: make defaults settable via Inkscape Options
    self.sliderThreshold.intValue = 15;
    [self didChangeThreshold:nill];
    self.inputOffset.doubleValue = 0.0;
    [self didChangeOffset:nill];
    [self.popupChannels selectItemAtIndex:FLOOD_CHANNELS_RGB];
    [self didChangeChannel:nill];
    [self.popupAutoGap selectItemAtIndex:0];
    [self didChangeAutoGap:nill];
}

@end
