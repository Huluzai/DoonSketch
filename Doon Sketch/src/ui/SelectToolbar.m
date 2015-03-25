//
//  SelectToolbar.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "SelectToolbar.h"
#import "CocoaAction.h"
#import "ZGSelection.h"
#import "Toolbox.h"

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

@interface SelectToolbar ()
@property SPDesktop *desktop;
@property ZGSelection *zSel;
@property BOOL update;
@property NSArray *contextControls;
@end

@implementation SelectToolbar

- (id)initWithDesktop:(SPDesktop *)desktop
{
    if (self = [super initWithNibName:@"SelectToolbar" bundle:nill]) {
        self.desktop = Inkscape::GC::anchor(desktop);
        self.zSel = [[ZGSelection alloc] initWithSPDesktop:self.desktop];
    }
    return self;
}

- (void)dealloc
{
    self.contextControls = nill;
    [defaultNfc removeObserver:self];
    self.zSel = nill;
    Inkscape::GC::release(self.desktop);
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    
    self.contextControls = [NSArray array];
    self.contextControls = [self.contextControls arrayByAddingObject:self.buttonDeselect];
    self.contextControls = [self.contextControls arrayByAddingObject:self.buttonRotate90CCW];
    self.contextControls = [self.contextControls arrayByAddingObject:self.buttonRotate90CW];
    self.contextControls = [self.contextControls arrayByAddingObject:self.buttonFlipHorizontal];
    self.contextControls = [self.contextControls arrayByAddingObject:self.buttonFlipVertical];
    self.contextControls = [self.contextControls arrayByAddingObject:self.buttonSelectionToBack];
    self.contextControls = [self.contextControls arrayByAddingObject:self.buttonSelectionToFront];
    self.contextControls = [self.contextControls arrayByAddingObject:self.buttonSelectionRaise];
    self.contextControls = [self.contextControls arrayByAddingObject:self.buttonSelectionLower];
    self.contextControls = [self.contextControls arrayByAddingObject:self.inputX];
    self.contextControls = [self.contextControls arrayByAddingObject:self.inputY];
    self.contextControls = [self.contextControls arrayByAddingObject:self.inputWidth];
    self.contextControls = [self.contextControls arrayByAddingObject:self.inputHeight];
    
    [self.segmentAffect setSelected:GlbPrefs->getBool("/options/transform/stroke", 0) forSegment:0];
    [self.segmentAffect setSelected:GlbPrefs->getBool("/options/transform/rectcorners", 0) forSegment:1];
    [self.segmentAffect setSelected:GlbPrefs->getBool("/options/transform/gradient", 0) forSegment:2];
    [self.segmentAffect setSelected:GlbPrefs->getBool("/options/transform/pattern", 0) forSegment:3];

    [self didSelectionChanged:nill];
    
    [defaultNfc addObserver:self
                   selector:@selector(didSelectionChanged:)
                       name:kZGSelectionDidChangedNotification
                     object:self.zSel];
    [defaultNfc addObserver:self
                   selector:@selector(didSelectionModified:)
                       name:kZGSelectionDidModifiedNotification
                     object:self.zSel];
}

