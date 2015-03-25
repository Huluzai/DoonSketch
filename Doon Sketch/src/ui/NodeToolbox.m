//
//  NodeToolbox.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/2.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "NodeToolbox.h"
#import "ZGSelection.h"
#import "Toolbox.h"
#import "ZGDesktop.h"
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

/** Temporary hack: Returns the node tool in the active desktop.
 * Will go away during tool refactoring. */
static InkNodeTool *get_node_tool()
{
    InkNodeTool *tool = 0;
    if (SP_ACTIVE_DESKTOP ) {
        SPEventContext *ec = SP_ACTIVE_DESKTOP->event_context;
        if (INK_IS_NODE_TOOL(ec)) {
            tool = static_cast<InkNodeTool*>(ec);
        }
    }
    return tool;
}

@interface NodeToolbox () {
    BOOL _enabled;
}
@property NSArray *mainActions;
@property ZGSelection *zSel;
@property ZGDesktop *zDt;
@property BOOL update;
@property BOOL freeze;
@property SPDesktop *desktop;
@property ZGPreferencesNode *zPref;

- (void)didSelectionChanged:(NSNotification *)aNotification;

@end


@implementation NodeToolbox

- (id)initWithSPDesktop:(SPDesktop *)desktop
{
    if (self = [super initWithNibName:@"NodeToolbox" bundle:nill]) {
        self.desktop = Inkscape::GC::anchor(desktop);
        self.zPref = [[ZGPreferencesNode alloc] initWithNodePath:@"/tools/nodes/"];
    }
    return self;
}

- (void)dealloc
{
    [defaultNfc removeObserver:self];
    self.zPref = nill;
    Inkscape::GC::release(self.desktop);
    self.desktop = NULL;
    self.mainActions = nill;
    self.zSel = nill;
    self.zDt = nill;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.

    self.zSel = [[ZGSelection alloc] initWithSPDesktop:self.desktop];
    self.zDt = [[ZGDesktop alloc] initWithSPDesktop:self.desktop];
    self.inputX.enabled = FALSE;
    self.inputX.doubleValue = GlbPrefs->getDouble("/tools/nodes/Xcoord", 0);
    self.inputY.enabled = FALSE;
    self.inputY.doubleValue = GlbPrefs->getDouble("/tools/nodes/Ycoord", 0);
    [self.segmentShowHandles setSelected:GlbPrefs->getBool("/tools/nodes/show_transform_handles", 0) forSegment:0];
    [self.segmentShowHandles setSelected:GlbPrefs->getBool("/tools/nodes/show_handles", 0) forSegment:1];
    [self.segmentShowHandles setSelected:GlbPrefs->getBool("/tools/nodes/show_outline", 0) forSegment:2];
    [self.segmentShowPaths setSelected:GlbPrefs->getBool("/tools/nodes/edit_clipping_paths", 0) forSegment:0];
    [self.segmentShowPaths setSelected:GlbPrefs->getBool("/tools/nodes/edit_masks", 0) forSegment:1];
//    [self.segmentShowPaths setSelected:GlbPrefs->getBool("/tools/nodes/show_outline", 0) forSegment:2];
    
    
    [self didSelectionChanged:nill];
    
    [defaultNfc addObserver:self
                   selector:@selector(didSelectionChanged:)
                       name:kZGSelectionDidChangedNotification
                     object:self.zSel];
    [defaultNfc addObserver:self
                   selector:@selector(didSelectionModified:)
                       name:kZGSelectionDidModifiedNotification
                     object:self.zSel];
    [defaultNfc addObserver:self
                   selector:@selector(didCoordChanged:)
                       name:kZGDesktopSubselectionChanged
                     object:self.zDt];
    [defaultNfc addObserver:self
                   selector:@selector(didPreferencesChanged:)
                       name:kZGPreferencesDidChangedValueNotification
                     object:self.zPref];
}

- (IBAction)didCoordValueChanged:(id)sender
{
    g_assert([sender respondsToSelector:@selector(tag)]);
    
    Geom::Dim2 d = (Geom::Dim2)[sender tag];
    SPDesktop *desktop = self.desktop;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    
    SPUnit const *unit = sp_unit_get_by_abbreviation("px");
    
    if (sp_document_get_undo_sensitive(sp_desktop_document(desktop))) {
        prefs->setDouble(Glib::ustring("/tools/nodes/") + (d == Geom::X ? "x" : "y"),
                         sp_units_get_pixels([sender doubleValue], *unit));
    }
    
    // quit if run by the attr_changed listener
    if (self.freeze) {
        return;
    }
    self.freeze = TRUE;
    
    InkNodeTool *nt = get_node_tool();
    if (nt && !nt->_selected_nodes->empty()) {
        double val = sp_units_get_pixels([sender doubleValue], *unit);
        double oldval = nt->_selected_nodes->pointwiseBounds()->midpoint()[d];
        Geom::Point delta(0,0);
        delta[d] = val - oldval;
        nt->_multipath->move(delta);
    }
    
    self.freeze = FALSE;
}

- (void)didSelectionChanged:(NSNotification *)aNotification
{
    SPItem *item = self.zSel.selection->singleItem();
    if (item && SP_IS_LPE_ITEM(item)) {
        if (sp_lpe_item_has_path_effect(SP_LPE_ITEM(item))) {
            self.checkLpeedit.enabled = TRUE;
        } else {
            self.checkLpeedit.enabled = FALSE;
        }
    } else {
        self.checkLpeedit.enabled = FALSE;
    }
}

