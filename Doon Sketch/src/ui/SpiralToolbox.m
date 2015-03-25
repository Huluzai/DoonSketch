//
//  SpiralToolbox.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/4.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "SpiralToolbox.h"

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

@interface SpiralToolbox ()
@property Inkscape::XML::Node *repr;
- (void)didReprAttrChanged:(Inkscape::XML::Node *)repr;
@end

static void spiral_tb_event_attr_changed(Inkscape::XML::Node *repr,
                                         gchar const * /*name*/,
                                         gchar const * /*old_value*/,
                                         gchar const * /*new_value*/,
                                         bool /*is_interactive*/,
                                         gpointer data)
{
    SpiralToolbox *toolbox = (SpiralToolbox *)[Toolbox toolboxFromGPointer:data];
    [toolbox didReprAttrChanged:repr];
}

static Inkscape::XML::NodeEventVector spiral_tb_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    spiral_tb_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

@implementation SpiralToolbox

- (void)dealloc
{
    purge_repr_listener(NULL, self.gpointer);
}

- (NSString *)toolboxNibName
{
    return @"SpiralToolbox";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.inputTurns.doubleValue = GlbPrefs->getDouble("/tools/shapes/spiral/revolution", 3.0);
    self.inputDivergence.doubleValue = GlbPrefs->getDouble("/tools/shapes/spiral/expansion", 1.0);
    self.inputInnerRadius.doubleValue = GlbPrefs->getDouble("/tools/shapes/spiral/t0", 0.0);
}

- (void)didReprAttrChanged:(Inkscape::XML::Node *)repr
{
    ENTER_ONCE_CODE
    
    self.inputTurns.doubleValue = sp_repr_get_double_attribute(repr, "sodipodi:revolution", 3.0);
    self.inputDivergence.doubleValue = sp_repr_get_double_attribute(repr, "sodipodi:expansion", 1.0);
    self.inputInnerRadius.doubleValue = sp_repr_get_double_attribute(repr, "sodipodi:t0", 0.0);
    
    LEAVE_ONCE_CODE
}

- (void)didChangeValueName:(Glib::ustring const &)value_name value:(double)value
{
    SPDesktop *desktop = self.desktop;
    
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setDouble("/tools/shapes/spiral/" + value_name, value);
    }
    
    ENTER_ONCE_CODE
    
    gchar* namespaced_name = g_strconcat("sodipodi:", value_name.data(), NULL);
    
    bool modmade = false;
    for (GSList const *items = sp_desktop_selection(desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_SPIRAL((SPItem *) items->data)) {
            Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) items->data);
            sp_repr_set_svg_double( repr, namespaced_name, value );
            SP_OBJECT((SPItem *) items->data)->updateRepr();
            modmade = true;
        }
    }
    
    g_free(namespaced_name);
    
    if (modmade) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_SPIRAL,
                         _("Change spiral"));
    }
    
    LEAVE_ONCE_CODE
}

- (IBAction)didChangeTurns:(id)sender
{
    [self didChangeValueName:"revolution" value:self.inputTurns.doubleValue];
}

- (IBAction)didChangeDivergence:(id)sender
{
    [self didChangeValueName:"expansion" value:self.inputDivergence.doubleValue];
}

- (IBAction)didChangeInnerRadius:(id)sender
{
    [self didChangeValueName:"t0" value:self.inputInnerRadius.doubleValue];
}

- (IBAction)didResetDefaults:(id)sender
{
    // fixme: make settable
    gdouble rev = 3;
    gdouble exp = 1.0;
    gdouble t0 = 0.0;
    
    self.inputTurns.doubleValue = rev;
    [self didChangeTurns:nill];
    self.inputDivergence.doubleValue = exp;
    [self didChangeDivergence:nill];
    self.inputInnerRadius.doubleValue = t0;
    [self didChangeInnerRadius:nill];
}

- (void)didSelectionChanged:(NSNotification *)aNotification
{
    int n_selected = 0;
    Inkscape::XML::Node *repr = NULL;
    
    purge_repr_listener( NULL, self.gpointer );
    
    for (GSList const *items = sp_desktop_selection(self.desktop)->itemList();
         items != NULL;
         items = items->next)
    {
        if (SP_IS_SPIRAL((SPItem *) items->data)) {
            n_selected++;
            repr = SP_OBJECT_REPR((SPItem *) items->data);
        }
    }
    
    if (n_selected == 0) {
        self.labelMode.stringValue = @"New:";
    } else if (n_selected == 1) {
        self.labelMode.stringValue = @"Change:";
        
        if (repr) {
            self.repr = repr;
            Inkscape::GC::anchor(repr);
            sp_repr_add_listener(repr, &spiral_tb_repr_events, self.gpointer);
            sp_repr_synthesize_events(repr, &spiral_tb_repr_events, self.gpointer);
        }
    } else {
        // FIXME: implement averaging of all parameters for multiple selected
        //gtk_label_set_markup(GTK_LABEL(l), _("<b>Average:</b>"));
        self.labelMode.stringValue = @"Change:";
    }
}

@end
