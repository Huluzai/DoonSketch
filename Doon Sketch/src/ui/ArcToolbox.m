//
//  ArcToolbox.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ArcToolbox.h"

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
#import <xml/repr.h>
#import <xml/node-event-vector.h>
#import <mod360.h>

@interface ArcToolbox ()
@property BOOL single;
@property Inkscape::XML::Node *repr;
- (void)didReprAttrChanged:(Inkscape::XML::Node *)repr;
@end

static void arc_tb_event_attr_changed(Inkscape::XML::Node *repr, gchar const * /*name*/,
                                      gchar const * /*old_value*/, gchar const * /*new_value*/,
                                      bool /*is_interactive*/, gpointer data)
{
    ArcToolbox *toolbox = (__bridge ArcToolbox *)data;
    [toolbox didReprAttrChanged:repr];
}

static Inkscape::XML::NodeEventVector arc_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    arc_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

@implementation ArcToolbox

- (void)dealloc
{
    purge_repr_listener(NULL, (__bridge gpointer)self);
}

- (NSString *)toolboxNibName
{
    return @"ArcToolbox";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool isClosed = !prefs->getBool("/tools/shapes/arc/open", false);
    [self.segmentShape setSelectedSegment:isClosed ? 0 : 1];
    self.single = TRUE;
    self.inputStart.doubleValue = GlbPrefs->getDouble("/tools/shapes/arc/start", 0.0);
    self.inputEnd.doubleValue = GlbPrefs->getDouble("/tools/shapes/arc/end", 0.0);
    [self updateSensitivizeWithValue:0.f anotherValue:0.f];
}

- (void)didReprAttrChanged:(Inkscape::XML::Node *)repr
{
    ENTER_ONCE_CODE
    
    gdouble start = sp_repr_get_double_attribute(repr, "sodipodi:start", 0.0);
    gdouble end = sp_repr_get_double_attribute(repr, "sodipodi:end", 0.0);
    
    self.inputStart.doubleValue = mod360((start * 180)/M_PI);
    self.inputEnd.doubleValue = mod360((end * 180)/M_PI);
    
    [self updateSensitivizeWithValue:self.inputStart.doubleValue anotherValue:self.inputEnd.doubleValue];
    
    char const *openstr = NULL;
    openstr = repr->attribute("sodipodi:open");
    
    if (openstr) {
        [self.segmentShape setSelectedSegment:1];
    } else {
        [self.segmentShape setSelectedSegment:0];
    }
    
    LEAVE_ONCE_CODE
}

- (void)didChangeValueName:(gchar const *)value_name value:(double)value anotherValue:(double)otherValue
{
    SPDesktop *desktop = self.desktop;
    
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble(Glib::ustring("/tools/shapes/arc/") + value_name, value);
    }
    
    ENTER_ONCE_CODE
    
    gchar* namespaced_name = g_strconcat("sodipodi:", value_name, NULL);
    
    bool modmade = false;
    for (GSList const *items = sp_desktop_selection(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        SPItem *item = SP_ITEM(items->data);
        
        if (SP_IS_ARC(item) && SP_IS_GENERICELLIPSE(item)) {
            
            SPGenericEllipse *ge = SP_GENERICELLIPSE(item);
            SPArc *arc = SP_ARC(item);
            
            if (!strcmp(value_name, "start")) {
                ge->start = (value * M_PI)/ 180;
            } else {
                ge->end = (value * M_PI)/ 180;
            }
            
            sp_genericellipse_normalize(ge);
            ((SPObject *)arc)->updateRepr();
            ((SPObject *)arc)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            
            modmade = true;
        }
    }
    
    g_free(namespaced_name);

    [self updateSensitivizeWithValue:value anotherValue:otherValue];
    
    if (modmade) {
        sp_document_maybe_done(sp_desktop_document(desktop), value_name, SP_VERB_CONTEXT_ARC,
                               _("Arc: Change start/end"));
    }
    
    LEAVE_ONCE_CODE
}

- (void)updateSensitivizeWithValue:(double)v1 anotherValue:(double)v2
{
    if (v1 == 0 && v2 == 0) {
        if (self.single) { // only for a single selected ellipse (for now)
            self.segmentShape.enabled = FALSE;
            self.buttonWhole.enabled = FALSE;
        }
    } else {
        self.segmentShape.enabled = TRUE;
        self.buttonWhole.enabled = TRUE;
    }
}

- (IBAction)didChangeStart:(id)sender
{
    [self didChangeValueName:"start" value:self.inputStart.doubleValue anotherValue:self.inputEnd.doubleValue];
}

- (IBAction)didChangeEnd:(id)sender
{
    [self didChangeValueName:"end" value:self.inputEnd.doubleValue anotherValue:self.inputStart.doubleValue];
}

- (IBAction)didChangeShape:(id)sender
{
    SPDesktop *desktop = self.desktop;
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool("/tools/shapes/arc/open", ![self.segmentShape isSelectedForSegment:0]);
    }
    
    ENTER_ONCE_CODE
    
    bool modmade = false;
    
    if ( ![self.segmentShape isSelectedForSegment:0] ) {
        for (GSList const *items = sp_desktop_selection(desktop)->itemList();
             items != NULL;
             items = items->next)
        {
            if (SP_IS_ARC((SPItem *) items->data)) {
                Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
                repr->setAttribute("sodipodi:open", "true");
                SP_OBJECT((SPItem *) items->data)->updateRepr();
                modmade = true;
            }
        }
    } else {
        for (GSList const *items = sp_desktop_selection(desktop)->itemList();
             items != NULL;
             items = items->next)
        {
            if (SP_IS_ARC((SPItem *) items->data)) {
                Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
                repr->setAttribute("sodipodi:open", NULL);
                SP_OBJECT((SPItem *) items->data)->updateRepr();
                modmade = true;
            }
        }
    }
    
    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_ARC,
                         _("Arc: Change open/closed"));
    }
    
    LEAVE_ONCE_CODE
}

- (IBAction)didResetWhole:(id)sender
{
    self.inputStart.doubleValue = 0.f;
    [self didChangeStart:nill];
    self.inputEnd.doubleValue = 0.f;
    [self didChangeEnd:nill];
}

- (void)didSelectionChanged:(NSNotification *)aNotification
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;
    
    purge_repr_listener( NULL, (__bridge gpointer)self );
    
    for (GSList const *items = sp_desktop_selection(self.desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_ARC((SPItem *) items->data)) {
            n_selected++;
            repr = SP_OBJECT_REPR((SPItem *) items->data);
        }
    }
    
    self.single = FALSE;
    if (n_selected == 0) {
        self.labelMode.stringValue = @"New:";
    } else if (n_selected == 1) {
        self.single = TRUE;
        self.labelMode.stringValue = @"Change:";
        
        if (repr) {
            self.repr = repr;
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &arc_tb_repr_events, (__bridge gpointer)self);
            sp_repr_synthesize_events(repr, &arc_tb_repr_events, (__bridge gpointer)self);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        self.labelMode.stringValue = @"Change:";
        [self updateSensitivizeWithValue:1.0 anotherValue:0.f];
    }
}

@end
