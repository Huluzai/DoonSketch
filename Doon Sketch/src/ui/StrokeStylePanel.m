//
//  StrokeStylePanel.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-30.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#include "StrokeStylePanel.h"
#include "ZGSelection.h"
#include "NSView+Extension.h"

#include <desktop-handles.h>
#include <desktop-style.h>
#include <desktop.h>
#include <selection.h>
#include <inkscape.h>
#include <document-private.h>
#include <style.h>
#include <svg/css-ostringstream.h>
#include <ui/cache/svg_preview_cache.h>
#include <ui/icon-names.h>
#include <xml/repr.h>
#include <fill-or-stroke.h>
#include <preferences.h>

gchar const *const _prefs_path = "/palette/dashes";

static double dash_0[] = {-1.0};
static double dash_1_1[] = {1.0, 1.0, -1.0};
static double dash_2_1[] = {2.0, 1.0, -1.0};
static double dash_4_1[] = {4.0, 1.0, -1.0};
static double dash_1_2[] = {1.0, 2.0, -1.0};
static double dash_1_4[] = {1.0, 4.0, -1.0};

static double *builtin_dashes[] = {dash_0, dash_1_1, dash_2_1, dash_4_1, dash_1_2, dash_1_4, NULL};

static double **dashes = NULL;

static void
sp_stroke_style_set_scaled_dash(struct SPCSSAttr *css,
                                int ndash, double *dash, double offset,
                                double scale);

static void init_dashes()
{
    if (!dashes) {
        
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        std::vector<Glib::ustring> dash_prefs = prefs->getAllDirs(_prefs_path);
        
        if (!dash_prefs.empty()) {
            int pos = 0;
            SPStyle *style = sp_style_new (NULL);
            dashes = g_new (double *, dash_prefs.size() + 1);
            
            for (std::vector<Glib::ustring>::iterator i = dash_prefs.begin(); i != dash_prefs.end(); ++i) {
                sp_style_read_from_prefs(style, *i);
                
                if (style->stroke_dash.n_dash > 0) {
                    dashes[pos] = g_new (double, style->stroke_dash.n_dash + 1);
                    double *d = dashes[pos];
                    int i = 0;
                    for (; i < style->stroke_dash.n_dash; i++) {
                        d[i] = style->stroke_dash.dash[i];
                    }
                    d[i] = -1;
                } else {
                    dashes[pos] = dash_0;
                }
                pos += 1;
            }
            dashes[pos] = NULL;
        } else {
            dashes = builtin_dashes;
        }
    }
}

@interface StrokeStylePanel ()
@property BOOL update;
@property FillOrStroke kind;
@end

@implementation StrokeStylePanel

- (void)awakeFromNib
{
    init_dashes();
    [self buildDashesMenu];
    
    // Do view setup here.
    [self didSelectionChanged:nill];
}

- (IBAction)strokeStyleScaleLine:(id)sender
{
    if (self.update) {
        return;
    }
    
    g_assert(self.desktop);
    
    SPDocument *document = sp_desktop_document (self.desktop);
    Inkscape::Selection *selection = sp_desktop_selection (self.desktop);
    
    GSList const *items = selection->itemList();
    
    /* TODO: Create some standardized method */
    struct SPCSSAttr *css = sp_repr_css_attr_new();
    
    double *dashs = dashes[self.dashs];
    int ndash = 0;
    
    while (dashs[ndash] >= 0.0)
        ndash += 1;

    if (items) {
        for (GSList const *i = items; i != NULL; i = i->next) {
            /* Set stroke width */
            {
                Inkscape::CSSOStringStream os_width;
                os_width << self.width;
                sp_repr_css_set_property(css, "stroke-width", os_width.str().c_str());
            }
            
            {
                Inkscape::CSSOStringStream os_ml;
                os_ml << self.meterLimit;
                sp_repr_css_set_property(css, "stroke-miterlimit", os_ml.str().c_str());
            }
            
            /* Set dash */
            sp_stroke_style_set_scaled_dash(css, ndash, dashs, self.offset, self.width);
            
            sp_desktop_apply_css_recursive (SP_OBJECT(i->data), css, true);
        }
    }
    
    // we have already changed the items, so set style without changing selection
    // FIXME: move the above stroke-setting stuff, including percentages, to desktop-style
    sp_desktop_set_style (self.desktop, css, false);
    
    sp_repr_css_attr_unref(css);
    css = 0;
    
    sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE, "Set stroke style");
}

