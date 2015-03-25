//
//  TextParameter.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "TextParameter.h"
#import "ZGInkscape.h"
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

//#define DEBUG_TEXT

@interface TextParameter ()
@property ZGInkscape *inkscape;
@property BOOL freeze;
@property BOOL textStyleFromPrefs;
@end

@implementation TextParameter

- (id)init
{
    if (self = [super init]) {
        self.inkscape = [[ZGInkscape alloc] initWithInkscape:INKSCAPE];
        self.inkscape.delegate = self;
        self.textStyleFromPrefs = TRUE;
        if (SP_ACTIVE_DESKTOP) {
            [self didInkscape:self.inkscape activeDesktop:SP_ACTIVE_DESKTOP];
        }
    }
    return self;
}

- (void)dealloc
{
    self.desktopStyle = nill;
    self.inkscape = nill;
}

- (NSString *)nibName
{
    return @"TextParameter";
}

- (void) didInkscape:(ZGInkscape *)inkscape activeDesktop:(SPDesktop *)desktop
{
    [self.desktopStyle removeObserver:self forKeyPath:@"fontBold"];
    [self.desktopStyle removeObserver:self forKeyPath:@"fontItalic"];
 
    if (desktop) {
        self.desktopStyle = nill;
        self.desktopStyle = [[ZGDesktopStyle alloc] initWithDesktop:desktop];
        [self.desktopStyle addObserver:self
                            forKeyPath:@"fontBold"
                               options:NSKeyValueObservingOptionNew
                               context:NULL];
        [self.desktopStyle addObserver:self
                            forKeyPath:@"fontItalic"
                               options:NSKeyValueObservingOptionNew
                               context:NULL];
    }
}

- (void) didInkscape:(ZGInkscape *)inkscape deactiveDesktop:(SPDesktop *)desktop
{
    [self.desktopStyle removeObserver:self forKeyPath:@"fontBold"];
    [self.desktopStyle removeObserver:self forKeyPath:@"fontItalic"];
    self.desktopStyle = nill;
}

- (void)loadView
{
    [super loadView];
    // Do view setup here.
    
    // Font list
    Glib::RefPtr<Gtk::ListStore> store = Inkscape::FontLister::get_instance()->get_font_list();
    GtkListStore* model = store->gobj();
    GtkTreeIter iter;
    gint n = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), NULL);
    
    for (int i = 0; i < n; i++) {
        gchar *family;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(model), &iter, NULL, i);
        gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, 0, &family, -1);
        gchar *const family_escaped = g_markup_escape_text(family, -1);
        [self.fontFamilyPopUpButton addItemWithTitle:[NSString stringWithUTF8String:family_escaped]];
        g_free(family);
        g_free(family_escaped);
    }
//    sp_text_toolbox_selection_changed(self, NULL, NULL);
}

- (IBAction)didChangeTextStyle:(id)sender
{
    NSSegmentedControl *sc = (NSSegmentedControl *)sender;
    self.desktopStyle.fontBold = [sc isSelectedForSegment:0];
    self.desktopStyle.fontItalic = [sc isSelectedForSegment:1];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    if ([keyPath isEqualToString:@"fontBold"]) {
        [self.styleSegmentControl setSelected:self.desktopStyle.fontBold forSegment:0];
    } else if ([keyPath isEqualToString:@"fontItalic"]) {
        [self.styleSegmentControl setSelected:self.desktopStyle.fontItalic forSegment:1];
    }
}

@end

