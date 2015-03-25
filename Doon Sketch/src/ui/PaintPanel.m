//
//  FillPanel.m
//  Inxcape
//
//  Created by 张 光建 on 14-10-11.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#include "PaintPanel.h"
#include "PaintSelector.h"
#include "ZGInkscape.h"
#include "ZGSelection.h"
#include "ZGDesktop.h"

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
#include <sp-gradient.h>
#include <sp-linear-gradient.h>
#include <sp-radial-gradient.h>
#include <sp-pattern.h>
#include <display/canvas-base.h>
#include <gradient-chemistry.h>

#define ART_WIND_RULE_NONZERO 0

@interface PaintPanel () {
    SPDesktop *_desktop;
}

@property BOOL update;
@property FillOrStroke kind;
@property PaintSelector *psel;
@property ZGSelection *zSel;
@property ZGDesktop *zDt;

- (void)performUpdate;

@end


@implementation PaintPanel

- (id)initWithDesktop:(SPDesktop *)desktop fillOrStroke:(FillOrStroke)kind
{
    if (self = [super initWithNibName:@"PaintPanel" bundle:nill]) {
        self.kind = kind;
        self.desktop = desktop;
        self.psel = [[PaintSelector alloc] initWithNibName:@"PaintSelector" bundle:nill];
        self.psel.delegate = self;
    }
    return self;
}

- (id)initWithFillOrStroke:(FillOrStroke)kind
{
    return [self initWithDesktop:NULL fillOrStroke:kind];
}

- (void)dealloc
{
    [defaultNfc removeObserver:self];
    self.psel = nill;
}

- (void)awakeFromNib
{
    [self.pselContent addSubview:self.psel.view];
    self.psel.checkFillRule.hidden = (self.kind == STROKE);
    [self performUpdate];
    [defaultNfc addObserver:self
                   selector:@selector(didFillRuleChanged:)
                       name:kPaintSelectorChangedFillRuleNotification
                     object:self.psel];
}

- (void)performUpdate
{
    if ( self.update || !self.desktop ) {
        return;
    }
    
    self.update = true;
    
    // create temporary style
    SPStyle *query = sp_style_new(self.desktop->doc());
    
    // query style from desktop into it. This returns a result flag and fills query with the style of subselection, if any, or selection
    int result = sp_desktop_query_style(self.desktop, query, (self.kind == FILL) ? QUERY_STYLE_PROPERTY_FILL : QUERY_STYLE_PROPERTY_STROKE);
    
    SPIPaint &targPaint = (self.kind == FILL) ? query->fill : query->stroke;
    SPIScale24 &targOpacity = (self.kind == FILL) ? query->fill_opacity : query->stroke_opacity;
    
    switch (result) {
        case QUERY_STYLE_NOTHING:
        {
            /* No paint at all */
            [self.psel setMode:SPPaintSelector::MODE_EMPTY];
            break;
        }
            
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_AVERAGED: // TODO: treat this slightly differently, e.g. display "averaged" somewhere in paint selector
        case QUERY_STYLE_MULTIPLE_SAME:
        {
            SPPaintSelector::Mode pselmode = [PaintSelector getModeForStyle:*query fillOrStroke:self.kind];
            [self.psel setMode:pselmode];
            
            if (self.kind == FILL) {
                [self.psel setFillRule:query->fill_rule.computed == ART_WIND_RULE_NONZERO?
                       SPPaintSelector::FILLRULE_NONZERO : SPPaintSelector::FILLRULE_EVENODD];
            }
            
            if (targPaint.set && targPaint.isColor()) {
                [self.psel setColor:targPaint.value.color alpha:SP_SCALE24_TO_FLOAT(targOpacity.value)];
            } else if (targPaint.set && targPaint.isPaintserver()) {
                
                SPPaintServer *server = (self.kind == FILL) ? query->getFillPaintServer() : query->getStrokePaintServer();
                
                if (server && SP_IS_GRADIENT(server) && SP_GRADIENT(server)->getVector()->isSwatch()) {
                    SPGradient *vector = SP_GRADIENT(server)->getVector();
                    [self.psel setSwatch:vector];
                } else if (SP_IS_LINEARGRADIENT(server)) {
                    SPGradient *vector = SP_GRADIENT(server)->getVector();
                    [self.psel setGradientLinear:vector];
                    
                    SPLinearGradient *lg = SP_LINEARGRADIENT(server);
                    [self.psel setGradientProperties:lg->getUnits() spread:lg->getSpread()];
                } else if (SP_IS_RADIALGRADIENT(server)) {
                    SPGradient *vector = SP_GRADIENT(server)->getVector();
                    [self.psel setGradientRadial:vector];
                    
                    SPRadialGradient *rg = SP_RADIALGRADIENT(server);
                    [self.psel setGradientProperties:rg->getUnits() spread:rg->getSpread()];
                } else if (SP_IS_PATTERN(server)) {
                    SPPattern *pat = pattern_getroot(SP_PATTERN(server));
                    [self.psel updatePatternList:pat];
                }
            }
            break;
        }
            
        case QUERY_STYLE_MULTIPLE_DIFFERENT:
        {
            [self.psel setMode:SPPaintSelector::MODE_MULTIPLE];
            break;
        }
    }
    
    sp_style_unref(query);
    
    self.update = false;
}

