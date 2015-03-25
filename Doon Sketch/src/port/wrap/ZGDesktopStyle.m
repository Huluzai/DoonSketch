//
//  ZGStyle.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/5.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGDesktopStyle.h"

#include <iostream>
#include <cstring>
#include <string>
#include <glibmm/i18n.h>
#include <glib.h>

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
#include <libnrtype/FontFactory.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/font-lister.h>

#include "ZGDocument.h"
#include "ZGDesktop.h"
#include "ZGInkscape.h"
#include "ZGSelection.h"

static void sp_text_fontfamily_value_changed( ZGDesktopStyle *tp, gchar *fontFamily );
static void sp_text_style_changed( ZGDesktopStyle *tp, gboolean bold, gboolean italic );
static void sp_text_align_mode_changed( ZGDesktopStyle *tp, int/*NSTextAlignment*/ alignMode );
static void sp_text_lineheight_value_changed( ZGDesktopStyle *tp, CGFloat lineHeight);
static void sp_text_fontsize_value_changed( ZGDesktopStyle *tp, CGFloat size );
static void sp_text_toolbox_selection_changed(ZGDesktopStyle *ds, Inkscape::Selection */*selection*/, GObject *tbl);
static void sp_text_orientation_mode_changed( ZGDesktopStyle *tp, int mode );
static void sp_text_rotation_value_changed( ZGDesktopStyle *tp, double new_val );
static void sp_text_dy_value_changed( ZGDesktopStyle *tp, double new_val );
static void sp_text_dx_value_changed( ZGDesktopStyle *tp, double new_val );
static void sp_text_letterspacing_value_changed( ZGDesktopStyle *tp, double new_val );
static void sp_text_wordspacing_value_changed( ZGDesktopStyle *tp, double new_val );
static void sp_text_script_changed( ZGDesktopStyle *tp, bool prop, int new_val );

@interface ZGDesktopStyle () {
    CGFloat _fontSize;
    NSString *_fontFamily;
    CGFloat _lineHeight;
    NSTextAlignment _textAlign;
    BOOL _fontBold;
    BOOL _fontItalic;
    BOOL _textOrientation;
    CGFloat _letterSpacing;
    CGFloat _wordSpacing;
    CGFloat _verticalShift;
    CGFloat _horizontalShift;
    CGFloat _characterRotation;
    BOOL _superscript;
    BOOL _subscript;
}
@property ZGDocument *doc;
@property ZGDesktop *dt;
@property BOOL freeze;
@property BOOL textStyleFromPrefs;
@property SPDesktop * desktop;
@property ZGSelection *sel;

@end

@implementation ZGDesktopStyle

- (id)initWithDesktop:(SPDesktop *)desktop
{
    if (self = [super init]) {
        self.desktop = Inkscape::GC::anchor(desktop);
        self.sel = [[ZGSelection alloc] initWithSPDesktop:self.desktop];
        self.sel.delegate = self;
        self.dt = [[ZGDesktop alloc] initWithSPDesktop:self.desktop];
        self.dt.delegate = self;
    }
    return self;
}

- (void)dealloc
{
    self.sel = nill;
    self.doc = nill;
    self.dt = nill;
    Inkscape::GC::release(self.desktop);
    self.desktop = NULL;
}
/*
- (void)didDesktop:(ZGDesktop *)desktop replacedDocument:(SPDocument *)doc
{
    self.doc = [[ZGDocument alloc] initWithSPDocument:doc];
    self.doc.delegate = self;
    [self didDocumentModified:self.doc];
}
*/

#pragma mark - setter & getter

- (void)setFontSize:(CGFloat)fontSize
{
    if (fontSize != _fontSize) {
        _fontSize = fontSize;
        sp_text_fontsize_value_changed(self, fontSize);
    }
}

- (CGFloat)fontSize
{
    return _fontSize;
}

+ (NSArray *)availableFontSizes
{
    return [NSArray arrayWithObjects:@4, @6, @8, @9, @10, @11, @12, @13, @14, @16, @18, @20, @22, @24, @28, @32, @36, @40, @48, @56, @64, @72, @144, NULL];
}

- (void)setFontFamily:(NSString *)fontFamily
{
    if (![_fontFamily isEqualToString:fontFamily]) {
        _fontFamily = fontFamily;
        sp_text_fontfamily_value_changed(self, (gchar*)[fontFamily UTF8String]);
    }
}

- (NSString *)fontFamily
{
    return _fontFamily;
}

- (void)setFontBold:(BOOL)fontBold
{
    if (fontBold != _fontBold) {
        _fontBold = fontBold;
        sp_text_style_changed(self, _fontBold, _fontItalic);
    }
}

- (BOOL)fontBold
{
    return _fontBold;
}

- (void)setFontItalic:(BOOL)fontItalic
{
    if (_fontItalic != fontItalic) {
        _fontItalic = fontItalic;
        sp_text_style_changed(self, _fontBold, _fontItalic);
    }
}

- (BOOL)fontItalic
{
    return _fontItalic;
}

- (void)setTextAlign:(NSTextAlignment)textAlign
{
    if (_textAlign != textAlign) {
        _textAlign = textAlign;
        sp_text_align_mode_changed(self, _textAlign);
    }
}

- (NSTextAlignment)textAlign
{
    return _textAlign;
}

- (void)setLineHeight:(CGFloat)lineHeight
{
    if (_lineHeight != lineHeight) {
        _lineHeight = lineHeight;
        sp_text_lineheight_value_changed(self, _lineHeight);
    }
}

- (CGFloat)lineHeight
{
    return _lineHeight;
}

- (void)setLetterSpacing:(CGFloat)letterSpacing
{
    if (_letterSpacing != letterSpacing) {
        _letterSpacing = letterSpacing;
        sp_text_letterspacing_value_changed(self, letterSpacing);
    }
}

- (CGFloat)letterSpacing
{
    return _letterSpacing;
}

- (void)setWordSpacing:(CGFloat)wordSpacing
{
    if (_wordSpacing != wordSpacing) {
        _wordSpacing = wordSpacing;
        sp_text_wordspacing_value_changed(self, wordSpacing);
    }
}

