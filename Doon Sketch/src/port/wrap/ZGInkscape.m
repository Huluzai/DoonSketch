//
//  ZGInkscape.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/2.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGInkscape.h"
#import <glib-object.h>
#import <gobject/gobject.h>
#import <application/editor.h>
#import <sigc++/sigc++.h>

#define BINDED_HANDLER_NUM  (6)

static void zg_modify_selection (Inkscape::Application *inkscape,
                                 Inkscape::Selection *selection,
                                 guint flags,
                                 void *obj)
{
    ZGInkscape *ink = (__bridge ZGInkscape *)obj;
    if ([ink.delegate respondsToSelector:@selector(didInkscape:modifySelection:withFlags:)]) {
        [ink.delegate didInkscape:ink modifySelection:selection withFlags:flags];
    }
}

static void zg_change_selection (Inkscape::Application *inkscape,
                                 Inkscape::Selection *selection,
                                 void *obj)
{
    ZGInkscape *ink = (__bridge ZGInkscape *)obj;
    g_debug("panel is %s", [[ink description] UTF8String]);
    if ([ink.delegate respondsToSelector:@selector(didInkscape:changeSelection:)]) {
        [ink.delegate didInkscape:ink changeSelection:selection];
    }
}

static void zg_set_selection (Inkscape::Application */*inkscape*/,
                              Inkscape::Selection *selection,
                              void* obj)
{
    ZGInkscape *ink = (__bridge ZGInkscape *)obj;
    if ([ink.delegate respondsToSelector:@selector(didInkscape:setSelection:)]) {
        [ink.delegate didInkscape:ink setSelection:selection];
    }
}

static void zg_change_subsel (Inkscape::Application */*inkscape*/,
                              Inkscape::Selection *selection,
                              void* obj)
{
    ZGInkscape *ink = (__bridge ZGInkscape *)obj;
    if ([ink.delegate respondsToSelector:@selector(didInkscape:changeSubselection:)]) {
        [ink.delegate didInkscape:ink changeSubselection:selection];
    }
}

static void on_desktop_actived (Inkscape::Application */*inkscape*/, SPDesktop *desktop, void* sender)
{
    ZGInkscape *ink = (__bridge ZGInkscape *)sender;
    if ([ink.delegate respondsToSelector:@selector(didInkscape:activeDesktop:)]) {
        [ink.delegate didInkscape:ink activeDesktop:desktop];
    }
}

static void on_desktop_deactived (Inkscape::Application */*inkscape*/, SPDesktop *desktop, void* sender)
{
    ZGInkscape *ink = (__bridge ZGInkscape *)sender;
    if ([ink.delegate respondsToSelector:@selector(didInkscape:deactiveDesktop:)]) {
        [ink.delegate didInkscape:ink deactiveDesktop:desktop];
    }
}

@interface ZGInkscape () {
    gulong handler[BINDED_HANDLER_NUM];
    gpointer _inkscape;
}
@end

@implementation ZGInkscape

- (id)initWithInkscape:(Inkscape::Application *)inkscape
{
    if (self = [super init]) {
        g_object_add_weak_pointer(G_OBJECT (inkscape), &_inkscape);
        _inkscape = inkscape;
        handler[0] = g_signal_connect (G_OBJECT (inkscape), "modify_selection", G_CALLBACK (zg_modify_selection), (__bridge void*)self);
        handler[1] = g_signal_connect (G_OBJECT (inkscape), "change_selection", G_CALLBACK (zg_change_selection), (__bridge void*)self);
        handler[2] = g_signal_connect (G_OBJECT (inkscape), "set_selection", G_CALLBACK (zg_set_selection), (__bridge void*)self);
        handler[3] = g_signal_connect (G_OBJECT (inkscape), "change_subselection", G_CALLBACK (zg_change_subsel), (__bridge void*)self);
        handler[4] = g_signal_connect (G_OBJECT (inkscape), "deactivate_desktop", G_CALLBACK (on_desktop_deactived), (__bridge void*)self);
        handler[5] = g_signal_connect (G_OBJECT (inkscape), "activate_desktop", G_CALLBACK (on_desktop_actived), (__bridge void*)self);
    }
    
    return self;
}

- (void)dealloc
{
    if (_inkscape) {
        for (int i = 0; i < BINDED_HANDLER_NUM; i++) {
            g_signal_handler_disconnect(G_OBJECT(_inkscape), handler[i]);
        }
        gpointer p = _inkscape;
        g_object_remove_weak_pointer(G_OBJECT(p), &_inkscape);
        _inkscape = NULL;
    }
    _inkscape = NULL;
}

- (Inkscape::Application *)inkscape
{
    return (Inkscape::Application *)_inkscape;
}

@end
