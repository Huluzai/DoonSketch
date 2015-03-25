//
//  PaintSelector.m
//  Inxcape
//
//  Created by 张 光建 on 14-10-11.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#include "PaintSelector.h"
#include "NSColor+SPColor.h"
#include "GradientSelector.h"
#include "NSView+Extension.h"

#include <cstring>
#include <string>

#include <gtk/gtk.h>

#include <glibmm/i18n.h>
#include <xml/repr.h>

#include <sp-linear-gradient-fns.h>
#include <sp-radial-gradient-fns.h>
#include <inkscape.h>
#include <document-private.h>
#include <desktop-style.h>
#include <style.h>
#include <svg/svg-color.h>
#include <svg/css-ostringstream.h>
#include <path-prefix.h>
#include <io/sys.h>
#include <helper/stock-items.h>
#include <sp-gradient.h>
#include <sp-pattern.h>
#include <gc.h>

#ifdef SP_PS_VERBOSE
static gchar const* modeStrings[] = {
    "MODE_EMPTY",
    "MODE_MULTIPLE",
    "MODE_NONE",
    "MODE_COLOR_RGB",
    "MODE_COLOR_CMYK",
    "MODE_GRADIENT_LINEAR",
    "MODE_GRADIENT_RADIAL",
    "MODE_PATTERN",
    "MODE_SWATCH",
    "MODE_UNSET",
    ".",
    ".",
    ".",
};
#endif

NSString *kPaintSelectorChangedFillRuleNotification = @"PaintSelectorChangedFillRuleNotification";

static bool isPaintModeGradient( SPPaintSelector::Mode mode )
{
    bool isGrad = (mode == SPPaintSelector::MODE_GRADIENT_LINEAR) ||
    (mode == SPPaintSelector::MODE_GRADIENT_RADIAL) ||
    (mode == SPPaintSelector::MODE_SWATCH);
    
    return isGrad;
}

@interface PaintSelector () {
    BOOL update;
    NSColor *_uiColor;
    SPPaintSelector::Mode _mode;
}

@property GradientSelector *gsel;

- (void)setModeEmpty;
- (void)setModeMultiple;
- (void)setModeNone;
- (void)setModeColor:(SPPaintSelector::Mode)mode;
- (void)setModeGradient:(SPPaintSelector::Mode)mode;
- (void)setModePattern:(SPPaintSelector::Mode)mode;
- (void)setModeSwatch:(SPPaintSelector::Mode)mode;
- (void)setModeUnset;

@end


@implementation PaintSelector

- (id)initWithNibName:(NSString *)nibNameOrNill bundle:(NSBundle *)nibBundleOrNil
{
    if (self = [super initWithNibName:nibNameOrNill bundle:nibBundleOrNil]) {
        self.gsel = [[GradientSelector alloc] initWithNibName:@"GradientSelector" bundle:nill];
        self.gsel.mode = SPGradientSelector::MODE_LINEAR;
        self.gsel.delegate = self;
        self.uiColor = [NSColor blackColor];
    }
    return self;
}

- (void)dealloc
{
    self.gsel = nill;
}

- (void)awakeFromNib
{
    self.segmentModes.enabled = FALSE;
    self.checkFillRule.enabled = FALSE;
    [self.linearContentView addSubview:self.gsel.view];
}

- (SPPaintSelector::Mode)modeForSegment:(NSInteger)segment
{
    SPPaintSelector::Mode r = SPPaintSelector::MODE_EMPTY;
    
    switch (segment) {
        case 0:
            r = (SPPaintSelector::Mode)2;
            break;
        case 1:
            r = (SPPaintSelector::Mode)3;
            break;
        case 2:
            r = (SPPaintSelector::Mode)5;
            break;
        case 3:
            r = (SPPaintSelector::Mode)6;
        default:
            break;
    }
    return r;
}

- (BOOL)isSupportedMode:(SPPaintSelector::Mode)mode
{
    int m = mode;
    return m == 2 || m == 3 || m == 5 || m == 6;
}

