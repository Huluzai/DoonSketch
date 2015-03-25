//
//  Calligraphy.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/5.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "CalligraphyToolbox.h"
#import "ZGPreferences.h"

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

@interface CalligraphyToolbox ()
@property BOOL presetsBlocked;
@property ZGPreferencesNode *zPrefs;
@property NSUInteger savePresetIndex;
@property NSDictionary *adjControls;
@property NSModalSession modalSession;
@end

@implementation CalligraphyToolbox

- (id)initWithSPDesktop:(SPDesktop *)desktop
{
    if (self = [super initWithSPDesktop:desktop]) {
        self.zPrefs = [[ZGPreferencesNode alloc] initWithNodePath:@"/tools/calligraphic/"];
    }
    return self;
}

- (void)dealloc
{
    [defaultNfc removeObserver:self];
    self.zPrefs = nill;
    self.adjControls = nill;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.presetsBlocked = TRUE;
    
    self.sliderWidth.doubleValue = GlbPrefs->getDouble("/tools/calligraphic/width", 15);
    self.inputAngle.enabled = !GlbPrefs->getBool("/tools/calligraphic/usetilt", true);
    self.buttonTile.state = GlbPrefs->getBool("/tools/calligraphic/usetilt", true) ? NSOnState : NSOffState;
    self.sliderWidth.doubleValue = GlbPrefs->getDouble("/tools/calligraphic/width", 15);
    self.inputThinning.doubleValue = GlbPrefs->getDouble("/tools/calligraphic/thinning", 10);
    self.inputAngle.doubleValue = GlbPrefs->getDouble("/tools/calligraphic/angle", 30);
    self.inputFixation.doubleValue = GlbPrefs->getDouble("/tools/calligraphic/flatness", 90);
    self.inputCaps.doubleValue = GlbPrefs->getDouble("/tools/calligraphic/cap_rounding", 0.0);
    self.sliderTremor.doubleValue = GlbPrefs->getDouble("/tools/calligraphic/tremor", 0.0);
    self.sliderWiggle.doubleValue = GlbPrefs->getDouble("/tools/calligraphic/wiggle", 0.0);
    self.sliderMass.doubleValue = GlbPrefs->getDouble("/tools/calligraphic/mass", 2.0);
    [self.segmentPressureAndTrace setSelected:GlbPrefs->getBool("/tools/calligraphic/usepressure", 0) forSegment:0];
    [self.segmentPressureAndTrace setSelected:GlbPrefs->getBool("/tools/calligraphic/tracebackground", 0) forSegment:1];
    
    self.adjControls = [NSDictionary dictionaryWithObjectsAndKeys:
                        self.inputAngle, @"angle",
                        self.buttonTile, @"usetilt",
                        self.sliderMass, @"mass",
                        self.sliderWiggle, @"wiggle",
                        self.inputThinning, @"thinning",
                        self.sliderTremor, @"tremor",
                        self.inputFixation, @"flatness",
                        self.inputCaps, @"cap_rounding",
                        self.sliderWidth, @"width",
                        NULL];
    [self buildPresetsList];
    
    [defaultNfc addObserver:self
                   selector:@selector(didChangedPreferences:)
                       name:kZGPreferencesDidChangedValueNotification
                     object:self.zPrefs];
    
    self.presetsBlocked = FALSE;
}

- (NSString *)toolboxNibName
{
    return @"CalligraphyToolbox";
}

- (void)didChangedPreferences:(NSNotification *)aNotification
{
    NSString *path = [aNotification.userInfo objectForKey:@"path"];
    
    if ([path isEqualTo:@"/tools/calligraphic/tracebackground"]) {
        BOOL val = GlbPrefs->getBool("/tools/calligraphic/tracebackground");
        [self.segmentPressureAndTrace setSelected:val forSegment:1];
    } else if ([path isEqualTo:@"/tools/calligraphic/usepressure"]) {
        BOOL val = GlbPrefs->getBool("/tools/calligraphic/usepressure");
        [self.segmentPressureAndTrace setSelected:val forSegment:0];
    } else if ([path isEqualTo:@"/tools/calligraphic/usetilt"]) {
        BOOL val = GlbPrefs->getBool("/tools/calligraphic/usetilt");
        [self.buttonTile setState:(val ? NSOnState : NSOffState)];
    } else {
        return;
    }
    
    [self updatePresetsList];
}