- (IBAction)didStrokeStyleAnyToggled:(id)sender
{
    if (self.update) {
        return;
    }
    
    static gchar joins[][10] = {"miter", "round", "bevel"};
    static gchar caps[][10] = {"butt", "round", "square"};
    
    if ([sender isEnabled]) {
        
        int join_type = (int)[self.segmentJoins selectedSegment];
        int cap_type = (int)[self.segmentCaps selectedSegment];
        
        if (sender == self.segmentJoins) {
            self.inputMeterLimit.enabled = [self.segmentJoins selectedSegment] == 0;
        }
        
        SPDesktop *desktop = self.desktop;
        
        /* TODO: Create some standardized method */
        SPCSSAttr *css = sp_repr_css_attr_new();
        
        if (sender == self.segmentJoins) {
            sp_repr_css_set_property(css, "stroke-linejoin", joins[join_type]);
            
            sp_desktop_set_style (desktop, css);
            
            [self setJoinSegments:join_type];
        } else if (sender == self.segmentCaps) {
            sp_repr_css_set_property(css, "stroke-linecap", caps[cap_type]);
            
            sp_desktop_set_style (desktop, css);
            
            [self setCapSegments:cap_type];
        }
        
        sp_repr_css_attr_unref(css);
        css = 0;
        
        sp_document_done(sp_desktop_document(desktop), SP_VERB_DIALOG_FILL_STROKE,
                         "Set stroke style");
    }
}

- (void)setJoinSegments:(int)join_type
{
    if (join_type < self.segmentJoins.segmentCount && join_type >= 0) {
        [self.segmentJoins setSelectedSegment:join_type];
    } else {
        [self.segmentJoins setSelectedSegment:-1];
    }
    [self.inputMeterLimit setEnabled:join_type == 0];
}

- (void)setCapSegments:(int)cap_type
{
    if (cap_type < self.segmentCaps.segmentCount && cap_type >= 0) {
        [self.segmentCaps setSelectedSegment:cap_type];
    } else {
        [self.segmentCaps setSelectedSegment:-1];
    }
}

/**
 * Sets the join type for a line, and updates the stroke style widget's buttons
 */
- (void)setJoinType:(unsigned const )jointype
{
    [self setJoinSegments:jointype];
}

/**
 * Sets the cap type for a line, and updates the stroke style widget's buttons
 */
- (void)setCapType:(unsigned const )captype
{
    [self setCapSegments:captype];
}

