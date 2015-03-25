//
//  PaintSelector.h
//  Inxcape
//
//  Created by 张 光建 on 14-10-11.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include <glib.h>

#include <fill-or-stroke.h>
#include <sp-gradient-spread.h>
#include <sp-gradient-units.h>
#include <gtk/gtk.h>
#include <color.h>
#include <libnr/nr-forward.h>
#include <widgets/paint-selector.h>

class SPGradient;
class SPDesktop;

extern NSString *kPaintSelectorChangedFillRuleNotification;

@interface PaintSelector : NSViewController

@property SPColor color;
@property float alpha;
@property(weak) id delegate;
@property SPPaintSelector::Mode mode;
@property NSColor *uiColor;
@property SPPaintSelector::FillRule fillRule;

@property IBOutlet NSView *modePlace;
@property IBOutlet NSView *modeColorPage;
@property IBOutlet NSView *modeNonePage;
@property IBOutlet NSView *modeUnsetPage;
@property IBOutlet NSView *modeLinearPage;
@property IBOutlet NSView *modeRadialPage;
@property IBOutlet NSView *modeSwatchPage;
@property IBOutlet NSView *modePatternPage;
@property IBOutlet NSView *modeEmptyPage;
@property IBOutlet NSView *modeMultiplePage;

@property IBOutlet NSButton *checkFillRule;
@property IBOutlet NSView *linearContentView;
@property IBOutlet NSView *radialContentView;
@property IBOutlet NSSegmentedControl *segmentModes;

- (IBAction)didChangeFillRule:(id)sender;
- (IBAction)didChangeModes:(id)sender;

+ (SPPaintSelector::Mode)getModeForStyle:(SPStyle const &)style fillOrStroke:(FillOrStroke)kind;

- (void)setColor:(SPColor const &)color alpha:(float)alpha;
- (void)getColor:(SPColor &)color alpha:(gfloat &)alpha;

- (void)setGradientLinear:(SPGradient *)vector;
- (void)setGradientRadial:(SPGradient *)vector;
- (void)setSwatch:(SPGradient *)vector;

- (void)setGradientProperties:(SPGradientUnits)units spread:(SPGradientSpread)spread;
- (void)getGradientProperties:(SPGradientUnits &)units spread:(SPGradientSpread &)spread;

- (void)pushAttrsToGradient:(SPGradient *)gr;
- (SPGradient *)getGradientVector;
- (SPPattern *)getPattern;
- (void)updatePatternList:(SPPattern *)pat;

// TODO move this elsewhere:
- (void)setFlatColor:(SPDesktop *)desktop colorProperty:(const gchar *)color_property opacity:(const gchar *)opacity_property;

@end


@interface PaintSelector (Delegate)

- (void)didPaintSelector:(PaintSelector *)sel changeMode:(SPPaintSelector::Mode)mode;
- (void)didPaintSelectorChanged:(PaintSelector *)sel;

@end