- (void)buildPresetsList
{
    self.presetsBlocked = TRUE;
   
    NSMenu *menu = self.popupPreset.menu;
    [menu removeAllItems];
    
    [menu addItemWithTitle:@"No preset" action:@selector(didChangePreset:) keyEquivalent:@""];

    // iterate over all presets to populate the list
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    std::vector<Glib::ustring> presets = prefs->getAllDirs("/tools/calligraphic/preset");
    
    for (std::vector<Glib::ustring>::iterator i = presets.begin(); i != presets.end(); ++i) {
        Glib::ustring preset_name = prefs->getString(*i + "/name");
        [menu addItemWithTitle:[NSString stringWithUTF8String:preset_name.data()]
                        action:@selector(didChangePreset:)
                 keyEquivalent:@""];
    }
    
    [menu addItemWithTitle:@"Save..." action:@selector(didChangePreset:) keyEquivalent:@""];

    for (NSMenuItem *item in menu.itemArray) {
        [item setTarget:self];
    }
    
    self.savePresetIndex = [menu indexOfItemWithTitle:@"Save..."];
    self.presetsBlocked = FALSE;
    
    [self updatePresetsList];
}

- (void)updatePresetsList
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (self.presetsBlocked) {
        return;
    }
    
    std::vector<Glib::ustring> presets = prefs->getAllDirs("/tools/calligraphic/preset");
    
    int ege_index = 1;
    for (std::vector<Glib::ustring>::iterator i = presets.begin(); i != presets.end(); ++i, ++ege_index) {
        bool match = true;
        
        std::vector<Inkscape::Preferences::Entry> preset = prefs->getAllEntries(*i);
        for (std::vector<Inkscape::Preferences::Entry>::iterator j = preset.begin(); j != preset.end(); ++j) {
            Glib::ustring entry_name = j->getEntryName();
            if (entry_name == "id" || entry_name == "name") {
                continue;
            }
            
            double v = j->getDouble();
            double adjVal = 0;
            
            if (entry_name == "tracebackground") {
                adjVal = [self.segmentPressureAndTrace isSelectedForSegment:1] ? 1 : 0;
            } else if (entry_name == "usepressure") {
                adjVal = [self.segmentPressureAndTrace isSelectedForSegment:0] ? 1 : 0;
            } else if (entry_name == "usetilt") {
                adjVal = (self.buttonTile.state == NSOnState ? 1 : 0);
            } else {
                NSControl *control = [self.adjControls objectForKey:[NSString stringWithUTF8String:entry_name.data()]];
                if (control) {
                    adjVal = control.doubleValue;
                } else {
                    g_warning("Bad key found in a preset record: %s\n", entry_name.data());
                    continue;
                }
            }
            
            if (fabs(adjVal - v) > 1e-6) {
                match = false;
                break;
            }
        }
        
        if (match) {
            // newly added item is at the same index as the
            // save command, so we need to change twice for it to take effect
            [self.popupPreset selectItemAtIndex:ege_index];
            return;
        }
    }
    
    // no match found
    [self.popupPreset selectItemAtIndex:0];
}

- (void)saveProfile
{
    SPDesktop *desktop = self.desktop;
    if (! desktop) {
        return;
    }
    
    if (self.presetsBlocked) {
        return;
    }
    
    self.modalSession = [NSApp beginModalSessionForWindow:self.savePanel];
}

- (IBAction)didChangePreset:(id)sender
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    
    gint preset_index = (gint)[self.popupPreset indexOfSelectedItem];
    
    // This is necessary because EgeSelectOneAction spams us with GObject "changed" signal calls
    // even when the preset is not changed. It would be good to replace it with something more
    // modern. Index 0 means "No preset", so we don't do anything.
    if (preset_index == 0) {
        return;
    }
    
    gint save_presets_index = (gint)self.savePresetIndex;
    
    if (preset_index == save_presets_index) {
        // this is the Save command
        [self saveProfile];
        return;
    }
    
    if (self.presetsBlocked) {
        return;
    }
    
    // preset_index is one-based so we subtract 1
    std::vector<Glib::ustring> presets = prefs->getAllDirs("/tools/calligraphic/preset");
    Glib::ustring preset_path = presets.at(preset_index - 1);
    
    if (!preset_path.empty()) {
        self.presetsBlocked = TRUE;
        
        std::vector<Inkscape::Preferences::Entry> preset = prefs->getAllEntries(preset_path);
        
        // Shouldn't this be std::map?
        for (std::vector<Inkscape::Preferences::Entry>::iterator i = preset.begin(); i != preset.end(); ++i) {
            Glib::ustring entry_name = i->getEntryName();
            if (entry_name == "id" || entry_name == "name") {
                continue;
            }
            
            if (entry_name == "tracebackground") {
                [self.segmentPressureAndTrace setSelected:i->getBool() forSegment:1];
            } else if (entry_name == "usepressure") {
                [self.segmentPressureAndTrace setSelected:i->getBool() forSegment:0];
            } else if (entry_name == "usetile") {
                [self.buttonTile setState:(i->getBool() ? NSOnState : NSOffState)];
            } else {
                NSControl *control = [self.adjControls objectForKey:[NSString stringWithUTF8String:entry_name.data()]];
                if (control) {
                    control.doubleValue = i->getDouble();
                } else {
                    g_warning("Bad key found in a preset record: %s\n", entry_name.data());
                }
            }
        }
        
        [self didChangeWidth:nill];
        [self didChangeThinning:nill];
        [self didChangePressureOrTrance:nill];
        [self didChangeTile:nill];
        [self didChangeTremor:nill];
        [self didChangeWiggle:nill];
        [self didChangeMass:nill];
        [self didChangeAngle:nill];
        [self didChangeCaps:nill];
        
        self.presetsBlocked = FALSE;
    }
}