- (IBAction)didChangeAnyValue:(id)sender
{
    if (self.update || !self.desktop) {
        return;
    }
    self.update = TRUE;
    
    Inkscape::Selection *selection = sp_desktop_selection(self.desktop);
    SPDocument *document = sp_desktop_document(self.desktop);
    
    sp_document_ensure_up_to_date (document);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int prefs_bbox = prefs->getInt("/tools/bounding_box");
    SPItem::BBoxType bbox_type = (prefs_bbox ==0)?
    SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;
    Geom::OptRect bbox = selection->bounds(bbox_type);
    
    if ( !bbox ) {
        self.update = FALSE;
        return;
    }
    
    gdouble x0 = 0;
    gdouble y0 = 0;
    gdouble x1 = 0;
    gdouble y1 = 0;
    gdouble xrel = 0;
    gdouble yrel = 0;
    SPUnit const *unit = sp_unit_get_by_abbreviation("px");
    
    if (unit->base == SP_UNIT_ABSOLUTE || unit->base == SP_UNIT_DEVICE) {
        x0 = sp_units_get_pixels (self.inputX.doubleValue, *unit);
        y0 = sp_units_get_pixels (self.inputY.doubleValue, *unit);
        x1 = x0 + sp_units_get_pixels (self.inputWidth.doubleValue, *unit);
        xrel = sp_units_get_pixels (self.inputWidth.doubleValue, *unit) / bbox->dimensions()[Geom::X];
        y1 = y0 + sp_units_get_pixels (self.inputHeight.doubleValue, *unit);
        yrel = sp_units_get_pixels (self.inputHeight.doubleValue, *unit) / bbox->dimensions()[Geom::Y];
    } else {
        double const x0_propn = self.inputX.doubleValue * unit->unittobase;
        x0 = bbox->min()[Geom::X] * x0_propn;
        double const y0_propn = self.inputY.doubleValue * unit->unittobase;
        y0 = y0_propn * bbox->min()[Geom::Y];
        xrel = self.inputWidth.doubleValue * unit->unittobase;
        x1 = x0 + xrel * bbox->dimensions()[Geom::X];
        yrel = self.inputHeight.doubleValue * unit->unittobase;
        y1 = y0 + yrel * bbox->dimensions()[Geom::Y];
    }
    
    // Keep proportions if lock is on
    if ( self.buttonLock.state == NSOnState ) {
        if (sender == self.inputHeight) {
            x1 = x0 + yrel * bbox->dimensions()[Geom::X];
        } else if (sender == self.inputWidth) {
            y1 = y0 + xrel * bbox->dimensions()[Geom::Y];
        }
    }
    
    // scales and moves, in px
    double mh = fabs(x0 - bbox->min()[Geom::X]);
    double sh = fabs(x1 - bbox->max()[Geom::X]);
    double mv = fabs(y0 - bbox->min()[Geom::Y]);
    double sv = fabs(y1 - bbox->max()[Geom::Y]);
    
    // unless the unit is %, convert the scales and moves to the unit
    if (unit->base == SP_UNIT_ABSOLUTE || unit->base == SP_UNIT_DEVICE) {
        mh = sp_pixels_get_units (mh, *unit);
        sh = sp_pixels_get_units (sh, *unit);
        mv = sp_pixels_get_units (mv, *unit);
        sv = sp_pixels_get_units (sv, *unit);
    }
    
    // do the action only if one of the scales/moves is greater than half the last significant
    // digit in the spinbox (currently spinboxes have 3 fractional digits, so that makes 0.0005). If
    // the value was changed by the user, the difference will be at least that much; otherwise it's
    // just rounding difference between the spinbox value and actual value, so no action is
    // performed
    char const * const actionkey = ( mh > 5e-4 ? "selector:toolbar:move:horizontal" :
                                    sh > 5e-4 ? "selector:toolbar:scale:horizontal" :
                                    mv > 5e-4 ? "selector:toolbar:move:vertical" :
                                    sv > 5e-4 ? "selector:toolbar:scale:vertical" : NULL );
    
    if (actionkey != NULL) {
        
        // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed
        sp_desktop_canvas(self.desktop)->force_full_redraw_after_interruptions(0);
        
        gdouble strokewidth = stroke_average_width (selection->itemList());
        int transform_stroke = prefs->getBool("/options/transform/stroke", true) ? 1 : 0;
        
        Geom::Matrix scaler = get_scale_transform_with_stroke (*bbox, strokewidth, transform_stroke, x0, y0, x1, y1);
        
        sp_selection_apply_affine(selection, scaler);
        sp_document_maybe_done (document, actionkey, SP_VERB_CONTEXT_SELECT,
                                "Transform by toolbar");
        
        // resume interruptibility
        sp_desktop_canvas(self.desktop)->end_forced_full_redraws();
    }
    
    self.update = FALSE;
}

- (IBAction)didChangeAffect:(id)sender
{
    g_assert([sender isKindOfClass:[NSSegmentedControl class]]);
    
    NSSegmentedControl *seg = (NSSegmentedControl *)sender;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    prefs->setBool("/options/transform/stroke", [seg isSelectedForSegment:0]);
    prefs->setBool("/options/transform/rectcorners", [seg isSelectedForSegment:1]);
    prefs->setBool("/options/transform/gradient", [seg isSelectedForSegment:2]);
    prefs->setBool("/options/transform/pattern", [seg isSelectedForSegment:3]);
}

- (void)didSelectionModified:(NSNotification *)aNotification
{
    NSNumber *val = [[aNotification userInfo] objectForKey:@"flags"];
    if (val && (val.unsignedIntValue & (SP_OBJECT_MODIFIED_FLAG        |
                                        SP_OBJECT_PARENT_MODIFIED_FLAG |
                                        SP_OBJECT_CHILD_MODIFIED_FLAG  ))) {
        [self updateControls];
    }
}

- (void)didSelectionChanged:(NSNotification *)aNotification
{
    gboolean setActive = !self.zSel.selection->isEmpty();
    for (NSControl *ctl in self.contextControls) {
        if (setActive != ctl.enabled) {
            [ctl setEnabled:setActive];
        }
    }
    
    [self updateControls];
}

- (void)updateControls
{
    if (self.update || !self.desktop) {
        return;
    }
    self.update = TRUE;
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Inkscape::Selection *sel = self.zSel.selection;
    using Geom::X;
    using Geom::Y;
    if ( sel && !sel->isEmpty() ) {
        int prefs_bbox = prefs->getInt("/tools/bounding_box", 0);
        SPItem::BBoxType bbox_type = (prefs_bbox ==0)?
        SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;
        Geom::OptRect const bbox(sel->bounds(bbox_type));
        if ( bbox ) {
            SPUnit const &unit = *sp_unit_get_by_abbreviation("px");
            
            struct { NSControl *key; double val; } const keyval[] = {
                { self.inputX, bbox->min()[X] },
                { self.inputY, bbox->min()[Y] },
                { self.inputWidth, bbox->dimensions()[X] },
                { self.inputHeight, bbox->dimensions()[Y] }
            };
            
            if (unit.base == SP_UNIT_DIMENSIONLESS) {
                double const val = 1. / unit.unittobase;
                for (unsigned i = 0; i < G_N_ELEMENTS(keyval); ++i) {
                    NSControl *a = keyval[i].key;
                    a.doubleValue = val;
//                    tracker->setFullVal( a, keyval[i].val );
                    g_assert_not_reached(); //zhangguangjian, not implementated
                }
            } else {
                for (unsigned i = 0; i < G_N_ELEMENTS(keyval); ++i) {
                    NSControl *a = keyval[i].key;
                    a.doubleValue = sp_pixels_get_units(keyval[i].val, unit);
                }
            }
        }
    }
    
    self.update = FALSE;
}

@end