- (CGFloat)wordSpacing
{
    return _wordSpacing;
}

- (void)setVerticalShift:(CGFloat)verticalShift
{
    if (_verticalShift != verticalShift) {
        _verticalShift = verticalShift;
        sp_text_dy_value_changed(self, verticalShift);
    }
}

- (CGFloat)verticalShift
{
    return _verticalShift;
}

- (void)setHorizontalShift:(CGFloat)horizontalShift
{
    if (_horizontalShift != horizontalShift) {
        _horizontalShift = horizontalShift;
        sp_text_dx_value_changed(self, horizontalShift);
    }
}

- (CGFloat)horizontalShift
{
    return _horizontalShift;
}

- (void)setCharacterRotation:(CGFloat)characterRotation
{
    if (_characterRotation != characterRotation) {
        _characterRotation = characterRotation;
        sp_text_rotation_value_changed(self, characterRotation);
    }
}

- (CGFloat)characterRotation
{
    return _characterRotation;
}

- (void)setTextOrientation:(BOOL)textOrientation
{
    if (_textOrientation != textOrientation) {
        _textOrientation = textOrientation;
        sp_text_orientation_mode_changed(self, textOrientation);
    }
}

- (BOOL)textOrientation
{
    return _textOrientation;
}

- (void)setSuperscript:(BOOL)superscript
{
    if (_superscript != superscript) {
        _superscript = superscript;
        sp_text_script_changed(self, 0, superscript);
    }
}

- (BOOL)superscript
{
    return _superscript;
}

- (void)setSubscript:(BOOL)subscript
{
    if (_subscript != subscript) {
        _subscript = subscript;
        sp_text_script_changed(self, 1, subscript);
    }
}

- (BOOL)subscript
{
    return _subscript;
}

#pragma mark - Delegate
/*
- (void)didDocumentModified:(ZGDocument *)doc
{
    sp_text_toolbox_selection_changed(self, NULL, NULL);
}

- (void) didInkscape:(ZGInkscape *)inkscape modifySelection:(Inkscape::Selection *)selection withFlags:(guint)flags;
{
    sp_text_toolbox_selection_changed(self, NULL, NULL);
}

- (void) didInkscape:(ZGInkscape *)inkscape changeSelection:(Inkscape::Selection *)selection
{
    sp_text_toolbox_selection_changed(self, NULL, NULL);
}

- (void) didInkscape:(ZGInkscape *)inkscape setSelection:(Inkscape::Selection *)selection
{
    sp_text_toolbox_selection_changed(self, NULL, NULL);
}

- (void) didInkscape:(ZGInkscape *)inkscape changeSubselection:(Inkscape::Selection *)selection
{
    sp_text_toolbox_selection_changed(self, NULL, NULL);
}
*/

- (void)didSelectionModified:(ZGSelection *)zSel flags:(guint)flags
{
    sp_text_toolbox_selection_changed(self, NULL, NULL);
}

- (void)didSelectionChanged:(ZGSelection *)zSel
{
    sp_text_toolbox_selection_changed(self, NULL, NULL);
}

@end

// Font size
static void sp_text_fontsize_value_changed( ZGDesktopStyle *tp, CGFloat size )
{
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
    // Set css font size.
    SPCSSAttr *css = sp_repr_css_attr_new ();
    Inkscape::CSSOStringStream osfs;
    osfs << size << "px"; // For now always use px
    sp_repr_css_set_property (css, "font-size", osfs.str().c_str());
    
    // Apply font size to selected objects.
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    sp_desktop_set_style (desktop, css, true, true);
    
    // If no selected objects, set default.
    SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
    int result_numbers =
    sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->mergeStyle("/tools/text/style", css);
    } else {
        // Save for undo
        sp_document_maybe_done (sp_desktop_document (SP_ACTIVE_DESKTOP), "ttb:size", SP_VERB_NONE,
                                _("Text: Change font size"));
    }
    
    sp_style_unref(query);
    
    sp_repr_css_attr_unref (css);
    
    tp.freeze = FALSE;
}

/*
 * This function sets up the text-tool tool-controls, setting the entry boxes
 * etc. to the values from the current selection or the default if no selection.
 * It is called whenever a text selection is changed, including stepping cursor
 * through text.
 */