- (IBAction)didChangeModes:(id)sender
{
    self.mode = [self modeForSegment:[self.segmentModes selectedSegment]];
}

+ (SPPaintSelector::Mode)getModeForStyle:(SPStyle const &)style fillOrStroke:(FillOrStroke)kind
{
    SPPaintSelector::Mode mode = SPPaintSelector::MODE_UNSET;
    SPIPaint const & target = (kind == FILL) ? style.fill : style.stroke;
    
    if ( !target.set ) {
        mode = SPPaintSelector::MODE_UNSET;
    } else if ( target.isPaintserver() ) {
        SPPaintServer const *server = (kind == FILL) ? style.getFillPaintServer() : style.getStrokePaintServer();
        
#ifdef SP_PS_VERBOSE
        g_message("SPPaintSelector::getModeForStyle(%p, %d)", &style, kind);
        g_message("==== server:%p %s  grad:%s   swatch:%s", server, server->getId(), (SP_IS_GRADIENT(server)?"Y":"n"), (SP_IS_GRADIENT(server) && SP_GRADIENT(server)->getVector()->isSwatch()?"Y":"n"));
#endif // SP_PS_VERBOSE
        
        
        if (server && SP_IS_GRADIENT(server) && SP_GRADIENT(server)->getVector()->isSwatch()) {
            mode = SPPaintSelector::MODE_SWATCH;
        } else if (SP_IS_LINEARGRADIENT(server)) {
            mode = SPPaintSelector::MODE_GRADIENT_LINEAR;
        } else if (SP_IS_RADIALGRADIENT(server)) {
            mode = SPPaintSelector::MODE_GRADIENT_RADIAL;
        } else if (SP_IS_PATTERN(server)) {
            mode = SPPaintSelector::MODE_PATTERN;
        } else {
            g_warning( "file %s: line %d: Unknown paintserver", __FILE__, __LINE__ );
            mode = SPPaintSelector::MODE_NONE;
        }
    } else if ( target.isColor() ) {
        // TODO this is no longer a valid assertion:
        mode = SPPaintSelector::MODE_COLOR_RGB; // so far only rgb can be read from svg
    } else if ( target.isNone() ) {
        mode = SPPaintSelector::MODE_NONE;
    } else {
        g_warning( "file %s: line %d: Unknown paint type", __FILE__, __LINE__ );
        mode = SPPaintSelector::MODE_NONE;
    }
    
    return mode;
}

- (void)setUiColor:(NSColor *)uiColor
{
    _uiColor = uiColor;
    if ([self.delegate respondsToSelector:@selector(didPaintSelectorChanged:)]) {
        [self.delegate didPaintSelectorChanged:self];
    }
}

- (NSColor *)uiColor
{
    return _uiColor;
}

- (void)setFillRule:(SPPaintSelector::FillRule)fillRule
{
    if (self.fillRule != fillRule) {
        self.checkFillRule.state = (fillRule == SPPaintSelector::FILLRULE_EVENODD) ? NSOnState : NSOffState;
    }
}

- (SPPaintSelector::FillRule)fillRule
{
    return (self.checkFillRule.state == NSOnState) ? SPPaintSelector::FILLRULE_EVENODD : SPPaintSelector::FILLRULE_NONZERO;
}

- (SPPaintSelector::Mode)mode
{
    return _mode;
}

