//
//  StarToolbox.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "StarToolbox.h"
#import "ZGSelection.h"

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

@interface StarToolbox ()
@property Inkscape::XML::Node *repr;
- (void)didReprAttrChangedWithRepr:(Inkscape::XML::Node *)repr name:(gchar const *)name;
@end

static void star_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                                       gchar const * /*old_value*/, gchar const * /*new_value*/,
                                       bool /*is_interactive*/, gpointer data);

static Inkscape::XML::NodeEventVector star_tb_repr_events =
{
    NULL, /* child_added */
    NULL, /* child_removed */
    star_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

void purge_repr_listener( GObject* /*obj*/, gpointer tbl )
{
    StarToolbox *toolbox = (__bridge StarToolbox*)tbl;
    Inkscape::XML::Node* oldrepr = toolbox.repr;
    if (oldrepr) { // remove old listener
        sp_repr_remove_listener_by_data(oldrepr, tbl);
        Inkscape::GC::release(oldrepr);
        oldrepr = 0;
        toolbox.repr = NULL;
    }
}

static void star_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const *name,
                                       gchar const * /*old_value*/, gchar const * /*new_value*/,
                                       bool /*is_interactive*/, __weak gpointer data)
{
    StarToolbox *toolbox = (__bridge StarToolbox *)data;
    [toolbox didReprAttrChangedWithRepr:repr name:name];
}

@implementation StarToolbox

- (void)dealloc
{
    purge_repr_listener(NULL, (__bridge gpointer)self);
}

- (NSString *)toolboxNibName
{
    return @"StarToolbox";
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool isFlatSided = prefs->getBool("/tools/shapes/star/isflatsided", true);
    [self.segmentSidesFlat setSelectedSegment:!isFlatSided];
    self.inputSpokeRatio.enabled = !isFlatSided;
    self.inputCorners.intValue = GlbPrefs->getInt("/tools/shapes/star/magnitude", 3);
    self.inputSpokeRatio.doubleValue = GlbPrefs->getDouble("/tools/shapes/star/proportion", 0.5);
    self.inputRounded.doubleValue = GlbPrefs->getDouble("/tools/shapes/star/rounded", 0.0);
    self.inputRandomized.intValue = GlbPrefs->getDouble("/tools/shapes/star/randomized", 0.0);
}

- (IBAction)didSidesflatChanged:(id)sender
{
    SPDesktop *desktop = self.desktop;
    bool flat = [self.segmentSidesFlat isSelectedForSegment:0];
    
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool( "/tools/shapes/star/isflatsided", flat);
    }
    
    // quit if run by the attr_changed listener
    if (_freeze) {
        return;
    }
    _freeze = TRUE;
    
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    bool modmade = false;
    
    self.inputSpokeRatio.enabled = !flat;
    
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            repr->setAttribute("inkscape:flatsided", flat ? "true" : "false" );
            SP_OBJECT((SPItem *) items->data)->updateRepr();
            modmade = true;
        }
    }
    
    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR,
                         flat ? _("Make polygon") : _("Make star"));
    }
    
    _freeze = FALSE;
}

- (IBAction)didMagnitudeChanged:(id)sender
{
    SPDesktop *desktop = self.desktop;
    
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        // do not remember prefs if this call is initiated by an undo change, because undoing object
        // creation sets bogus values to its attributes before it is deleted
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setInt("/tools/shapes/star/magnitude", self.inputCorners.intValue);
    }
    
    // quit if run by the attr_changed listener
    if (_freeze) {
        return;
    }
    _freeze = TRUE;
    
    bool modmade = false;
    
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_int(repr,"sodipodi:sides", self.inputCorners.intValue);
            sp_repr_set_svg_double(repr, "sodipodi:arg2",
                                   (sp_repr_get_double_attribute(repr, "sodipodi:arg1", 0.5)
                                    + M_PI / (gint)self.inputCorners.intValue));
            SP_OBJECT((SPItem *) items->data)->updateRepr();
            modmade = true;
        }
    }
    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR,
                         _("Star: Change number of corners"));
    }
    
    _freeze = FALSE;
}

- (IBAction)didProportionChanged:(id)sender
{
    SPDesktop *desktop = self.desktop;
    
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        if (!IS_NAN(self.inputSpokeRatio.doubleValue)) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            prefs->setDouble("/tools/shapes/star/proportion", self.inputSpokeRatio.doubleValue);
        }
    }
    
    // quit if run by the attr_changed listener
    if (_freeze) {
        return;
    }
    _freeze = TRUE;
    
    bool modmade = false;
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            
            gdouble r1 = sp_repr_get_double_attribute(repr, "sodipodi:r1", 1.0);
            gdouble r2 = sp_repr_get_double_attribute(repr, "sodipodi:r2", 1.0);
            if (r2 < r1) {
                sp_repr_set_svg_double(repr, "sodipodi:r2", r1*self.inputSpokeRatio.doubleValue);
            } else {
                sp_repr_set_svg_double(repr, "sodipodi:r1", r2*self.inputSpokeRatio.doubleValue);
            }
            
            SP_OBJECT((SPItem *) items->data)->updateRepr();
            modmade = true;
        }
    }
    
    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR,
                         _("Star: Change spoke ratio"));
    }
    
    _freeze = FALSE;
}