- (IBAction)didChangeWidth:(id)sender
{
    GlbPrefs->setDouble( "/tools/calligraphic/width", self.sliderWidth.doubleValue );
    self.labelWidth.stringValue = [NSString stringWithFormat:@"Width:%d", self.sliderWidth.intValue];
    [self updatePresetsList];
}

- (IBAction)didChangePressureOrTrance:(id)sender
{
    GlbPrefs->setBool("/tools/calligraphic/tracebackground", [self.segmentPressureAndTrace isSelectedForSegment:1]);
    GlbPrefs->setBool("/tools/calligraphic/usepressure", [self.segmentPressureAndTrace isSelectedForSegment:0]);
}

- (IBAction)didChangeThinning:(id)sender
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/tools/calligraphic/thinning", self.inputThinning.doubleValue );
    [self updatePresetsList];
}

- (IBAction)didChangeAngle:(id)sender
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/angle", self.inputAngle.doubleValue );
    [self updatePresetsList];
}

- (IBAction)didChangeFixation:(id)sender
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/flatness", self.inputFixation.doubleValue );
    [self updatePresetsList];
}

- (IBAction)didChangeCaps:(id)sender
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/cap_rounding", self.inputCaps.doubleValue );
    [self updatePresetsList];
}

- (IBAction)didChangeTile:(id)sender
{
    self.inputAngle.enabled = (self.buttonTile.state == NSOffState);
}

- (IBAction)didChangeWiggle:(id)sender
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/wiggle", self.sliderWiggle.doubleValue );
    self.labelWiggle.stringValue = [NSString stringWithFormat:@"Wiggle:%d", self.sliderWiggle.intValue];
    [self updatePresetsList];
}

- (IBAction)didChangeTremor:(id)sender
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/tremor", self.sliderTremor.doubleValue );
    self.labelTremor.stringValue = [NSString stringWithFormat:@"Tremor:%d", self.sliderTremor.intValue];
    [self updatePresetsList];
}

- (IBAction)didChangeMass:(id)sender
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble( "/tools/calligraphic/mass", self.sliderMass.doubleValue );
    self.labelMass.stringValue = [NSString stringWithFormat:@"Mass:%d", self.sliderMass.intValue];
    [self updatePresetsList];
}

- (IBAction)didCancelSaveProfile:(id)sender
{
    [NSApp endModalSession:self.modalSession];
    [self.savePanel orderOut:nill];
    [self updatePresetsList];
}

- (IBAction)didSaveProfile:(id)sender
{
    [NSApp endModalSession:self.modalSession];
    
    Glib::ustring profile_name = [self.inputProfileName.stringValue UTF8String];
    
    if (profile_name.empty()) {
        // empty name entered
        [self updatePresetsList];
        return;
    }
    
    self.presetsBlocked = TRUE;
    
    // If there's a preset with the given name, find it and set save_path appropriately
    std::vector<Glib::ustring> presets = GlbPrefs->getAllDirs("/tools/calligraphic/preset");
    int total_presets = (int)presets.size();
    int new_index = -1;
    Glib::ustring save_path; // profile pref path without a trailing slash
    
    int temp_index = 0;
    for (std::vector<Glib::ustring>::iterator i = presets.begin(); i != presets.end(); ++i, ++temp_index) {
        Glib::ustring name = GlbPrefs->getString(*i + "/name");
        if (!name.empty() && profile_name == name) {
            new_index = temp_index;
            save_path = *i;
            break;
        }
    }
    
    if (new_index == -1) {
        // no preset with this name, create
        new_index = total_presets + 1;
        gchar *profile_id = g_strdup_printf("/dcc%d", new_index);
        save_path = Glib::ustring("/tools/calligraphic/preset") + profile_id;
        g_free(profile_id);
    }
    
    GlbPrefs->setBool(save_path + "/" + "tracebackground", [self.segmentPressureAndTrace isSelectedForSegment:1]);
    GlbPrefs->setBool(save_path + "/" + "usepressure", [self.segmentPressureAndTrace isSelectedForSegment:0]);
    GlbPrefs->setBool(save_path + "/" + "usetile", (self.buttonTile.state == NSOnState));
    
    for (NSString *key in self.adjControls) {
        NSControl *control = [self.adjControls objectForKey:key];
        GlbPrefs->setDouble(save_path + "/" + key.UTF8String, control.doubleValue);
    }
    
    GlbPrefs->setString(save_path + "/name", profile_name);
    
    self.presetsBlocked = FALSE;
    [self buildPresetsList];
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return TRUE;
}

@end