static void sp_text_toolbox_selection_changed(ZGDesktopStyle *ds, Inkscape::Selection */*selection*/, GObject *tbl)
{
#ifdef DEBUG_TEXT
    static int count = 0;
    ++count;
    std::cout << std::endl;
    std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;
    std::cout << "sp_text_toolbox_selection_changed: start " << count << std::endl;
    
    std::cout << "  Selected items:" << std::endl;
    for (GSList const *items = sp_desktop_selection(SP_ACTIVE_DESKTOP)->itemList();
         items != NULL;
         items = items->next)
    {
        const gchar* id = SP_OBJECT_ID((SPItem *) items->data);
        std::cout << "    " << id << std::endl;
    }
    Glib::ustring selected_text = sp_text_get_selected_text((SP_ACTIVE_DESKTOP)->event_context);
    std::cout << "  Selected text:" << std::endl;
    std::cout << selected_text << std::endl;
#endif
    
    // quit if run by the _changed callbacks
    if (ds.freeze) {
#ifdef DEBUG_TEXT
        std::cout << "    Frozen, returning" << std::endl;
        std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;
        std::cout << std::endl;
#endif
        return;
    }
    ds.freeze = TRUE;
    
    // Only flowed text can be justified, only normal text can be kerned...
    // Find out if we have flowed text now so we can use it several places
    gboolean isFlow = false;
    for (GSList const *items = sp_desktop_selection(ds.desktop)->itemList();
         items != NULL;
         items = items->next) {
        // const gchar* id = SP_OBJECT_ID((SPItem *) items->data);
        // std::cout << "    " << id << std::endl;
        if( SP_IS_FLOWTEXT(( SPItem *) items->data )) {
            isFlow = true;
            // std::cout << "   Found flowed text" << std::endl;
            break;
        }
    }
    
    /*
     * Query from current selection:
     *   Font family (font-family)
     *   Style (font-weight, font-style, font-stretch, font-variant, font-align)
     *   Numbers (font-size, letter-spacing, word-spacing, line-height, text-anchor, writing-mode)
     *   Font specification (Inkscape private attribute)
     */
    SPStyle *query =
    sp_style_new (ds.doc.spDocument);
    int result_family   = sp_desktop_query_style (ds.desktop, query, QUERY_STYLE_PROPERTY_FONTFAMILY);
    int result_style    = sp_desktop_query_style (ds.desktop, query, QUERY_STYLE_PROPERTY_FONTSTYLE);
    int result_numbers  = sp_desktop_query_style (ds.desktop, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);
    int result_baseline = sp_desktop_query_style (ds.desktop, query, QUERY_STYLE_PROPERTY_BASELINES);
    
    // Used later:
    sp_desktop_query_style (ds.desktop, query, QUERY_STYLE_PROPERTY_FONT_SPECIFICATION);
    
    /*
     * If no text in selection (querying returned nothing), read the style from
     * the /tools/text preferencess (default style for new texts). Return if
     * tool bar already set to these preferences.
     */
    if (result_family == QUERY_STYLE_NOTHING || result_style == QUERY_STYLE_NOTHING || result_numbers == QUERY_STYLE_NOTHING) {
        // There are no texts in selection, read from preferences.
        sp_style_read_from_prefs(query, "/tools/text");
#ifdef DEBUG_TEXT
        std::cout << "    read style from prefs:" << std::endl;
        sp_print_font( query );
#endif
        if (ds.textStyleFromPrefs) {
            // Do not reset the toolbar style from prefs if we already did it last time
            sp_style_unref(query);
            ds.freeze = FALSE;
#ifdef DEBUG_TEXT
            std::cout << "    text_style_from_prefs: toolbar already set" << std:: endl;
            std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;
            std::cout << std::endl;
#endif
            return;
        }
        
        ds.textStyleFromPrefs = TRUE;
    } else {
        ds.textStyleFromPrefs = FALSE;
    }
    
    // If we have valid query data for text (font-family, font-specification) set toolbar accordingly.
    if (query->text)
    {
        // Font family
        if( query->text->font_family.value ) {
            gchar *fontFamily = query->text->font_family.value;
            ds.fontFamily = [NSString stringWithUTF8String:fontFamily];
        }
        
        // Size (average of text selected)
        double size = query->font_size.computed;
        if (size != ds.fontSize) {
            ds.fontSize = size;
        }
        
        // Weight (Bold)
        // Note: in the enumeration, normal and lighter come at the end so we must explicitly test for them.
        gboolean boldSet = ((query->font_weight.computed >= SP_CSS_FONT_WEIGHT_700) &&
                            (query->font_weight.computed != SP_CSS_FONT_WEIGHT_NORMAL) &&
                            (query->font_weight.computed != SP_CSS_FONT_WEIGHT_LIGHTER));
        
        ds.fontBold = boldSet;
            
        // Style (Italic/Oblique)
        gboolean italicSet = (query->font_style.computed != SP_CSS_FONT_STYLE_NORMAL);
        
        ds.fontItalic = italicSet;
    
#if 0
        // Superscript
        gboolean superscriptSet =
        ((result_baseline == QUERY_STYLE_SINGLE || result_baseline == QUERY_STYLE_MULTIPLE_SAME ) &&
         query->baseline_shift.set &&
         query->baseline_shift.type == SP_BASELINE_SHIFT_LITERAL &&
         query->baseline_shift.literal == SP_CSS_BASELINE_SHIFT_SUPER );
        
        InkToggleAction* textSuperscriptAction = INK_TOGGLE_ACTION( g_object_get_data( tbl, "TextSuperscriptAction" ) );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(textSuperscriptAction), superscriptSet );
        
        
        // Subscript
        gboolean subscriptSet =
        ((result_baseline == QUERY_STYLE_SINGLE || result_baseline == QUERY_STYLE_MULTIPLE_SAME ) &&
         query->baseline_shift.set &&
         query->baseline_shift.type == SP_BASELINE_SHIFT_LITERAL &&
         query->baseline_shift.literal == SP_CSS_BASELINE_SHIFT_SUB );
        
        InkToggleAction* textSubscriptAction = INK_TOGGLE_ACTION( g_object_get_data( tbl, "TextSubscriptAction" ) );
        gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(textSubscriptAction), subscriptSet );
        
#endif
        // Alignment
        
        // Note: SVG 1.1 doesn't include text-align, SVG 1.2 Tiny doesn't include text-align="justify"
        // text-align="justify" was a draft SVG 1.2 item (along with flowed text).
        // Only flowed text can be left and right justified at the same time.
        // Disable button if we don't have flowed text.
        
        // The GtkTreeModel class doesn't have a set function so we can't
        // simply add an ege_select_one_action_set_sensitive method!
        // We must set values directly with the GtkListStore and then
        // ask that the GtkAction update the sensitive parameters.
        
        ds.textAlign = NSLeftTextAlignment;
        if (query->text_align.computed  == SP_CSS_TEXT_ALIGN_JUSTIFY)
        {
            ds.textAlign = NSJustifiedTextAlignment;
        } else {
            if (query->text_anchor.computed == SP_CSS_TEXT_ANCHOR_START)  ds.textAlign = NSLeftTextAlignment;
            if (query->text_anchor.computed == SP_CSS_TEXT_ANCHOR_MIDDLE) ds.textAlign = NSCenterTextAlignment;
            if (query->text_anchor.computed == SP_CSS_TEXT_ANCHOR_END)    ds.textAlign = NSRightTextAlignment;
        }
        
        // Line height (spacing)
        double height;
        if (query->line_height.normal) {
            height = Inkscape::Text::Layout::LINE_HEIGHT_NORMAL;
        } else {
            if (query->line_height.unit == SP_CSS_UNIT_PERCENT) {
                height = query->line_height.value;
            } else {
                height = query->line_height.computed;
            }
        }
        
        ds.lineHeight = height;