- (void)updateWithSelection:(Inkscape::Selection *)sel
{
    if (self.update || !self.desktop) {
        return;
    }
    self.update = TRUE;
    
    // create temporary style
    SPStyle *query = sp_style_new (sp_desktop_document(self.desktop));
    // query into it
    int result_sw = sp_desktop_query_style (self.desktop, query, QUERY_STYLE_PROPERTY_STROKEWIDTH);
    int result_ml = sp_desktop_query_style (self.desktop, query, QUERY_STYLE_PROPERTY_STROKEMITERLIMIT);
    int result_cap = sp_desktop_query_style (self.desktop, query, QUERY_STYLE_PROPERTY_STROKECAP);
    int result_join = sp_desktop_query_style (self.desktop, query, QUERY_STYLE_PROPERTY_STROKEJOIN);
    SPIPaint &targPaint = query->stroke;
    
    if (!sel || sel->isEmpty()) {
        // Nothing selected, grey-out all controls in the stroke-style dialog
        [self.view setSensitive:FALSE];
        self.update = FALSE;
        return;
    } else {
        [self.view setSensitive:TRUE];
        
        SPUnit const *unit = sp_unit_get_by_abbreviation("px");
        
        if (result_sw == QUERY_STYLE_MULTIPLE_AVERAGED) {
        } else {
            // same width, or only one object; no sense to keep percent, switch to absolute
            if (unit->base != SP_UNIT_ABSOLUTE && unit->base != SP_UNIT_DEVICE) {
            }
        }
        
        if (unit->base == SP_UNIT_ABSOLUTE || unit->base == SP_UNIT_DEVICE) {
            double avgwidth = sp_pixels_get_units (query->stroke_width.computed, *unit);
            self.width = avgwidth;
        } else {
            self.width = 100;
        }
        
        // if none of the selected objects has a stroke, than quite some controls should be disabled
        // The markers might still be shown though, so these will not be disabled
        bool enabled = (result_sw != QUERY_STYLE_NOTHING) && !targPaint.isNoneSet();
        /* No objects stroked, set insensitive */
        self.segmentJoins.enabled = enabled;
        self.inputMeterLimit.enabled = enabled;
        self.segmentCaps.enabled = enabled;
        self.popupDashes.enabled = enabled;
        self.inputOffset.enabled = enabled;
    }
    
    self.width = query->stroke_width.computed;
    
    if (result_ml != QUERY_STYLE_NOTHING) {
        self.meterLimit = query->stroke_miterlimit.value;
    }
    
    if (result_join != QUERY_STYLE_MULTIPLE_DIFFERENT) {
        [self setJoinType:query->stroke_linejoin.value];
    } else {
        [self setJoinSegments:-1];
    }
    
    if (result_cap != QUERY_STYLE_MULTIPLE_DIFFERENT) {
        [self setCapType:query->stroke_linecap.value];
    } else {
        [self setCapSegments:-1];
    }
    
    sp_style_unref(query);
    
    if (!sel || sel->isEmpty()) {
        self.update = FALSE;
        return;
    }
    
    GSList const *objects = sel->itemList();
    SPObject * const object = SP_OBJECT(objects->data);
    SPStyle * const style = SP_OBJECT_STYLE(object);
    
    /* Markers */
    //sp_stroke_style_update_marker_menus(spw, objects); // FIXME: make this desktop query too
    
    if (style && style->stroke_dash.n_dash > 0) {
        double d[64];
        int len = MIN(style->stroke_dash.n_dash, 64);
        for (int i = 0; i < len; i++) {
            if (style->stroke_width.computed != 0)
                d[i] = style->stroke_dash.dash[i] / style->stroke_width.computed;
            else
                d[i] = style->stroke_dash.dash[i]; // is there a better thing to do for stroke_width==0?
        }
        
        int offset = (style->stroke_width.computed != 0 ?
                      style->stroke_dash.offset / style->stroke_width.computed  :
                      style->stroke_dash.offset);
        
        [self setDash:len dashs:d offset:offset];
        
    } else {
        [self setDash:0 dashs:NULL offset:0.0];
    }
    
    self.update = FALSE;
}

- (void)buildDashesMenu
{
    [self.popupDashes removeAllItems];
    
    for (int i = 0; dashes[i]; i++) {
        int ndash = 0;
        double *p = dashes[i];
    
 //       NSMenuItem *mi = [[NSMenuItem alloc] init];
 //       mi.target = self;
        NSString *title = @"Dash ";
        
        while (p[ndash] >= 0.0) {
            title = [title stringByAppendingString:[NSString stringWithFormat:@" %.1f", p[ndash]]];
            ndash += 1;
        }
        
        if (ndash == 0) {
            title = @"Solid";
        }
        
        [self.popupDashes addItemWithTitle:title];
    }
}

- (void)setDash:(int)len dashs:(double[])dashs offset:(int)offset
{
    for (int i = 0; dashes[i]; i++) {
        int ndash = 0;
        double *p = dashes[i];
        
        while (p[ndash] >= 0.0 && ndash < len) {
            if (!ZGFloatEqualToFloat(p[ndash], dashs[ndash])) {
                break;
            }
            ndash += 1;
        }
        
        if (ndash == len) {
            self.dashs = i;
            break;
        }
    }
}

- (void)didSelectionModified:(NSNotification *)aNotification
{
    [self updateWithSelection:self.zSel.selection];
    [super didSelectionModified:aNotification];
}

- (void)didSelectionChanged:(NSNotification *)aNotification
{
    [self updateWithSelection:self.zSel.selection];
    [super didSelectionChanged:aNotification];
}

@end


static void
sp_stroke_style_set_scaled_dash(SPCSSAttr *css,
                                int ndash, double *dash, double offset,
                                double scale)
{
    if (ndash > 0) {
        Inkscape::CSSOStringStream osarray;
        for (int i = 0; i < ndash; i++) {
            osarray << dash[i] * scale;
            if (i < (ndash - 1)) {
                osarray << ",";
            }
        }
        sp_repr_css_set_property(css, "stroke-dasharray", osarray.str().c_str());
        
        Inkscape::CSSOStringStream osoffset;
        osoffset << offset * scale;
        sp_repr_css_set_property(css, "stroke-dashoffset", osoffset.str().c_str());
    } else {
        sp_repr_css_set_property(css, "stroke-dasharray", "none");
        sp_repr_css_set_property(css, "stroke-dashoffset", NULL);
    }
}