- (void)didSelectionModified:(NSNotification *)aNotificaiton
{
    [self didSelectionChanged:nill];
}

/* is called when the node selection is modified */
- (void)didCoordChanged:(NSNotification *)aNotification
{
    // quit if run by the attr_changed listener
    if (self.freeze) {
        return;
    }
    
    // in turn, prevent listener from responding
    self.freeze = TRUE;
    
    SPUnit const *unit = sp_unit_get_by_abbreviation("px");
    
    InkNodeTool *nt = get_node_tool();
    if (!nt || nt->_selected_nodes->empty()) {
        // no path selected
        self.inputX.enabled = FALSE;
        self.inputY.enabled = FALSE;
    } else {
        self.inputX.enabled = TRUE;
        self.inputY.enabled = TRUE;
        Geom::Coord oldx = sp_units_get_pixels(self.inputX.doubleValue, *unit);
        Geom::Coord oldy = sp_units_get_pixels(self.inputY.doubleValue, *unit);
        Geom::Point mid = nt->_selected_nodes->pointwiseBounds()->midpoint();
        
        if (oldx != mid[Geom::X]) {
            self.inputX.doubleValue = sp_pixels_get_units(mid[Geom::X], *unit);
        }
        if (oldy != mid[Geom::Y]) {
            self.inputY.doubleValue = sp_pixels_get_units(mid[Geom::Y], *unit);
        }
    }
    
    self.freeze = FALSE;
}

- (void)setEnabled:(BOOL)enabled
{
    self.inputX.enabled = enabled;
    self.inputY.enabled = enabled;
    _enabled = enabled;
}

- (BOOL)enabled
{
    return _enabled;
}

- (IBAction)didButtonAdd:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->insertNodes();
    }
}

- (IBAction)didButtonDelete:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        nt->_multipath->deleteNodes(prefs->getBool("/tools/nodes/delete_preserves_shape", true));
    }
}

- (IBAction)didButtonDeleteSegment:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->deleteSegments();
    }
}

- (IBAction)didButtonBreak:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->breakNodes();
    }
}

- (IBAction)didButtonJoin:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->joinNodes();
    }
}

- (IBAction)didButtonJoinSegment:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->joinSegments();
    }
}

- (IBAction)didButtonToLine:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setSegmentType(Inkscape::UI::SEGMENT_STRAIGHT);
    }
}

- (IBAction)didButtonToCureve:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setSegmentType(Inkscape::UI::SEGMENT_CUBIC_BEZIER);
    }
}

- (IBAction)didButtonCusp:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_CUSP);
    }
}

- (IBAction)didButtonSmooth:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_SMOOTH);
    }
}

- (IBAction)didButtonSymmetrical:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_SYMMETRIC);
    }
}

- (IBAction)didButtonAuto:(id)sender
{
    InkNodeTool *nt = get_node_tool();
    if (nt) {
        nt->_multipath->setNodeType(Inkscape::UI::NODE_AUTO);
    }
}

- (IBAction)didButtonNextLPEparam:(id)sender
{
    sp_selection_next_patheffect_param( self.desktop );
}

#define EditClippingPaths ("/tools/nodes/edit_clipping_paths")
#define EditMasks ("/tools/nodes/edit_masks")
#define ShowHandles ("/tools/nodes/show_handles")
#define ShowOutline ("/tools/nodes/show_outline")
#define ShowTransformHandles ("/tools/nodes/show_transform_handles")
#define Pref    (Inkscape::Preferences::get())

- (IBAction)didSegmentToggled:(id)sender
{
    Pref->setBool(EditClippingPaths, [self.segmentShowPaths isSelectedForSegment:0]);
    Pref->setBool(EditMasks, [self.segmentShowPaths isSelectedForSegment:1]);
    Pref->setBool(ShowHandles, [self.segmentShowHandles isSelectedForSegment:0]);
    Pref->setBool(ShowOutline, [self.segmentShowHandles isSelectedForSegment:1]);
    Pref->setBool(ShowTransformHandles, [self.segmentShowHandles isSelectedForSegment:2]);
}

- (void)didPreferencesChanged:(NSNotification *)aNotification
{
    Inkscape::Preferences *pref = Inkscape::Preferences::get();
    NSString *path = [[aNotification userInfo] objectForKey:@"path"];
    
    if ([path isEqualTo:@(EditClippingPaths)]) {
        [self.segmentShowPaths setSelected:pref->getBool(EditClippingPaths) forSegment:0];
    } else if ([path isEqualTo:@(EditMasks)]) {
        [self.segmentShowPaths setSelected:pref->getBool(EditMasks) forSegment:1];
    } else if ([path isEqualTo:@(ShowHandles)]) {
        [self.segmentShowHandles setSelected:pref->getBool(ShowHandles) forSegment:0];
    } else if ([path isEqualTo:@(ShowOutline)]) {
        [self.segmentShowHandles setSelected:pref->getBool(ShowOutline) forSegment:1];
    } else if ([path isEqualTo:@(ShowTransformHandles)]) {
        [self.segmentShowHandles setSelected:pref->getBool(ShowTransformHandles) forSegment:2];
    }
}

@end
