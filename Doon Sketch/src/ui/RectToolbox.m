//
//  RectToolbox.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "RectToolbox.h"
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
#import <inkscape.h>
#import <xml/repr.h>
#import <xml/node-event-vector.h>

typedef void (*SetterFun)(SPRect *, gdouble);

@interface RectToolbox ()
@property BOOL single;
@property Inkscape::XML::Node *repr;
@property SPItem *item;
- (void)didReprAttrChanged;
@end

static void rect_tb_event_attr_changed(Inkscape::XML::Node * /*repr*/, gchar const * /*name*/,
                                       gchar const * /*old_value*/, gchar const * /*new_value*/,
                                       bool /*is_interactive*/, gpointer data)
{
    RectToolbox *toolbox = (__bridge RectToolbox *)data;
    [toolbox didReprAttrChanged];
}

static Inkscape::XML::NodeEventVector rect_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    rect_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

@implementation RectToolbox

- (void)dealloc
{
    purge_repr_listener(NULL, (__bridge gpointer)self);
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.single = TRUE;
    self.inputWidth.doubleValue = GlbPrefs->getDouble("/tools/shapes/rect/width", 0);
    self.inputHeight.doubleValue = GlbPrefs->getDouble("/tools/shapes/rect/Height", 0);
    self.inputRx.doubleValue = GlbPrefs->getDouble("/tools/shapes/rect/rx", 0);
    self.inputRy.doubleValue = GlbPrefs->getDouble("/tools/shapes/rect/ry", 0);
}

- (NSString *)toolboxNibName
{
    return @"RectToolbox";
}

- (IBAction)didValueChangedWithValueName:(gchar const *)value_name value:(double)value setter:(SetterFun)setter
{
    SPDesktop *desktop = self.desktop;
    
    SPUnit const *unit = sp_unit_get_by_abbreviation("px");
    
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble(Glib::ustring("/tools/shapes/rect/") + value_name, sp_units_get_pixels(value, *unit));
    }
    
    ENTER_ONCE_CODE
    
    bool modmade = false;
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    for (GSList const *items = selection->itemList(); items != NULL; items = items->next) {
        if (SP_IS_RECT(items->data)) {
            if (value != 0) {
                setter(SP_RECT(items->data), sp_units_get_pixels(value, *unit));
            } else {
                SP_OBJECT_REPR(items->data)->setAttribute(value_name, NULL);
            }
            modmade = true;
        }
    }
    
    [self updateSensitivize];
    
    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_RECT,
                         _("Change rectangle"));
    }
    
    LEAVE_ONCE_CODE
}

- (void)updateSensitivize
{
    if (self.inputRx.doubleValue == 0 && self.inputRy.doubleValue == 0 && self.single) { // only for a single selected rect (for now)
        self.buttonReset.enabled = FALSE;
    } else {
        self.buttonReset.enabled = TRUE;
    }
}

- (IBAction)didResetRounded:(id)sender
{
    self.inputRx.doubleValue = 0.f;
    [self didChangeRx:nill];
    self.inputRy.doubleValue = 0.f;
    [self didChangeRy:nill];
    [self updateSensitivize];
}

- (IBAction)didChangeWidth:(id)sender
{
    [self didValueChangedWithValueName:"width"
                                 value:self.inputWidth.doubleValue
                                setter:sp_rect_set_visible_width];
}

- (IBAction)didChangeHeight:(id)sender
{
    [self didValueChangedWithValueName:"height"
                                 value:self.inputHeight.doubleValue
                                setter:sp_rect_set_visible_height];
}

- (IBAction)didChangeRx:(id)sender
{
    [self didValueChangedWithValueName:"rx"
                                 value:self.inputRx.doubleValue
                                setter:sp_rect_set_visible_rx];
}

- (IBAction)didChangeRy:(id)sender
{
    [self didValueChangedWithValueName:"ry"
                                 value:self.inputRy.doubleValue
                                setter:sp_rect_set_visible_ry];
}

- (void)didSelectionChanged:(NSNotification *)aNotification
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;
    SPItem *item = NULL;
    
    if ( self.repr ) {
        self.item = NULL;
    }
    purge_repr_listener( NULL, (__bridge gpointer)self) ;
    
    for (GSList const *items = sp_desktop_selection(self.desktop)->itemList();
         items != NULL;
         items = items->next) {
        if (SP_IS_RECT((SPItem *) items->data)) {
            n_selected++;
            item = (SPItem *) items->data;
            repr = SP_OBJECT_REPR(item);
        }
    }
    
    self.single = FALSE;
    
    if (n_selected == 0) {
        [self.labelMode setStringValue:@"New:"];
        self.inputWidth.enabled = FALSE;
        self.inputHeight.enabled = FALSE;
    } else if (n_selected == 1) {
        [self.labelMode setStringValue:@"Change:"];
        self.single = TRUE;
        self.inputWidth.enabled = TRUE;
        self.inputHeight.enabled = TRUE;
        
        if (repr) {
            self.repr = repr;
            self.item = item;
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &rect_tb_repr_events, (__bridge gpointer)self);
            sp_repr_synthesize_events(repr, &rect_tb_repr_events, (__bridge gpointer)self);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        [self.labelMode setStringValue:@"Change:"];
        [self updateSensitivize];
    }
}

- (void)didReprAttrChanged
{
    ENTER_ONCE_CODE
    
    SPUnit const *unit = sp_unit_get_by_abbreviation("px");
    
    gpointer item = self.item;
    if (item && SP_IS_RECT(item)) {
        {
            gdouble rx = sp_rect_get_visible_rx(SP_RECT(item));
            self.inputRx.doubleValue = sp_pixels_get_units(rx, *unit);
        }
        
        {
            gdouble ry = sp_rect_get_visible_ry(SP_RECT(item));
            self.inputRy.doubleValue = sp_pixels_get_units(ry, *unit);
        }
        
        {
            gdouble width = sp_rect_get_visible_width (SP_RECT(item));
            self.inputWidth.doubleValue = sp_pixels_get_units(width, *unit);
        }
        
        {
            gdouble height = sp_rect_get_visible_height (SP_RECT(item));
            self.inputHeight.doubleValue = sp_pixels_get_units(height, *unit);
        }
    }
    
    [self updateSensitivize];
    
    LEAVE_ONCE_CODE
}

@end
