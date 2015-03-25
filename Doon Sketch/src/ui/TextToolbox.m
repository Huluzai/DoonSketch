//
//  TextToolbox.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/5.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "TextToolbox.h"
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

@interface TextToolbox ()

@end

@implementation TextToolbox

- (id)initWithSPDesktop:(SPDesktop *)desktop
{
    if (self = [super initWithSPDesktop:desktop]) {
        self.zStyle = [[ZGDesktopStyle alloc] initWithDesktop:desktop];
        [self.zStyle addObserver:self
                      forKeyPath:@"fontBold"
                         options:NSKeyValueObservingOptionNew
                         context:nill];
        [self.zStyle addObserver:self
                      forKeyPath:@"fontItalic"
                         options:NSKeyValueObservingOptionNew
                         context:nill];
        [self.zStyle addObserver:self
                      forKeyPath:@"superscript"
                         options:NSKeyValueObservingOptionNew
                         context:nill];
        [self.zStyle addObserver:self
                      forKeyPath:@"subscript"
                         options:NSKeyValueObservingOptionNew
                         context:nill];
    }
    return self;
}

- (void)dealloc
{
    [self.zStyle removeObserver:self forKeyPath:@"fontBold"];
    [self.zStyle removeObserver:self forKeyPath:@"fontItalic"];
    [self.zStyle removeObserver:self forKeyPath:@"superscript"];
    [self.zStyle removeObserver:self forKeyPath:@"subscript"];
    self.zStyle = nill;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self.popupFamilys setTitle:@"Loading..."];
//    [self performSelectorInBackground:@selector(buildFamilyList) withObject:nill];
    [self buildFamilyList];
    [self.comboSizes removeAllItems];
    [self.comboSizes addItemsWithObjectValues:[ZGDesktopStyle availableFontSizes]];
    [self.comboSizes setStringValue:@"40"];
}

- (NSString *)toolboxNibName
{
    return @"TextToolbox";
}

- (void)buildFamilyList
{
    Glib::RefPtr<Gtk::ListStore> store = Inkscape::FontLister::get_instance()->get_font_list();
    GtkListStore* model = store->gobj();
    GtkTreeIter iter;
    gint n = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), NULL);

    NSMenu *menu = [[NSMenu alloc] init];
    
    g_debug("%d fonts founded", n);
    
    for (int i = 0; i < n; i++) {
        gchar *family;
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(model), &iter, NULL, i);
        gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, 0, &family, -1);
        gchar *const family_escaped = g_markup_escape_text(family, -1);
        NSMenuItem *item = [[NSMenuItem alloc] init];
        item.title = [NSString stringWithUTF8String:family_escaped];
        item.target = self;
        [menu addItem:item];
        g_free(family);
        g_free(family_escaped);
    }
    
    [self.popupFamilys performSelectorOnMainThread:@selector(setMenu:) withObject:menu waitUntilDone:FALSE];
}

- (IBAction)didChangeStyle:(id)sender
{
    self.zStyle.fontItalic = [self.segmentStyle isSelectedForSegment:1];
    self.zStyle.fontBold = [self.segmentStyle isSelectedForSegment:0];
}

- (IBAction)didChangeScript:(id)sender
{
    self.zStyle.subscript = [self.segmentScript isSelectedForSegment:1];
    self.zStyle.superscript = [self.segmentScript isSelectedForSegment:0];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualTo:@"fontBold"]) {
        [self.segmentStyle setSelected:self.zStyle.fontBold forSegment:0];
    } else if ([keyPath isEqualTo:@"fontItalic"]) {
        [self.segmentStyle setSelected:self.zStyle.fontItalic forSegment:1];
    } else if ([keyPath isEqualTo:@"superScript"]) {
        [self.segmentScript setSelected:self.zStyle.superscript forSegment:0];
    } else if ([keyPath isEqualTo:@"subScript"]) {
        [self.segmentScript setSelected:self.zStyle.subscript forSegment:1];
    }
}


@end