#if 0
        // Word spacing
        double wordSpacing;
        if (query->word_spacing.normal) wordSpacing = 0.0;
        else wordSpacing = query->word_spacing.computed; // Assume no units (change in desktop-style.cpp)
        
        GtkAction* wordSpacingAction = GTK_ACTION( g_object_get_data( tbl, "TextWordSpacingAction" ) );
        GtkAdjustment *wordSpacingAdjustment =
        ege_adjustment_action_get_adjustment(EGE_ADJUSTMENT_ACTION( wordSpacingAction ));
        gtk_adjustment_set_value( wordSpacingAdjustment, wordSpacing );
        
        
        // Letter spacing
        double letterSpacing;
        if (query->letter_spacing.normal) letterSpacing = 0.0;
        else letterSpacing = query->letter_spacing.computed; // Assume no units (change in desktop-style.cpp)
        
        GtkAction* letterSpacingAction = GTK_ACTION( g_object_get_data( tbl, "TextLetterSpacingAction" ) );
        GtkAdjustment *letterSpacingAdjustment =
        ege_adjustment_action_get_adjustment(EGE_ADJUSTMENT_ACTION( letterSpacingAction ));
        gtk_adjustment_set_value( letterSpacingAdjustment, letterSpacing );
        
        
        // Orientation
        int activeButton2 = (query->writing_mode.computed == SP_CSS_WRITING_MODE_LR_TB ? 0 : 1);
        
        EgeSelectOneAction* textOrientationAction =
        EGE_SELECT_ONE_ACTION( g_object_get_data( tbl, "TextOrientationAction" ) );
        ege_select_one_action_set_active( textOrientationAction, activeButton2 );
#endif
        
    } // if( query->text )
    
#ifdef DEBUG_TEXT
    std::cout << "    GUI: fontfamily.value: "
    << (query->text->font_family.value ? query->text->font_family.value : "No value")
    << std::endl;
    std::cout << "    GUI: font_size.computed: "   << query->font_size.computed   << std::endl;
    std::cout << "    GUI: font_weight.computed: " << query->font_weight.computed << std::endl;
    std::cout << "    GUI: font_style.computed: "  << query->font_style.computed  << std::endl;
    std::cout << "    GUI: text_anchor.computed: " << query->text_anchor.computed << std::endl;
    std::cout << "    GUI: text_align.computed:  " << query->text_align.computed  << std::endl;
    std::cout << "    GUI: line_height.computed: " << query->line_height.computed
    << "  line_height.value: "    << query->line_height.value
    << "  line_height.unit: "     << query->line_height.unit  << std::endl;
    std::cout << "    GUI: word_spacing.computed: " << query->word_spacing.computed
    << "  word_spacing.value: "    << query->word_spacing.value
    << "  word_spacing.unit: "     << query->word_spacing.unit  << std::endl;
    std::cout << "    GUI: letter_spacing.computed: " << query->letter_spacing.computed
    << "  letter_spacing.value: "    << query->letter_spacing.value
    << "  letter_spacing.unit: "     << query->letter_spacing.unit  << std::endl;
    std::cout << "    GUI: writing_mode.computed: " << query->writing_mode.computed << std::endl;
#endif
    
    sp_style_unref(query);
#if 0
    // Kerning (xshift), yshift, rotation.  NB: These are not CSS attributes.
    if( SP_IS_TEXT_CONTEXT((SP_ACTIVE_DESKTOP)->event_context) ) {
        SPTextContext *const tc = SP_TEXT_CONTEXT((SP_ACTIVE_DESKTOP)->event_context);
        if( tc ) {
            unsigned char_index = -1;
            TextTagAttributes *attributes =
            text_tag_attributes_at_position( tc->text, std::min(tc->text_sel_start, tc->text_sel_end), &char_index );
            if( attributes ) {
                
                // Dx
                double dx = attributes->getDx( char_index );
                GtkAction* dxAction = GTK_ACTION( g_object_get_data( tbl, "TextDxAction" ));
                GtkAdjustment *dxAdjustment =
                ege_adjustment_action_get_adjustment(EGE_ADJUSTMENT_ACTION( dxAction ));
                gtk_adjustment_set_value( dxAdjustment, dx );
                
                // Dy
                double dy = attributes->getDy( char_index );
                GtkAction* dyAction = GTK_ACTION( g_object_get_data( tbl, "TextDyAction" ));
                GtkAdjustment *dyAdjustment =
                ege_adjustment_action_get_adjustment(EGE_ADJUSTMENT_ACTION( dyAction ));
                gtk_adjustment_set_value( dyAdjustment, dy );
                
                // Rotation
                double rotation = attributes->getRotate( char_index );
                /* SVG value is between 0 and 360 but we're using -180 to 180 in widget */
                if( rotation > 180.0 ) rotation -= 360.0;
                GtkAction* rotationAction = GTK_ACTION( g_object_get_data( tbl, "TextRotationAction" ));
                GtkAdjustment *rotationAdjustment =
                ege_adjustment_action_get_adjustment(EGE_ADJUSTMENT_ACTION( rotationAction ));
                gtk_adjustment_set_value( rotationAdjustment, rotation );
                
#ifdef DEBUG_TEXT
                std::cout << "    GUI: Dx: " << dx << std::endl;
                std::cout << "    GUI: Dy: " << dy << std::endl;
                std::cout << "    GUI: Rotation: " << rotation << std::endl;
#endif
            }
        }
    }
    
    {
        // Set these here as we don't always have kerning/rotating attributes
        GtkAction* dxAction = GTK_ACTION( g_object_get_data( tbl, "TextDxAction" ));
        gtk_action_set_sensitive( GTK_ACTION(dxAction), !isFlow );
        
        GtkAction* dyAction = GTK_ACTION( g_object_get_data( tbl, "TextDyAction" ));
        gtk_action_set_sensitive( GTK_ACTION(dyAction), !isFlow );
        
        GtkAction* rotationAction = GTK_ACTION( g_object_get_data( tbl, "TextRotationAction" ));
        gtk_action_set_sensitive( GTK_ACTION(rotationAction), !isFlow );
    }
#endif
#ifdef DEBUG_TEXT
    std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;
    std::cout << std::endl;
#endif
    
    ds.freeze = FALSE;
    
}