- (void)setMode:(SPPaintSelector::Mode)mode
{
    if (_mode != mode) {
        update = TRUE;
#ifdef SP_PS_VERBOSE
        g_print("Mode change %d -> %d   %s -> %s\n", _mode, mode, modeStrings[_mode], modeStrings[mode]);
#endif
        switch (mode) {
            case SPPaintSelector::MODE_EMPTY:
                [self setModeEmpty];
                break;
            case SPPaintSelector::MODE_MULTIPLE:
                [self setModeMultiple];
                break;
            case SPPaintSelector::MODE_NONE:
                [self setModeNone];
                break;
            case SPPaintSelector::MODE_COLOR_RGB:
            case SPPaintSelector::MODE_COLOR_CMYK:
                [self setModeColor:mode];
                break;
            case SPPaintSelector::MODE_GRADIENT_LINEAR:
            case SPPaintSelector::MODE_GRADIENT_RADIAL:
                [self setModeGradient:mode];
                break;
            case SPPaintSelector::MODE_PATTERN:
                [self setModePattern:mode];
                break;
            case SPPaintSelector::MODE_SWATCH:
                [self setModeSwatch:mode];
                break;
            case SPPaintSelector::MODE_UNSET:
                [self setModeUnset];
                break;
            default:
                g_warning("file %s: line %d: Unknown paint mode %d", __FILE__, __LINE__, mode);
                break;
        }
        
        _mode = mode;
        if ([self isSupportedMode:_mode]) {
            [self.segmentModes selectSegmentWithTag:_mode];
        } else {
            for (int  i = 0; i < self.segmentModes.segmentCount; i++) {
                [self.segmentModes setSelected:FALSE forSegment:i];
            }
        }
            
        if ([self.delegate respondsToSelector:@selector(didPaintSelector:changeMode:)]) {
            [self.delegate didPaintSelector:self changeMode:mode];
        }
        
        update = FALSE;
    }
}

- (void)setColor:(SPColor const &)color alpha:(float)alpha
{
    self.uiColor = [NSColor colorWithSPColor:&color alpha:alpha];
}

- (void)getColor:(SPColor &)color alpha:(gfloat &)alpha
{
    color = *[self.uiColor spColor];
    alpha = [self.uiColor alphaComponent];
}

- (void)setGradientLinear:(SPGradient *)vector
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set GRADIENT RADIAL\n");
#endif
    self.mode = SPPaintSelector::MODE_GRADIENT_LINEAR;
    self.gsel.mode = SPGradientSelector::MODE_LINEAR;
    [self.gsel setVector:vector toDocument:SP_OBJECT_DOCUMENT(vector)];
}

- (void)setGradientRadial:(SPGradient *)vector
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set GRADIENT RADIAL\n");
#endif
    self.mode = SPPaintSelector::MODE_GRADIENT_RADIAL;
    self.gsel.mode = SPGradientSelector::MODE_RADIAL;
    [self.gsel setVector:vector toDocument:SP_OBJECT_DOCUMENT(vector)];
}

- (void)setSwatch:(SPGradient *)vector
{
    
}

- (void)setGradientProperties:(SPGradientUnits)units spread:(SPGradientSpread)spread
{
    g_return_if_fail(isPaintModeGradient(self.mode));
    
    [self.gsel setUnits:units];
    [self.gsel setSpread:spread];
}

- (void)getGradientProperties:(SPGradientUnits &)units spread:(SPGradientSpread &)spread
{
    g_return_if_fail(isPaintModeGradient(self.mode));
    
    units = self.gsel.units;
    spread = self.gsel.spread;
}

- (void)pushAttrsToGradient:(SPGradient *)gr
{
    SPGradientUnits units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
    SPGradientSpread spread = SP_GRADIENT_SPREAD_PAD;
    [self getGradientProperties:units spread:spread];
    gr->setUnits(units);
    gr->setSpread(spread);
    SP_OBJECT(gr)->updateRepr();
}

- (SPGradient *)getGradientVector
{
    return [self.gsel vector];
}

- (SPPattern *)getPattern
{
    return NULL;
}

- (void)updatePatternList:(SPPattern *)pat
{
    
}

