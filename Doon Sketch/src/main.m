//
//  main.m
//  Inkscape X
//
//  Created by 张 光建 on 14-9-14.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#import <Cocoa/Cocoa.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gc-core.h>
#include <debug/logger.h>
#include <debug/log-display-config.h>
#include <extension/extension.h>
#include <extension/system.h>
#include <extension/db.h>
#include <extension/output.h>
#include <extension/input.h>
#include <extension/init.h>
#include <glibmm/i18n.h>
#include <path-prefix.h>
#include <libnrtype/FontFactory.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/font-lister.h>

#ifndef HAVE_BIND_TEXTDOMAIN_CODESET
#define bind_textdomain_codeset(p,c)
#endif

#include <inkscape.h>
#include <application/application.h>
#include <errno.h>

#include <sp-namedview.h>
#include "preferences.h"
#include "sp-object-repr.h"
#include "sp-guide.h"
#include <gtk/gtk.h>

static void set_extensions_env();
static void captureLogMessages();


#include <pango/pangocairo.h>
#include <pango/pangocoretext.h>
#include <freetype.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdkmm.h>
#include <gtkmm/main.h>
#include <gtk/gtk.h>
#include <gtkmm.h>

#include <verbs.h>

extern void setPortraceTracingEngionCallback(void (*callback)(double progress, void *privdata));

static void on_portrace_status(double progress, void *privdata)
{
    g_message("portrace in progress : %.2f", progress);
}

static void on_object_weak_removed(gpointer data, GObject *object)
{
    g_assert(data && object);
}

static void testWeakRef()
{
    GtkWidget *bt = gtk_button_new();
    g_object_weak_ref(G_OBJECT(bt), (GWeakNotify)&on_object_weak_removed, NULL);
    g_object_weak_unref(G_OBJECT(bt), (GWeakNotify)&on_object_weak_removed, NULL);
    g_object_weak_unref(G_OBJECT(bt), (GWeakNotify)&on_object_weak_removed, NULL);
    g_object_unref(G_OBJECT(bt));
}

static void testFt()
{
    g_debug("--- test freetype2 start ---");
    
    PangoFontMap * fontServer = pango_cairo_font_map_get_default();
    PangoContext *ctx = pango_font_map_create_context(fontServer);
    
    PangoFontDescription *font_description = pango_font_description_from_string("Sans");
    PangoFont *font = pango_font_map_load_font(fontServer, ctx, font_description);

    PangoAttrList *attlist = pango_attr_list_new();
    PangoAttribute *fontattr = pango_attr_font_desc_new(font_description);
    pango_attr_list_insert(attlist, fontattr);
    
    const char *text = "hello, world";
    
    GList * itemslist = pango_itemize(ctx, text, 0, (int)strlen(text), attlist, NULL);
    PangoItem *headitem = (PangoItem*)itemslist->data;
    
    PangoGlyphString *glyphString = pango_glyph_string_new();
    pango_shape(text, headitem->length, &(headitem->analysis), glyphString);
    
    g_debug("Result glyphs number : %d", glyphString->num_glyphs);
    for (int i = 0; i < glyphString->num_glyphs; i++) {
        g_debug("\tglyph %d : 0x%08x", i, glyphString->glyphs[i].glyph);
    }
    
    for (GList *current_pango_item = itemslist ; current_pango_item != NULL ; current_pango_item = current_pango_item->next) {
        PangoItem *item = (PangoItem*)current_pango_item->data;
        PangoFontDescription *font_description = pango_font_describe(item->analysis.font);
        pango_font_description_free(font_description);   // Face() makes a copy
    }
    g_list_free(itemslist);
    /*
    for (int i = 0; i < 0xffffff; i++) {
        FT_Error err = FT_Load_Glyph(theFace, i, 0);
        if (err) {
            g_debug("Load glyph 0x%04x failed:%02x", i, err);
            break;
        }
    }
    */
    g_debug("---- test freetype2 finish ---");
}

void testGdkPixbuf()
{
    /* XPM */
    static char const *handle_center_xpm[] = {
        "13 13 3 1",
        " 	c None",
        ".	c #000000",
        "+	c #FFFFFF",
        "             ",
        "      .      ",
        "      .      ",
        "      .      ",
        "    ++.++    ",
        "    ++.++    ",
        " ..... ..... ",
        "    ++.++    ",
        "    ++.++    ",
        "      .      ",
        "      .      ",
        "      .      ",
        "             "};
    
    GtkButton* cbutton = (GtkButton *)gtk_button_new_with_label("hello");
    Gtk::Button* button = Glib::wrap(cbutton);
    g_assert(button);
    
    GdkPixbuf *p = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 1, 8, 200, 200);
    g_assert(p);
    Glib::RefPtr<Gdk::Pixbuf> gp = Glib::wrap(p);
    g_assert(gp);

    GdkPixbuf *pb = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_center_xpm);
    g_assert(pb);
}

#include "sp-canvas-cocoa.h"

void testCanvas()
{
/*    CanvasBase *c = new SPCocoaCanvas();
    g_assert(c);
    c->unref();
    GObject *p = G_OBJECT(c);
    g_assert_not_reached();*/
}

#include <glib/gthread.h>

static gpointer thread_0(gpointer data)
{
    return 0;
}

static void thread_test()
{
    GThread *thr = g_thread_new("thread_0", (GThreadFunc)thread_0, NULL);
    g_assert(thr);
    g_debug("1 processes num : %d", g_get_num_processors());
    GThread *thr2 = g_thread_new("thread_0", (GThreadFunc)thread_0, NULL);
    g_debug("2 processes num : %d", g_get_num_processors());
}