// Font family
static void sp_text_fontfamily_value_changed( ZGDesktopStyle *tp, gchar *family )
{
#ifdef DEBUG_TEXT
    std::cout << std::endl;
    std::cout << "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM" << std::endl;
    std::cout << "sp_text_fontfamily_value_changed: " << std::endl;
#endif
    
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
#ifdef DEBUG_TEXT
    std::cout << "  New family: " << family << std::endl;
#endif
    
    // First try to get the old font spec from the stored value
    SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
    int result_fontspec = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONT_SPECIFICATION);
    
    Glib::ustring fontSpec = query->text->font_specification.set ?  query->text->font_specification.value : "";
    
    // If that didn't work, try to get font spec from style
    if (fontSpec.empty()) {
        
        // Must query all to fill font-family, font-style, font-weight, font-specification
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTFAMILY);
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTSTYLE);
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);
        
        // Construct a new font specification if it does not yet exist
        font_instance * fontFromStyle = font_factory::Default()->FaceFromStyle(query);
        if( fontFromStyle ) {
            fontSpec = font_factory::Default()->ConstructFontSpecification(fontFromStyle);
            fontFromStyle->Unref();
        }
#ifdef DEBUG_TEXT
        std::cout << "  Fontspec not defined, reconstructed from style :" << fontSpec << ":" << std::endl;
        sp_print_font( query );
#endif
    }
    
    // And if that didn't work use default
    if( fontSpec.empty() ) {
        sp_style_read_from_prefs(query, "/tools/text");
#ifdef DEBUG_TEXT
        std::cout << "    read style from prefs:" << std::endl;
        sp_print_font( query );
#endif
        // Construct a new font specification if it does not yet exist
        font_instance * fontFromStyle = font_factory::Default()->FaceFromStyle(query);
        if( fontFromStyle ) {
            fontSpec = font_factory::Default()->ConstructFontSpecification(fontFromStyle);
            fontFromStyle->Unref();
        }
#ifdef DEBUG_TEXT
        std::cout << "  Fontspec not defined, reconstructed from style :" << fontSpec << ":" << std::endl;
        sp_print_font( query );
#endif
    }
    
    SPCSSAttr *css = sp_repr_css_attr_new ();
    if (!fontSpec.empty()) {
        
        // Now we have a font specification, replace family.
        Glib::ustring  newFontSpec = font_factory::Default()->ReplaceFontSpecificationFamily(fontSpec, family);
        
#ifdef DEBUG_TEXT
        std::cout << "  New FontSpec from ReplaceFontSpecificationFamily :" << newFontSpec << ":" << std::endl;
#endif
        
        if (!newFontSpec.empty()) {
            
            if (fontSpec != newFontSpec) {
                
                font_instance *font = font_factory::Default()->FaceFromFontSpecification(newFontSpec.c_str());
                
                if (font) {
                    sp_repr_css_set_property (css, "-inkscape-font-specification", newFontSpec.c_str());
                    
                    // Set all the these just in case they were altered when finding the best
                    // match for the new family and old style...
                    
                    gchar c[256];
                    
                    font->Family(c, 256);
                    
                    sp_repr_css_set_property (css, "font-family", c);
                    
                    font->Attribute( "weight", c, 256);
                    sp_repr_css_set_property (css, "font-weight", c);
                    
                    font->Attribute("style", c, 256);
                    sp_repr_css_set_property (css, "font-style", c);
                    
                    font->Attribute("stretch", c, 256);
                    sp_repr_css_set_property (css, "font-stretch", c);
                    
                    font->Attribute("variant", c, 256);
                    sp_repr_css_set_property (css, "font-variant", c);
                    
                    font->Unref();
                }
            }
            
        } else {
            
            // newFontSpec empty
            // If the old font on selection (or default) does not exist on the system,
            // or the new font family does not exist,
            // ReplaceFontSpecificationFamily does not work. In that case we fall back to blindly
            // setting the family reported by the family chooser.
            
            // g_print ("fallback setting family: %s\n", family);
            sp_repr_css_set_property (css, "-inkscape-font-specification", family);
            sp_repr_css_set_property (css, "font-family", family);
            // Shoud we set other css font attributes?
        }
        
    }  // fontSpec not empty or not
    
    // If querying returned nothing, update default style.
    if (result_fontspec == QUERY_STYLE_NOTHING)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->mergeStyle("/tools/text/style", css);
        //        sp_text_edit_dialog_default_set_insensitive (); //FIXME: Replace through a verb   //zhangguangjian
    }
    else
    {
        sp_desktop_set_style (SP_ACTIVE_DESKTOP, css, true, true);
    }
    
    sp_style_unref(query);
    
    //    g_free (family);
    
    // Save for undo
    if (result_fontspec != QUERY_STYLE_NOTHING) {
        sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP), SP_VERB_CONTEXT_TEXT,
                          _("Text: Change font family"));
    }
    sp_repr_css_attr_unref (css);
    
    // unfreeze
    tp.freeze = FALSE;
    
    // focus to canvas
    //    gtk_widget_grab_focus (SP_GTK_CANVAS_WIDGET((SP_ACTIVE_DESKTOP)->canvas));    //zhangguangjian
    
#ifdef DEBUG_TEXT
    std::cout << "sp_text_toolbox_fontfamily_changes: exit"  << std::endl;
    std::cout << "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM" << std::endl;
    std::cout << std::endl;
#endif
}