#pragma mark -
#pragma mark Paint selector delegate

- (void)didPaintSelector:(PaintSelector *)sel changeMode:(SPPaintSelector::Mode)mode
{
#ifdef SP_FS_VERBOSE
    g_message("didPaintSelector:%p changeMode:%d)", self, mode);
#endif
    if (self && !self.update) {
        [self updateFromPaint];
    }
}

- (void)didPaintSelectorChanged:(PaintSelector *)sel
{
#ifdef SP_FS_VERBOSE
    g_message("didPaintSelector:%p changeMode:%d)", self, mode);
#endif
    if (self && !self.update) {
        [self updateFromPaint];
    }
}

static gchar const *undo_F_label_1 = "fill:flatcolor:1";
static gchar const *undo_F_label_2 = "fill:flatcolor:2";

static gchar const *undo_S_label_1 = "stroke:flatcolor:1";
static gchar const *undo_S_label_2 = "stroke:flatcolor:2";

static gchar const *undo_F_label = undo_F_label_1;
static gchar const *undo_S_label = undo_S_label_1;

- (void)updateFromPaint
{
    if (!self.desktop) {
        return;
    }
    
    self.update = true;
    
    SPDocument *document = sp_desktop_document(self.desktop);
    Inkscape::Selection *selection = sp_desktop_selection(self.desktop);
    
    GSList const *items = selection->itemList();
    
    switch (self.psel.mode) {
        case SPPaintSelector::MODE_EMPTY:
            // This should not happen.
            g_warning( "file %s: line %d: Paint %d should not emit 'changed'",
                      __FILE__, __LINE__, self.psel.mode);
            break;
        case SPPaintSelector::MODE_MULTIPLE:
            // This happens when you switch multiple objects with different gradients to flat color;
            // nothing to do here.
            break;
            
        case SPPaintSelector::MODE_NONE:
        {
            SPCSSAttr *css = sp_repr_css_attr_new();
            sp_repr_css_set_property(css, (self.kind == FILL) ? "fill" : "stroke", "none");
            
            sp_desktop_set_style(self.desktop, css);
            
            sp_repr_css_attr_unref(css);
            css = 0;
            
            sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                             (self.kind == FILL) ? ("Remove fill") : ("Remove stroke"));
            break;
        }
            
        case SPPaintSelector::MODE_COLOR_RGB:
        case SPPaintSelector::MODE_COLOR_CMYK:
        {
            if (self.kind == FILL) {
                // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in losing release events
                sp_desktop_canvas(self.desktop)->force_full_redraw_after_interruptions(0);
            }
            
            [self.psel setFlatColor:self.desktop
                      colorProperty:(self.kind == FILL) ? "fill" : "stroke"
                            opacity:(self.kind == FILL) ? "fill-opacity" : "stroke-opacity"];
            sp_document_maybe_done(document,
                                   (self.kind == FILL) ? undo_F_label : undo_S_label,
                                   SP_VERB_DIALOG_FILL_STROKE,
                                   (self.kind == FILL) ? ("Set fill color") : ("Set stroke color"));
            
            if (self.kind == FILL) {
                // resume interruptibility
                sp_desktop_canvas(self.desktop)->end_forced_full_redraws();
            }
            
            // on release, toggle undo_label so that the next drag will not be lumped with this one
            if (undo_F_label == undo_F_label_1) {
                undo_F_label = undo_F_label_2;
                undo_S_label = undo_S_label_2;
            } else {
                undo_F_label = undo_F_label_1;
                undo_S_label = undo_S_label_1;
            }
            
            break;
        }
            
        case SPPaintSelector::MODE_GRADIENT_LINEAR:
        case SPPaintSelector::MODE_GRADIENT_RADIAL:
        case SPPaintSelector::MODE_SWATCH:
            if (items) {
                SPGradientType const gradient_type = (self.psel.mode != SPPaintSelector::MODE_GRADIENT_RADIAL
                                                      ? SP_GRADIENT_TYPE_LINEAR
                                                      : SP_GRADIENT_TYPE_RADIAL );
                bool createSwatch = (self.psel.mode == SPPaintSelector::MODE_SWATCH);
                
                SPCSSAttr *css = 0;
                if (self.kind == FILL) {
                    // HACK: reset fill-opacity - that 0.75 is annoying; BUT remove this when we have an opacity slider for all tabs
                    css = sp_repr_css_attr_new();
                    sp_repr_css_set_property(css, "fill-opacity", "1.0");
                }
                
                SPGradient *vector = [self.psel getGradientVector];
                
                if (!vector) {
                    /* No vector in paint selector should mean that we just changed mode */
                    
                    SPStyle *query = sp_style_new(self.desktop->doc());
                    int result = objects_query_fillstroke(const_cast<GSList *>(items), query, self.kind == FILL);
                    if (result == QUERY_STYLE_MULTIPLE_SAME) {
                        SPIPaint &targPaint = (self.kind == FILL) ? query->fill : query->stroke;
                        SPColor common;
                        if (!targPaint.isColor()) {
                            common = sp_desktop_get_color(self.desktop, self.kind == FILL);
                        } else {
                            common = targPaint.value.color;
                        }
                        vector = sp_document_default_gradient_vector(document, common, createSwatch );
                        g_debug("created new vector : %s[%p], doc = %p", vector->getId(), vector, SP_OBJECT_DOCUMENT(vector));
                        
                        if ( vector && createSwatch ) {
                            vector->setSwatch();
                        }
                    }
                    sp_style_unref(query);
                    
                    for (GSList const *i = items; i != NULL; i = i->next) {
                        //FIXME: see above
                        if (self.kind == FILL) {
                            sp_repr_css_change_recursive(reinterpret_cast<SPObject*>(i->data)->repr, css, "style");
                        }
                        
                        SPGradient *old = sp_item_gradient(SP_ITEM(i->data), self.kind == FILL);
                        
                        if (old) {
                            g_debug("Old vector _total_hrefcount = %d before set", old->_total_hrefcount);
                        }
                        
                        if (!vector) {
                            SPGradient *gr = sp_gradient_vector_for_object(document,
                                                                           self.desktop,
                                                                           reinterpret_cast<SPObject*>(i->data),
                                                                           self.kind == FILL,
                                                                           createSwatch );
                            g_debug("created new vector : %s[%p], doc = %p", gr->getId(), gr, SP_OBJECT_DOCUMENT(gr));

                            if ( gr && createSwatch ) {
                                gr->setSwatch();
                            }
                            sp_item_set_gradient(SP_ITEM(i->data),
                                                 gr,
                                                 gradient_type, self.kind == FILL);
                        } else {
                            sp_item_set_gradient(SP_ITEM(i->data), vector, gradient_type, self.kind == FILL);
                        }
                        
                        if (old) {
                            g_debug("Old vector _total_hrefcount = %d after set", old->_total_hrefcount);
                        }
                    }
                } else {
                    // We have changed from another gradient type, or modified spread/units within
                    // this gradient type.
                    vector = sp_gradient_ensure_vector_normalized(vector);
                    for (GSList const *i = items; i != NULL; i = i->next) {
                        //FIXME: see above
                        if (self.kind == FILL) {
                            sp_repr_css_change_recursive(reinterpret_cast<SPObject*>(i->data)->repr, css, "style");
                        }
                        
                        SPGradient *old = sp_item_gradient(SP_ITEM(i->data), self.kind == FILL);
                        
                        if (old) {
                            g_debug("Old vector %p[%s] _total_hrefcount = %d before set", old, old->getId(), old->_total_hrefcount);
                        }
                        
                        SPGradient *gr = sp_item_set_gradient(SP_ITEM(i->data), vector, gradient_type, self.kind == FILL);
                        [self.psel pushAttrsToGradient:gr];
                        
                        if (old) {
                            g_debug("Old vector _total_hrefcount = %d after set", old->_total_hrefcount);
                        }
                        
                        SPGradient *newVector = sp_item_gradient(SP_ITEM(i->data), self.kind == FILL);
                        if (newVector) {
                            g_debug("New vector is %p[%s]", newVector, newVector->getId());
                        }
                    }
                }
                
                if (css) {
                    sp_repr_css_attr_unref(css);
                    css = 0;
                }
                
                sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                                 (self.kind == FILL) ? ("Set gradient on fill") : ("Set gradient on stroke"));
            }
            break;
            
        case SPPaintSelector::MODE_PATTERN:
            
            if (items) {
                
                SPPattern *pattern = [self.psel getPattern];
                if (!pattern) {
                    
                    /* No Pattern in paint selector should mean that we just
                     * changed mode - dont do jack.
                     */
                    
                } else {
                    Inkscape::XML::Node *patrepr = pattern->repr;
                    SPCSSAttr *css = sp_repr_css_attr_new();
                    gchar *urltext = g_strdup_printf("url(#%s)", patrepr->attribute("id"));
                    sp_repr_css_set_property(css, (self.kind == FILL) ? "fill" : "stroke", urltext);
                    
                    // HACK: reset fill-opacity - that 0.75 is annoying; BUT remove this when we have an opacity slider for all tabs
                    if (self.kind == FILL) {
                        sp_repr_css_set_property(css, "fill-opacity", "1.0");
                    }
                    
                    // cannot just call sp_desktop_set_style, because we don't want to touch those
                    // objects who already have the same root pattern but through a different href
                    // chain. FIXME: move this to a sp_item_set_pattern
                    for (GSList const *i = items; i != NULL; i = i->next) {
                        Inkscape::XML::Node *selrepr = reinterpret_cast<SPObject*>(i->data)->repr;
                        if ( (self.kind == STROKE) && !selrepr) {
                            continue;
                        }
                        SPObject *selobj = reinterpret_cast<SPObject*>(i->data);
                        
                        SPStyle *style = selobj->style;
                        if (style && ((self.kind == FILL) ? style->fill : style->stroke).isPaintserver()) {
                            SPPaintServer *server = (self.kind == FILL) ?
                            selobj->style->getFillPaintServer() :
                            selobj->style->getStrokePaintServer();
                            if (SP_IS_PATTERN(server) && pattern_getroot(SP_PATTERN(server)) == pattern)
                                // only if this object's pattern is not rooted in our selected pattern, apply
                                continue;
                        }
                        
                        if (self.kind == FILL) {
                            sp_desktop_apply_css_recursive(selobj, css, true);
                        } else {
                            sp_repr_css_change_recursive(selrepr, css, "style");
                        }
                    }
                    
                    sp_repr_css_attr_unref(css);
                    css = 0;
                    g_free(urltext);
                    
                } // end if
                
                sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                                 (self.kind == FILL) ? ("Set pattern on fill") :
                                 ("Set pattern on stroke"));
            } // end if
            
            break;
            
        case SPPaintSelector::MODE_UNSET:
            if (items) {
                SPCSSAttr *css = sp_repr_css_attr_new();
                if (self.kind == FILL) {
                    sp_repr_css_unset_property(css, "fill");
                } else {
                    sp_repr_css_unset_property(css, "stroke");
                    sp_repr_css_unset_property(css, "stroke-opacity");
                    sp_repr_css_unset_property(css, "stroke-width");
                    sp_repr_css_unset_property(css, "stroke-miterlimit");
                    sp_repr_css_unset_property(css, "stroke-linejoin");
                    sp_repr_css_unset_property(css, "stroke-linecap");
                    sp_repr_css_unset_property(css, "stroke-dashoffset");
                    sp_repr_css_unset_property(css, "stroke-dasharray");
                }
                
                sp_desktop_set_style(self.desktop, css);
                sp_repr_css_attr_unref(css);
                css = 0;
                
                sp_document_done(document, SP_VERB_DIALOG_FILL_STROKE,
                                 (self.kind == FILL) ? ("Unset fill") : ("Unset stroke"));
            }
            break;
            
        default:
            g_warning( "file %s: line %d: Paint selector should not be in "
                      "mode %d",
                      __FILE__, __LINE__,
                      self.psel.mode );
            break;
    }
    
    self.update = false;
}