static void a_shared_func()
{
    static bool busy = false;
    g_assert(!busy);
    
    busy = true;
    sleep(100);
    busy = false;
}

static gboolean idle_handler(gpointer data)
{
    a_shared_func();
    return TRUE;
}

static gboolean idle_handler2(gpointer data)
{
    a_shared_func();
    return TRUE;
}

static gboolean idle_handler3(gpointer data)
{
    a_shared_func();
    return TRUE;
}

static void idle_test()
{
    g_idle_add(idle_handler, 0);
    g_idle_add(idle_handler2, 0);
    g_idle_add(idle_handler3, 0);
}

static void printVerbs()
{
    g_print("verbs list : \n");
    
    for (int i = 0; i < SP_VERB_LAST; i++) {
        Inkscape::Verb *v = Inkscape::Verb::get(i);
        if (v) {
            g_print("[ %04d ] %s | %s \n", i, v->get_id(), v->get_name());
        }
    }
}

#include <gc-soft-ptr.h>

static void testGC()
{
    int a = 100;
    int *p = new int[10];
    Inkscape::GC::soft_ptr<int> softPtr = p;
    delete p;
}

int main(int argc, const char * argv[])
{
    captureLogMessages();

//    Glib::init();
#if 1
    gtk_init (&argc, (char ***)&argv);

    Inkscape::Debug::Logger::init();
    Gtk::Main::init_gtkmm_internals();
    
#undef ENABLE_NLS
    
#if defined(ENABLE_NLS)
# ifdef ENABLE_BINRELOC
    bindtextdomain(GETTEXT_PACKAGE, BR_LOCALEDIR(""));
# else
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
# endif
#endif
    
    // the bit below compiles regardless of platform
#ifdef ENABLE_NLS
    // Allow the user to override the locale directory by setting
    // the environment variable INKSCAPE_LOCALEDIR.
    char const *inkscape_localedir = g_getenv("INKSCAPE_LOCALEDIR");
    if (inkscape_localedir != NULL) {
        bindtextdomain(GETTEXT_PACKAGE, inkscape_localedir);
    }
    
    // common setup
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif
    
    set_extensions_env();
#endif
    
    g_setenv("_INKSCAPE_GC", "alternative", TRUE);
    Inkscape::GC::init();

    testGC();
    
    inkscape_application_init("", false);
  
    sp_object_type_register ("sodipodi:namedview", SP_TYPE_NAMEDVIEW);
    sp_object_type_register ("sodipodi:guide", SP_TYPE_GUIDE);
    
    Inkscape::Preferences::get(); // Ensure preferences are loaded

//    testFt();
//    testGdkPixbuf();
//    testCanvas();
//    thread_test();
//    idle_test();
//    printVerbs();
//    testWeakRef();
    
    setPortraceTracingEngionCallback(&on_portrace_status);

    return NSApplicationMain(argc, argv);
}

static void loggingCallback (const gchar *log_domain,
                             GLogLevelFlags log_level,
                             const gchar *message,
                             gpointer user_data)
{
    if (log_level & G_LOG_LEVEL_DEBUG) {
#ifdef DEBUG
        g_print("** (%s): DEBUG **: %s\n", log_domain, message);
#endif
    } else {
        // need a env varivate to print debug info
        g_log_default_handler(log_domain, log_level, message, user_data);
        if (log_level & G_LOG_LEVEL_CRITICAL) {
//            G_BREAKPOINT();
        }
    }
    //    G_BREAKPOINT();
}

static void captureLogMessages()
{
    /*
     This might likely need more code, to capture Gtkmm
     and Glibmm warnings, or maybe just simply grab stdout/stderr
     */
    GLogLevelFlags flags = (GLogLevelFlags) (G_LOG_LEVEL_ERROR   | G_LOG_LEVEL_CRITICAL |
                                             G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE  |
                                             G_LOG_LEVEL_INFO    | G_LOG_LEVEL_DEBUG);
    g_log_set_handler(NULL, flags, loggingCallback, NULL);
    g_log_set_handler("GLib", flags, loggingCallback, NULL);
    g_log_set_handler("Gtk", flags, loggingCallback, NULL);
    g_log_set_handler("Gdk", flags, loggingCallback, NULL);
    g_log_set_handler("GLib-GObject", flags, loggingCallback, NULL);
}

static void set_extensions_env()
{
    gchar const *pythonpath = g_getenv("PYTHONPATH");
    gchar *extdir;
    gchar *new_pythonpath;
    
#ifdef WIN32
    extdir = g_win32_locale_filename_from_utf8(INKSCAPE_EXTENSIONDIR);
#else
    extdir = g_strdup(INKSCAPE_EXTENSIONDIR);
#endif
    
    // On some platforms, INKSCAPE_EXTENSIONDIR is not absolute,
    // but relative to the directory that contains the Inkscape executable.
    // Since we spawn Python chdir'ed into the script's directory,
    // we need to obtain the absolute path here.
    if (!g_path_is_absolute(extdir)) {
        gchar *curdir = g_get_current_dir();
        gchar *extdir_new = g_build_filename(curdir, extdir, NULL);
        g_free(extdir);
        g_free(curdir);
        extdir = extdir_new;
    }
    
    if (pythonpath) {
        new_pythonpath = g_strdup_printf("%s" G_SEARCHPATH_SEPARATOR_S "%s",
                                         extdir, pythonpath);
        g_free(extdir);
    } else {
        new_pythonpath = extdir;
    }
    
    g_setenv("PYTHONPATH", new_pythonpath, TRUE);
    g_free(new_pythonpath);
    //printf("PYTHONPATH = %s\n", g_getenv("PYTHONPATH"));
}