// Handles both Bold and Italic/Oblique
static void sp_text_style_changed( ZGDesktopStyle *tp, gboolean bold, gboolean italic  )
{
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
    // First query font-specification, this is the most complete font face description.
    SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
    int result_fontspec = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONT_SPECIFICATION);
    
    // font_specification will not be set unless defined explicitely on a tspan.
    // This should be fixed!
    Glib::ustring fontSpec = query->text->font_specification.set ?  query->text->font_specification.value : "";
    
    if (fontSpec.empty()) {
        // Construct a new font specification if it does not yet exist
        // Must query font-family, font-style, font-weight, to find correct font face.
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTFAMILY);
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTSTYLE);
        sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);
        
        font_instance * fontFromStyle = font_factory::Default()->FaceFromStyle(query);
        if( fontFromStyle ) {
            fontSpec = font_factory::Default()->ConstructFontSpecification(fontFromStyle);
            fontFromStyle->Unref();
        }
    }
    
    // Now that we have the old face, find the new face.
    Glib::ustring newFontSpec = "";
    SPCSSAttr   *css        = sp_repr_css_attr_new ();
    gboolean nochange = true;
    
    if (!fontSpec.empty()) {
        // Bold
        newFontSpec = font_factory::Default()->FontSpecificationSetBold(fontSpec, bold);
        
        if (!newFontSpec.empty()) {
            
            // Set weight if we found font.
            font_instance * font = font_factory::Default()->FaceFromFontSpecification(newFontSpec.c_str());
            if (font) {
                gchar c[256];
                font->Attribute( "weight", c, 256);
                sp_repr_css_set_property (css, "font-weight", c);
                font->Unref();
                font = NULL;
            }
            nochange = false;
        }
        
        //italic
        newFontSpec = font_factory::Default()->FontSpecificationSetItalic(fontSpec, italic);
        
        if (!newFontSpec.empty()) {
            
            // Don't even set the italic/oblique if the font didn't exist on the system
            if( italic ) {
                if( newFontSpec.find( "Italic" ) != Glib::ustring::npos ) {
                    sp_repr_css_set_property (css, "font-style", "italic");
                } else {
                    sp_repr_css_set_property (css, "font-style", "oblique");
                }
            } else {
                sp_repr_css_set_property (css, "font-style", "normal");
            }
            nochange = false;
        }
    }
    
    if (!newFontSpec.empty()) {
        sp_repr_css_set_property (css, "-inkscape-font-specification", newFontSpec.c_str());
    }
    
    // If querying returned nothing, update default style.
    if (result_fontspec == QUERY_STYLE_NOTHING)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->mergeStyle("/tools/text/style", css);
    }
    
    sp_style_unref(query);
    
    // Do we need to update other CSS values?
    SPDesktop   *desktop    = SP_ACTIVE_DESKTOP;
    sp_desktop_set_style (desktop, css, true, true);
    if (result_fontspec != QUERY_STYLE_NOTHING) {
        sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP), SP_VERB_CONTEXT_TEXT,
                          _("Text: Change font style"));
    }
    sp_repr_css_attr_unref (css);
    
    tp.freeze = FALSE;
}

// Handles both Superscripts and Subscripts
// prop : Called by Superscript or Subscript button?
static void sp_text_script_changed( ZGDesktopStyle *tp, bool prop, int new_val )
{
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
    
#ifdef DEBUG_TEXT
    std::cout << "sp_text_script_changed: " << prop << std::endl;
#endif
    
    // Query baseline
    SPStyle *query = sp_style_new (sp_desktop_document(tp.desktop));
    int result_baseline = sp_desktop_query_style (tp.desktop, query, QUERY_STYLE_PROPERTY_BASELINES);
    
    bool setSuper = false;
    bool setSub   = false;
    
    if(result_baseline == QUERY_STYLE_NOTHING || result_baseline == QUERY_STYLE_MULTIPLE_DIFFERENT ) {
        // If not set or mixed, turn on superscript or subscript
        if( prop == 0 ) {
            setSuper = true;
        } else {
            setSub = true;
        }
    } else {
        // Superscript
        gboolean superscriptSet = (query->baseline_shift.set &&
                                   query->baseline_shift.type == SP_BASELINE_SHIFT_LITERAL &&
                                   query->baseline_shift.literal == SP_CSS_BASELINE_SHIFT_SUPER );
        
        // Subscript
        gboolean subscriptSet = (query->baseline_shift.set &&
                                 query->baseline_shift.type == SP_BASELINE_SHIFT_LITERAL &&
                                 query->baseline_shift.literal == SP_CSS_BASELINE_SHIFT_SUB );
        
        setSuper = !superscriptSet && prop == 0;
        setSub   = !subscriptSet   && prop == 1;
    }
    
    // Set css properties
    SPCSSAttr *css = sp_repr_css_attr_new ();
    if( setSuper || setSub ) {
        // Openoffice 2.3 and Adobe use 58%, Microsoft Word 2002 uses 65%, LaTex about 70%.
        // 58% looks too small to me, especially if a superscript is placed on a superscript.
        // If you make a change here, consider making a change to baseline-shift amount
        // in style.cpp.
        sp_repr_css_set_property (css, "font-size", "65%");
    } else {
        sp_repr_css_set_property (css, "font-size", "");
    }
    if( setSuper ) {
        sp_repr_css_set_property (css, "baseline-shift", "super");
    } else if( setSub ) {
        sp_repr_css_set_property (css, "baseline-shift", "sub");
    } else {
        sp_repr_css_set_property (css, "baseline-shift", "baseline");
    }
    
    // Apply css to selected objects.
    SPDesktop *desktop = tp.desktop;
    sp_desktop_set_style (desktop, css, true, false);
    
    // Save for undo
    if(result_baseline != QUERY_STYLE_NOTHING) {
        sp_document_maybe_done (sp_desktop_document (tp.desktop), "ttb:script", SP_VERB_NONE,
                                _("Text: Change superscript or subscript"));
    }
    tp.freeze = FALSE;
}