// TODO move this elsewhere:
- (void)setFlatColor:(SPDesktop *)desktop colorProperty:(const gchar *)colorProperty opacity:(const gchar *)opacityProperty
{
    SPCSSAttr *css = sp_repr_css_attr_new();
    
    SPColor *clr = [self.uiColor spColor];
    gfloat alpha = [[self.uiColor colorUsingColorSpaceName:NSCalibratedRGBColorSpace] alphaComponent];
    
    std::string colorStr = clr->toString();
    
    delete clr;
    clr = NULL;
    
#ifdef SP_PS_VERBOSE
    guint32 rgba = color.toRGBA32( alpha );
    g_message("sp_paint_selector_set_flat_color() to '%s' from 0x%08x::%s",
              colorStr.c_str(),
              rgba,
              (color.icc ? color.icc->colorProfile.c_str():"<null>") );
#endif // SP_PS_VERBOSE
    
    sp_repr_css_set_property(css, colorProperty, colorStr.c_str());
    Inkscape::CSSOStringStream osalpha;
    osalpha << alpha;
    sp_repr_css_set_property(css, opacityProperty, osalpha.str().c_str());
    
    sp_desktop_set_style(desktop, css);
    
    sp_repr_css_attr_unref(css);
}

- (void)setModeEmpty
{
    self.segmentModes.enabled = FALSE;
    self.checkFillRule.enabled = FALSE;
    [self.modePlace setSubviews:[NSArray arrayWithObject:self.modeEmptyPage]];
}

- (void)setModeMultiple
{
    self.segmentModes.enabled = TRUE;
    [self.modePlace setSubviews:[NSArray arrayWithObject:self.modeMultiplePage]];
}

- (void)setModeNone
{
    self.segmentModes.enabled = TRUE;
    self.checkFillRule.enabled = TRUE;
    [self.modePlace setSubviews:[NSArray array]];
    [self.modePlace addSubview:self.modeNonePage];
}

- (void)setModeColor:(SPPaintSelector::Mode)mode
{
    self.segmentModes.enabled = TRUE;
    self.checkFillRule.enabled = TRUE;
    [self.modePlace setSubviews:[NSArray array]];
    [self.modePlace addSubview:self.modeColorPage];
}

- (void)setModeGradient:(SPPaintSelector::Mode)mode
{
    self.segmentModes.enabled = TRUE;
    self.checkFillRule.enabled = TRUE;
    [self.modePlace setSubviews:[NSArray array]];
    NSView *v = mode == SPPaintSelector::MODE_GRADIENT_LINEAR ? self.modeLinearPage : self.modeRadialPage;
    [self.gsel.view removeFromSuperview];
    [v addSubview:self.gsel.view];
    [self.modePlace addSubview:v];
}

- (void)setModePattern:(SPPaintSelector::Mode)mode
{
    self.segmentModes.enabled = TRUE;
    self.checkFillRule.enabled = TRUE;
    [self.modePlace setSubviews:[NSArray array]];
    [self.modePlace addSubview:self.modePatternPage];
}

- (void)setModeSwatch:(SPPaintSelector::Mode)mode
{
    self.segmentModes.enabled = TRUE;
    self.checkFillRule.enabled = TRUE;
    [self.modePlace setSubviews:[NSArray array]];
    [self.modePlace addSubview:self.modeSwatchPage];
}

- (void)setModeUnset
{
    self.segmentModes.enabled = TRUE;
    self.checkFillRule.enabled = TRUE;
    [self.modePlace setSubviews:[NSArray array]];
    [self.modePlace addSubview:self.modeUnsetPage];
}

- (void)didGradientSelectorChanged:(GradientSelector *)gsel
{
    if ([self.delegate respondsToSelector:@selector(didPaintSelectorChanged:)]) {
        [self.delegate didPaintSelectorChanged:self];
    }
}

- (IBAction)didChangeFillRule:(id)sender
{
    self.fillRule = (self.checkFillRule.state == NSOffState) ? SPPaintSelector::FILLRULE_NONZERO : SPPaintSelector::FILLRULE_EVENODD;
    [defaultNfc postNotificationName:kPaintSelectorChangedFillRuleNotification object:self];
}

@end