#pragma mark -
#pragma mark Inkscape Delegate

- (void)didSelectionModified:(NSNotification *)aNotification
{
    NSNumber *val = [[aNotification userInfo] objectForKey:@"flags"];
    
    if (val) {
        int flags = [val unsignedIntValue];
        if (flags & ( SP_OBJECT_MODIFIED_FLAG |
                     SP_OBJECT_PARENT_MODIFIED_FLAG |
                     SP_OBJECT_STYLE_MODIFIED_FLAG) ) {
            [self performUpdate];
        }
    }
}

- (void)didSelectionChanged:(NSNotification *)aNotification
{
    [self performUpdate];
}

- (void)didSubselectionChanged:(NSNotification *)aNotification
{
    [self performUpdate];
}

- (void)didFillRuleChanged:(NSNotification *)aNotification
{
    SPPaintSelector::FillRule mode = self.psel.fillRule;
    
    if (!self.update && self.desktop) {
        SPCSSAttr *css = sp_repr_css_attr_new();
        sp_repr_css_set_property(css, "fill-rule", (mode == SPPaintSelector::FILLRULE_EVENODD) ? "evenodd":"nonzero");
        
        sp_desktop_set_style(self.desktop, css);
        
        sp_repr_css_attr_unref(css);
        css = 0;
        
        sp_document_done(self.desktop->doc(), SP_VERB_DIALOG_FILL_STROKE,
                         "Change fill rule");
    }
}

@end