static void sp_text_align_mode_changed( ZGDesktopStyle *tp, int/*NSTextAlignment*/ mode )
{
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/text/align_mode", mode);
    
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    
    // move the x of all texts to preserve the same bbox
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    for (GSList const *items = selection->itemList(); items != NULL; items = items->next) {
        if (SP_IS_TEXT((SPItem *) items->data)) {
            SPItem *item = SP_ITEM(items->data);
            
            unsigned writing_mode = SP_OBJECT_STYLE(item)->writing_mode.value;
            // below, variable names suggest horizontal move, but we check the writing direction
            // and move in the corresponding axis
            int axis;
            if (writing_mode == SP_CSS_WRITING_MODE_LR_TB || writing_mode == SP_CSS_WRITING_MODE_RL_TB) {
                axis = NR::X;
            } else {
                axis = NR::Y;
            }
            
            Geom::OptRect bbox
            = item->getBounds(Geom::identity(), SPItem::GEOMETRIC_BBOX);
            if (!bbox)
                continue;
            double width = bbox->dimensions()[axis];
            // If you want to align within some frame, other than the text's own bbox, calculate
            // the left and right (or top and bottom for tb text) slacks of the text inside that
            // frame (currently unused)
            double left_slack = 0;
            double right_slack = 0;
            unsigned old_align = SP_OBJECT_STYLE(item)->text_align.value;
            double move = 0;
            if (old_align == SP_CSS_TEXT_ALIGN_START || old_align == SP_CSS_TEXT_ALIGN_LEFT) {
                switch (mode) {
                    case NSLeftTextAlignment:
                        move = -left_slack;
                        break;
                    case NSCenterTextAlignment:
                        move = width/2 + (right_slack - left_slack)/2;
                        break;
                    case NSRightTextAlignment:
                        move = width + right_slack;
                        break;
                }
            } else if (old_align == SP_CSS_TEXT_ALIGN_CENTER) {
                switch (mode) {
                    case NSLeftTextAlignment:
                        move = -width/2 - left_slack;
                        break;
                    case NSCenterTextAlignment:
                        move = (right_slack - left_slack)/2;
                        break;
                    case NSRightTextAlignment:
                        move = width/2 + right_slack;
                        break;
                }
            } else if (old_align == SP_CSS_TEXT_ALIGN_END || old_align == SP_CSS_TEXT_ALIGN_RIGHT) {
                switch (mode) {
                    case NSLeftTextAlignment:
                        move = -width - left_slack;
                        break;
                    case NSCenterTextAlignment:
                        move = -width/2 + (right_slack - left_slack)/2;
                        break;
                    case NSRightTextAlignment:
                        move = right_slack;
                        break;
                }
            }
            Geom::Point XY = SP_TEXT(item)->attributes.firstXY();
            if (axis == NR::X) {
                XY = XY + Geom::Point (move, 0);
            } else {
                XY = XY + Geom::Point (0, move);
            }
            SP_TEXT(item)->attributes.setFirstXY(XY);
            SP_OBJECT(item)->updateRepr();
            SP_OBJECT(item)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
    }
    
    SPCSSAttr *css = sp_repr_css_attr_new ();
    switch (mode)
    {
        case NSLeftTextAlignment:
        {
            sp_repr_css_set_property (css, "text-anchor", "start");
            sp_repr_css_set_property (css, "text-align", "start");
            break;
        }
        case NSCenterTextAlignment:
        {
            sp_repr_css_set_property (css, "text-anchor", "middle");
            sp_repr_css_set_property (css, "text-align", "center");
            break;
        }
            
        case NSRightTextAlignment:
        {
            sp_repr_css_set_property (css, "text-anchor", "end");
            sp_repr_css_set_property (css, "text-align", "end");
            break;
        }
            
        case NSJustifiedTextAlignment:
        {
            sp_repr_css_set_property (css, "text-anchor", "start");
            sp_repr_css_set_property (css, "text-align", "justify");
            break;
        }
    }
    
    SPStyle *query =
    sp_style_new (SP_ACTIVE_DOCUMENT);
    int result_numbers =
    sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);
    
    // If querying returned nothing, update default style.
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->mergeStyle("/tools/text/style", css);
    }
    
    sp_style_unref(query);
    
    sp_desktop_set_style (desktop, css, true, true);
    if (result_numbers != QUERY_STYLE_NOTHING)
    {
        sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP), SP_VERB_CONTEXT_TEXT,
                          _("Text: Change alignment"));
    }
    sp_repr_css_attr_unref (css);
    
    //    gtk_widget_grab_focus (SP_GTK_CANVAS_WIDGET(desktop->canvas));    //zhangguangjian
    
    tp.freeze = FALSE;
}

static void sp_text_lineheight_value_changed( ZGDesktopStyle *tp, CGFloat newHeight)
{
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
    // At the moment this handles only numerical values (i.e. no percent).
    // Set css line height.
    SPCSSAttr *css = sp_repr_css_attr_new ();
    Inkscape::CSSOStringStream osfs;
    osfs << newHeight *100 << "%";
    sp_repr_css_set_property (css, "line-height", osfs.str().c_str());
    
    // Apply line-height to selected objects.
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    sp_desktop_set_style (desktop, css, true, false);
    
    
    // Until deprecated sodipodi:linespacing purged:
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList const *items = selection->itemList();
    bool modmade = false;
    for (; items != NULL; items = items->next) {
        if (SP_IS_TEXT (items->data)) {
            SP_OBJECT_REPR(items->data)->setAttribute("sodipodi:linespacing", sp_repr_css_property (css, "line-height", NULL));
            modmade = true;
        }
    }
    
    // Save for undo
    if(modmade) {
        sp_document_maybe_done (sp_desktop_document (SP_ACTIVE_DESKTOP), "ttb:line-height", SP_VERB_NONE,
                                _("Text: Change line-height"));
    }
    
    // If no selected objects, set default.
    SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
    int result_numbers =
    sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->mergeStyle("/tools/text/style", css);
    }
    sp_style_unref(query);
    
    sp_repr_css_attr_unref (css);
    
    tp.freeze = FALSE;
}

static void sp_text_wordspacing_value_changed( ZGDesktopStyle *tp, double new_val )
{
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
    // At the moment this handles only numerical values (i.e. no em unit).
    // Set css word-spacing
    SPCSSAttr *css = sp_repr_css_attr_new ();
    Inkscape::CSSOStringStream osfs;
    osfs << new_val << "px"; // For now always use px
    sp_repr_css_set_property (css, "word-spacing", osfs.str().c_str());
    
    // Apply word-spacing to selected objects.
    SPDesktop *desktop = tp.desktop;
    sp_desktop_set_style (desktop, css, true, false);
    
    // If no selected objects, set default.
    SPStyle *query = sp_style_new (sp_desktop_document(tp.desktop));
    int result_numbers =
    sp_desktop_query_style (tp.desktop, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->mergeStyle("/tools/text/style", css);
    } else {
        // Save for undo
        sp_document_maybe_done (sp_desktop_document (tp.desktop), "ttb:word-spacing", SP_VERB_NONE,
                                _("Text: Change word-spacing"));
    }
    sp_style_unref(query);
    
    sp_repr_css_attr_unref (css);
    
    tp.freeze = FALSE;
}