- (IBAction)didRoundedChanged:(id)sender
{
    SPDesktop *desktop = self.desktop;
    
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble("/tools/shapes/star/rounded", (gdouble) self.inputRounded.doubleValue);
    }
    
    // quit if run by the attr_changed listener
    if (_freeze) {
        return;
    }
    _freeze = TRUE;
    
    bool modmade = false;
    
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_svg_double(repr, "inkscape:rounded", (gdouble) self.inputRounded.doubleValue);
            SP_OBJECT(items->data)->updateRepr();
            modmade = true;
        }
    }
    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR,
                         _("Star: Change rounding"));
    }
    
    _freeze = FALSE;
}

- (IBAction)didRandomizedChanged:(id)sender
{
    SPDesktop *desktop = self.desktop;
    
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble("/tools/shapes/star/randomized", (gdouble) self.inputRandomized.doubleValue);
    }
    
    // quit if run by the attr_changed listener
    if (_freeze) {
        return;
    }
    _freeze = TRUE;
    
    bool modmade = false;
    
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        if (SP_IS_STAR((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_svg_double(repr, "inkscape:randomized", (gdouble) self.inputRandomized.doubleValue);
            SP_OBJECT(items->data)->updateRepr();
            modmade = true;
        }
    }
    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_STAR,
                         _("Star: Change randomization"));
    }
    
    _freeze = FALSE;
}

- (IBAction)didResetDefaults:(id)sender
{
    // FIXME: in this and all other _default functions, set some flag telling the value_changed
    // callbacks to lump all the changes for all selected objects in one undo step
    
    // fixme: make settable in prefs!
    gint mag = 5;
    gdouble prop = 0.5;
    gboolean flat = FALSE;
    gdouble randomized = 0;
    gdouble rounded = 0;
    
    [self.segmentSidesFlat setSelectedSegment:flat ? 0 : 1];
    self.inputSpokeRatio.enabled = !flat;
    [self.inputCorners setIntValue:mag];
    [self didMagnitudeChanged:nill];
    [self.inputSpokeRatio setDoubleValue:prop];
    [self didProportionChanged:nill];
    [self.inputRounded setDoubleValue:rounded];
    [self didRoundedChanged:nill];
    [self.inputRandomized setDoubleValue:randomized];
    [self didRandomizedChanged:nill];
}

- (void)didSelectionChanged:(NSNotification *)aNotification
{
    [super didSelectionChanged:aNotification];
    
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;
    
    purge_repr_listener( NULL, (__bridge gpointer)self);
    
    for (GSList const *items = sp_desktop_selection(self.desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_STAR((SPItem *) items->data)) {
            n_selected++;
            repr = SP_OBJECT_REPR((SPItem *) items->data);
        }
    }
    
    if (n_selected == 0) {
        [self.labelMode setStringValue:@"New:"];
    } else if (n_selected == 1) {
        [self.labelMode setStringValue:@"Change:"];
        
        if (repr) {
            self.repr = Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &star_tb_repr_events, (__bridge gpointer)self);
            sp_repr_synthesize_events(repr, &star_tb_repr_events, (__bridge gpointer)self);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected stars
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Change:</b>"));
    }
}

- (void)didReprAttrChangedWithRepr:(Inkscape::XML::Node *)repr name:(gchar const *)name
{
    // quit if run by the attr_changed listener
    if (_freeze) {
        return;
    }
    _freeze = TRUE;
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool isFlatSided = prefs->getBool("/tools/shapes/star/isflatsided", true);
    
    if (!strcmp(name, "inkscape:randomized")) {
        [self.inputRandomized setDoubleValue:sp_repr_get_double_attribute(repr, "inkscape:randomized", 0.0)];
    } else if (!strcmp(name, "inkscape:rounded")) {
        [self.inputRounded setDoubleValue:sp_repr_get_double_attribute(repr, "inkscape:rounded", 0.0)];
    } else if (!strcmp(name, "inkscape:flatsided")) {
        char const *flatsides = repr->attribute("inkscape:flatsided");
        if ( flatsides && !strcmp(flatsides,"false") ) {
            [self.segmentSidesFlat setSelectedSegment:1];
            self.inputSpokeRatio.enabled = TRUE;
        } else {
            [self.segmentSidesFlat setSelectedSegment:0];
            self.inputSpokeRatio.enabled = FALSE;
        }
    } else if ((!strcmp(name, "sodipodi:r1") || !strcmp(name, "sodipodi:r2")) && (!isFlatSided) ) {
        gdouble r1 = sp_repr_get_double_attribute(repr, "sodipodi:r1", 1.0);
        gdouble r2 = sp_repr_get_double_attribute(repr, "sodipodi:r2", 1.0);
        if (r2 < r1) {
            [self.inputSpokeRatio setDoubleValue:r2/r1];
        } else {
            [self.inputSpokeRatio setDoubleValue:r1/r2];
        }
    } else if (!strcmp(name, "sodipodi:sides")) {
        [self.inputCorners setIntValue:sp_repr_get_int_attribute(repr, "sodipodi:sides", 0)];
    }
    
    _freeze = FALSE;
}

@end