static void sp_text_letterspacing_value_changed( ZGDesktopStyle *tp, double new_val )
{
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
    // At the moment this handles only numerical values (i.e. no em unit).
    // Set css letter-spacing
    SPCSSAttr *css = sp_repr_css_attr_new ();
    Inkscape::CSSOStringStream osfs;
    osfs << new_val << "px";  // For now always use px
    sp_repr_css_set_property (css, "letter-spacing", osfs.str().c_str());
    
    // Apply letter-spacing to selected objects.
    SPDesktop *desktop = tp.desktop;
    sp_desktop_set_style (desktop, css, true, false);
    
    
    // If no selected objects, set default.
    SPStyle *query = sp_style_new (sp_desktop_document(tp.desktop));
    int result_numbers =
    sp_desktop_query_style (tp.desktop, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->mergeStyle("/tools/text/style", css);
    }
    else
    {
        // Save for undo
        sp_document_maybe_done (sp_desktop_document (tp.desktop), "ttb:letter-spacing", SP_VERB_NONE,
                                _("Text: Change letter-spacing"));
    }
    
    sp_style_unref(query);
    
    sp_repr_css_attr_unref (css);
    
    tp.freeze = FALSE;
}


static void sp_text_dx_value_changed( ZGDesktopStyle *tp, double new_val )
{
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
    gdouble new_dx = new_val;
    bool modmade = false;
    
    if( SP_IS_TEXT_CONTEXT((tp.desktop)->event_context) ) {
        SPTextContext *const tc = SP_TEXT_CONTEXT((tp.desktop)->event_context);
        if( tc ) {
            unsigned char_index = -1;
            TextTagAttributes *attributes =
            text_tag_attributes_at_position( tc->text, std::min(tc->text_sel_start, tc->text_sel_end), &char_index );
            if( attributes ) {
                double old_dx = attributes->getDx( char_index );
                double delta_dx = new_dx - old_dx;
                sp_te_adjust_dx( tc->text, tc->text_sel_start, tc->text_sel_end, tp.desktop, delta_dx );
                modmade = true;
            }
        }
    }
    
    if(modmade) {
        // Save for undo
        sp_document_maybe_done (sp_desktop_document (tp.desktop), "ttb:dx", SP_VERB_NONE,
                                _("Text: Change dx (kern)"));
    }
    tp.freeze = FALSE;
}

static void sp_text_dy_value_changed( ZGDesktopStyle *tp, double new_val )
{
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
    gdouble new_dy = new_val;
    bool modmade = false;
    
    if( SP_IS_TEXT_CONTEXT((tp.desktop)->event_context) ) {
        SPTextContext *const tc = SP_TEXT_CONTEXT((tp.desktop)->event_context);
        if( tc ) {
            unsigned char_index = -1;
            TextTagAttributes *attributes =
            text_tag_attributes_at_position( tc->text, std::min(tc->text_sel_start, tc->text_sel_end), &char_index );
            if( attributes ) {
                double old_dy = attributes->getDy( char_index );
                double delta_dy = new_dy - old_dy;
                sp_te_adjust_dy( tc->text, tc->text_sel_start, tc->text_sel_end, tp.desktop, delta_dy );
                modmade = true;
            }
        }
    }
    
    if(modmade) {
        // Save for undo
        sp_document_maybe_done (sp_desktop_document (tp.desktop), "ttb:dy", SP_VERB_NONE,
                                _("Text: Change dy"));
    }
    
    tp.freeze = FALSE;
}

static void sp_text_rotation_value_changed( ZGDesktopStyle *tp, double new_val )
{
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
    gdouble new_degrees = new_val;
    
    bool modmade = false;
    if( SP_IS_TEXT_CONTEXT((tp.desktop)->event_context) ) {
        SPTextContext *const tc = SP_TEXT_CONTEXT((tp.desktop)->event_context);
        if( tc ) {
            unsigned char_index = -1;
            TextTagAttributes *attributes =
            text_tag_attributes_at_position( tc->text, std::min(tc->text_sel_start, tc->text_sel_end), &char_index );
            if( attributes ) {
                double old_degrees = attributes->getRotate( char_index );
                double delta_deg = new_degrees - old_degrees;
                sp_te_adjust_rotation( tc->text, tc->text_sel_start, tc->text_sel_end, SP_ACTIVE_DESKTOP, delta_deg );
                modmade = true;
            }
        }
    }
    
    // Save for undo
    if(modmade) {
        sp_document_maybe_done (sp_desktop_document (tp.desktop), "ttb:rotate", SP_VERB_NONE,
                                _("Text: Change rotate"));
    }
    
    tp.freeze = FALSE;
}

static void sp_text_orientation_mode_changed( ZGDesktopStyle *tp, int mode )
{
    // quit if run by the _changed callbacks
    if (tp.freeze) {
        return;
    }
    tp.freeze = TRUE;
    
    SPCSSAttr   *css        = sp_repr_css_attr_new ();
    switch (mode)
    {
        case 0:
        {
            sp_repr_css_set_property (css, "writing-mode", "lr");
            break;
        }
            
        case 1:
        {
            sp_repr_css_set_property (css, "writing-mode", "tb");
            break;
        }
    }
    
    SPStyle *query =
    sp_style_new (sp_desktop_document(tp.desktop));
    int result_numbers =
    sp_desktop_query_style (tp.desktop, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);
    
    // If querying returned nothing, update default style.
    if (result_numbers == QUERY_STYLE_NOTHING)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->mergeStyle("/tools/text/style", css);
    }
    
    sp_desktop_set_style (tp.desktop, css, true, true);
    if(result_numbers != QUERY_STYLE_NOTHING)
    {
        sp_document_done (sp_desktop_document (tp.desktop), SP_VERB_CONTEXT_TEXT,
                          _("Text: Change orientation"));
    }
    sp_repr_css_attr_unref (css);
    
    tp.freeze = FALSE;
}

