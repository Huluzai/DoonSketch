#define __SP_INTERFACE_C__

/** @file
 * @brief Main UI stuff
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2004 David Turner
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtk/gtk.h>
#include <glib.h>

#include "inkscape-private.h"
#include "extension/db.h"
#include "extension/effect.h"
#include "extension/input.h"
#include "widgets/icon.h"
#include "preferences.h"
#include "path-prefix.h"
#include "shortcuts.h"
#include "document.h"
#include "desktop-handles.h"
#include "file.h"
#include "interface.h"
#include "desktop.h"
#include "ui/context-menu.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "svg-view-widget.h"
#include "widgets/desktop-widget.h"
#include "sp-item-group.h"
#include "sp-text.h"
#include "sp-gradient-fns.h"
#include "sp-gradient.h"
#include "sp-flowtext.h"
#include "sp-namedview.h"
#include "ui/view/view.h"
#include "helper/action.h"
#include "helper/gnome-utils.h"
#include "helper/window.h"
#include "io/sys.h"
#include "dialogs/dialog-events.h"
#include "message-context.h"
#include "ui/uxmanager.h"

// Added for color drag-n-drop
#if ENABLE_LCMS
#include "lcms.h"
#endif // ENABLE_LCMS
#include "display/sp-canvas.h"
#include "color.h"
#include "svg/svg-color.h"
#include "desktop-style.h"
#include "style.h"
#include "event-context.h"
#include "gradient-drag.h"
#include "widgets/ege-paint-def.h"
#include "display/canvas-base.h"
#include "display/sp-canvas-gtk.h"

/* Drag and Drop */
typedef enum {
    URI_LIST,
    SVG_XML_DATA,
    SVG_DATA,
    PNG_DATA,
    JPEG_DATA,
    IMAGE_DATA,
    APP_X_INKY_COLOR,
    APP_X_COLOR,
    APP_OSWB_COLOR,
} ui_drop_target_info;

static GtkTargetEntry ui_drop_target_entries [] = {
    {(gchar *)"text/uri-list",                0, URI_LIST        },
    {(gchar *)"image/svg+xml",                0, SVG_XML_DATA    },
    {(gchar *)"image/svg",                    0, SVG_DATA        },
    {(gchar *)"image/png",                    0, PNG_DATA        },
    {(gchar *)"image/jpeg",                   0, JPEG_DATA       },
#if ENABLE_MAGIC_COLORS
    {(gchar *)"application/x-inkscape-color", 0, APP_X_INKY_COLOR},
#endif // ENABLE_MAGIC_COLORS
    {(gchar *)"application/x-oswb-color",     0, APP_OSWB_COLOR  },
    {(gchar *)"application/x-color",          0, APP_X_COLOR     }
};

static GtkTargetEntry *completeDropTargets = 0;
static int completeDropTargetsCount = 0;
static bool temporarily_block_actions = false;

#define ENTRIES_SIZE(n) sizeof(n)/sizeof(n[0])
static guint nui_drop_target_entries = ENTRIES_SIZE(ui_drop_target_entries);
static void sp_ui_import_files(gchar *buffer);
static void sp_ui_import_one_file(char const *filename);
static void sp_ui_import_one_file_with_check(gpointer filename, gpointer unused);
static void sp_ui_drag_data_received(GtkWidget *widget,
                                     GdkDragContext *drag_context,
                                     gint x, gint y,
                                     GtkSelectionData *data,
                                     guint info,
                                     guint event_time,
                                     gpointer user_data);
static void sp_ui_drag_motion( GtkWidget *widget,
                               GdkDragContext *drag_context,
                               gint x, gint y,
                               GtkSelectionData *data,
                               guint info,
                               guint event_time,
                               gpointer user_data );
static void sp_ui_drag_leave( GtkWidget *widget,
                              GdkDragContext *drag_context,
                              guint event_time,
                              gpointer user_data );
static void sp_ui_menu_item_set_sensitive(SPAction *action,
                                          unsigned int sensitive,
                                          void *data);
static void sp_ui_menu_item_set_name(SPAction *action,
                                     Glib::ustring name,
                                     void *data);
static void sp_recent_open(GtkRecentChooser *, gpointer);

static void injectRenamedIcons();

SPActionEventVector menu_item_event_vector = {
    {NULL},
    NULL,
    NULL, /* set_active */
    sp_ui_menu_item_set_sensitive, /* set_sensitive */
    NULL, /* set_shortcut */
    sp_ui_menu_item_set_name /* set_name */
};

static const int MIN_ONSCREEN_DISTANCE = 50;

void
sp_create_window(SPViewWidget *vw, gboolean editable)
{
    g_return_if_fail(vw != NULL);
    g_return_if_fail(SP_IS_VIEW_WIDGET(vw));

    Gtk::Window *win = Inkscape::UI::window_new("", TRUE);

    gtk_container_add(GTK_CONTAINER(win->gobj()), GTK_WIDGET(vw));
    gtk_widget_show(GTK_WIDGET(vw));

    if (editable) {
        g_object_set_data(G_OBJECT(vw), "window", win);

        SPDesktopWidget *desktop_widget = reinterpret_cast<SPDesktopWidget*>(vw);
        SPDesktop* desktop = desktop_widget->desktop;

        desktop_widget->window = win;

        win->set_data("desktop", desktop);
        win->set_data("desktopwidget", desktop_widget);

        win->signal_delete_event().connect(sigc::mem_fun(*(SPDesktop*)vw->view, &SPDesktop::onDeleteUI));
        win->signal_window_state_event().connect(sigc::mem_fun(*desktop, &SPDesktop::onWindowStateEvent));
        win->signal_focus_in_event().connect(sigc::mem_fun(*desktop_widget, &SPDesktopWidget::onFocusInEvent));

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        gint prefs_geometry =
            (2==prefs->getInt("/options/savewindowgeometry/value", 0));
        if (prefs_geometry) {
            gint pw = prefs->getInt("/desktop/geometry/width", -1);
            gint ph = prefs->getInt("/desktop/geometry/height", -1);
            gint px = prefs->getInt("/desktop/geometry/x", -1);
            gint py = prefs->getInt("/desktop/geometry/y", -1);
            gint full = prefs->getBool("/desktop/geometry/fullscreen");
            gint maxed = prefs->getBool("/desktop/geometry/maximized");
            if (pw>0 && ph>0) {
                gint w = MIN(gdk_screen_width(), pw);
                gint h = MIN(gdk_screen_height(), ph);
                gint x = MIN(gdk_screen_width() - MIN_ONSCREEN_DISTANCE, px);
                gint y = MIN(gdk_screen_height() - MIN_ONSCREEN_DISTANCE, py);
                if (w>0 && h>0) {
                    x = MIN(gdk_screen_width() - w, x);
                    y = MIN(gdk_screen_height() - h, y);
                    desktop->setWindowSize(w, h);
                }

                // Only restore xy for the first window so subsequent windows don't overlap exactly
                // with first.  (Maybe rule should be only restore xy if it's different from xy of
                // other desktops?)

                // Empirically it seems that active_desktop==this desktop only the first time a
                // desktop is created.
                SPDesktop *active_desktop = SP_ACTIVE_DESKTOP;
                if (active_desktop == desktop || active_desktop==NULL) {
                    desktop->setWindowPosition(Geom::Point(x, y));
                }
            }
            if (maxed) {
                win->maximize();
            }
            if (full) {
                win->fullscreen();
            }
        }

    } else {
        gtk_window_set_policy(GTK_WINDOW(win->gobj()), TRUE, TRUE, TRUE);
    }

    if ( completeDropTargets == 0 || completeDropTargetsCount == 0 )
    {
        std::vector<gchar*> types;

        GSList *list = gdk_pixbuf_get_formats();
        while ( list ) {
            int i = 0;
            GdkPixbufFormat *one = (GdkPixbufFormat*)list->data;
            gchar** typesXX = gdk_pixbuf_format_get_mime_types(one);
            for ( i = 0; typesXX[i]; i++ ) {
                types.push_back(g_strdup(typesXX[i]));
            }
            g_strfreev(typesXX);

            list = g_slist_next(list);
        }
        completeDropTargetsCount = nui_drop_target_entries + types.size();
        completeDropTargets = new GtkTargetEntry[completeDropTargetsCount];
        for ( int i = 0; i < (int)nui_drop_target_entries; i++ ) {
            completeDropTargets[i] = ui_drop_target_entries[i];
        }
        int pos = nui_drop_target_entries;

        for (std::vector<gchar*>::iterator it = types.begin() ; it != types.end() ; it++) {
            completeDropTargets[pos].target = *it;
            completeDropTargets[pos].flags = 0;
            completeDropTargets[pos].info = IMAGE_DATA;
            pos++;
        }
    }

    gtk_drag_dest_set((GtkWidget*)win->gobj(),
                      GTK_DEST_DEFAULT_ALL,
                      completeDropTargets,
                      completeDropTargetsCount,
                      GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));


    g_signal_connect(G_OBJECT(win->gobj()),
                     "drag_data_received",
                     G_CALLBACK(sp_ui_drag_data_received),
                     NULL);
    g_signal_connect(G_OBJECT(win->gobj()),
                     "drag_motion",
                     G_CALLBACK(sp_ui_drag_motion),
                     NULL);
    g_signal_connect(G_OBJECT(win->gobj()),
                     "drag_leave",
                     G_CALLBACK(sp_ui_drag_leave),
                     NULL);
    win->show();

    // needed because the first ACTIVATE_DESKTOP was sent when there was no window yet
    inkscape_reactivate_desktop(SP_DESKTOP_WIDGET(vw)->desktop);
}

void
sp_ui_new_view()
{
    SPDocument *document;
    SPViewWidget *dtw;

    document = SP_ACTIVE_DOCUMENT;
    if (!document) return;

    dtw = sp_desktop_widget_new(sp_document_namedview(document, NULL));
    g_return_if_fail(dtw != NULL);

    sp_create_window(dtw, TRUE);
    sp_namedview_window_from_document(static_cast<SPDesktop*>(dtw->view));
    sp_namedview_update_layers_from_document(static_cast<SPDesktop*>(dtw->view));
}

/* TODO: not yet working */
/* To be re-enabled (by adding to menu) once it works. */
void
sp_ui_new_view_preview()
{
    SPDocument *document;
    SPViewWidget *dtw;

    document = SP_ACTIVE_DOCUMENT;
    if (!document) return;

    dtw = (SPViewWidget *) sp_svg_view_widget_new(document);
    g_return_if_fail(dtw != NULL);
    sp_svg_view_widget_set_resize(SP_SVG_VIEW_WIDGET(dtw), TRUE, 400.0, 400.0);

    sp_create_window(dtw, FALSE);
}

/**
 * \param widget unused
 */
void
sp_ui_close_view(GtkWidget */*widget*/)
{
	SPDesktop *dt = SP_ACTIVE_DESKTOP;

	if (dt == NULL) {
        return;
    }

    if (dt->shutdown()) {
        return; // Shutdown operation has been canceled, so do nothing
    }

    // Shutdown can proceed; use the stored reference to the desktop here instead of the current SP_ACTIVE_DESKTOP,
    // because the user might have changed the focus in the meantime (see bug #381357 on Launchpad)
    dt->destroyWidget();
}


/**
 *  sp_ui_close_all
 *
 *  This function is called to exit the program, and iterates through all
 *  open document view windows, attempting to close each in turn.  If the
 *  view has unsaved information, the user will be prompted to save,
 *  discard, or cancel.
 *
 *  Returns FALSE if the user cancels the close_all operation, TRUE
 *  otherwise.
 */
unsigned int
sp_ui_close_all(void)
{
    /* Iterate through all the windows, destroying each in the order they
       become active */
    while (SP_ACTIVE_DESKTOP) {
    	SPDesktop *dt = SP_ACTIVE_DESKTOP;
    	if (dt->shutdown()) {
            /* The user canceled the operation, so end doing the close */
            return FALSE;
        }
    	// Shutdown can proceed; use the stored reference to the desktop here instead of the current SP_ACTIVE_DESKTOP,
    	// because the user might have changed the focus in the meantime (see bug #381357 on Launchpad)
    	dt->destroyWidget();
    }

    return TRUE;
}

/*
 * Some day when the right-click menus are ready to start working
 * smarter with the verbs, we'll need to change this NULL being
 * sent to sp_action_perform to something useful, or set some kind
 * of global "right-clicked position" variable for actions to
 * investigate when they're called.
 */
static void
sp_ui_menu_activate(void */*object*/, SPAction *action)
{
    if (!temporarily_block_actions) {
    	sp_action_perform(action, NULL);
    }
}

static void
sp_ui_menu_select_action(void */*object*/, SPAction *action)
{
    action->view->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, action->tip);
}

static void
sp_ui_menu_deselect_action(void */*object*/, SPAction *action)
{
    action->view->tipsMessageContext()->clear();
}

static void
sp_ui_menu_select(gpointer object, gpointer tip)
{
    Inkscape::UI::View::View *view = static_cast<Inkscape::UI::View::View*> (g_object_get_data(G_OBJECT(object), "view"));
    view->tipsMessageContext()->set(Inkscape::NORMAL_MESSAGE, (gchar *)tip);
}

static void
sp_ui_menu_deselect(gpointer object)
{
    Inkscape::UI::View::View *view = static_cast<Inkscape::UI::View::View*>  (g_object_get_data(G_OBJECT(object), "view"));
    view->tipsMessageContext()->clear();
}

/**
 * sp_ui_menuitem_add_icon
 *
 * Creates and attaches a scaled icon to the given menu item.
 *
 */
void
sp_ui_menuitem_add_icon( GtkWidget *item, gchar *icon_name )
{
    static bool iconsInjected = false;
    if ( !iconsInjected ) {
        iconsInjected = true;
        injectRenamedIcons();
    }
    GtkWidget *icon;

    icon = sp_icon_new( Inkscape::ICON_SIZE_MENU, icon_name );
    gtk_widget_show(icon);
    gtk_image_menu_item_set_image((GtkImageMenuItem *) item, icon);
} // end of sp_ui_menu_add_icon

/**
 * sp_ui_menu_append_item
 *
 * Appends a UI item with specific info for Inkscape/Sodipodi.
 *
 */
static GtkWidget *
sp_ui_menu_append_item( GtkMenu *menu, gchar const *stock,
                        gchar const *label, gchar const *tip, Inkscape::UI::View::View *view, GCallback callback,
                        gpointer data, gboolean with_mnemonic = TRUE )
{
    GtkWidget *item;

    if (stock) {
        item = gtk_image_menu_item_new_from_stock(stock, NULL);
    } else if (label) {
        item = (with_mnemonic)
            ? gtk_image_menu_item_new_with_mnemonic(label) :
            gtk_image_menu_item_new_with_label(label);
    } else {
        item = gtk_separator_menu_item_new();
    }

    gtk_widget_show(item);

    if (callback) {
        g_signal_connect(G_OBJECT(item), "activate", callback, data);
    }

    if (tip && view) {
        g_object_set_data(G_OBJECT(item), "view", (gpointer) view);
        g_signal_connect( G_OBJECT(item), "select", G_CALLBACK(sp_ui_menu_select), (gpointer) tip );
        g_signal_connect( G_OBJECT(item), "deselect", G_CALLBACK(sp_ui_menu_deselect), NULL);
    }

    gtk_menu_append(GTK_MENU(menu), item);

    return item;

} // end of sp_ui_menu_append_item()

void
sp_ui_dialog_title_string(Inkscape::Verb *verb, gchar *c)
{
    SPAction     *action;
    unsigned int shortcut;
    gchar        *s;
    gchar        *atitle;

    action = verb->get_action(NULL);
    if (!action)
        return;

    atitle = sp_action_get_title(action);

    s = g_stpcpy(c, atitle);

    g_free(atitle);

    shortcut = sp_shortcut_get_primary(verb);
    if (shortcut != GDK_VoidSymbol) {
        gchar* key = sp_shortcut_get_label(shortcut);
        s = g_stpcpy(s, " (");
        s = g_stpcpy(s, key);
        s = g_stpcpy(s, ")");
        g_free(key);
    }
}


/**
 * sp_ui_menu_append_item_from_verb
 *
 * Appends a custom menu UI from a verb.
 *
 */

static GtkWidget *
sp_ui_menu_append_item_from_verb(GtkMenu *menu, Inkscape::Verb *verb, Inkscape::UI::View::View *view, bool radio = false, GSList *group = NULL)
{
    SPAction *action;
    GtkWidget *item;

    if (verb->get_code() == SP_VERB_NONE) {

        item = gtk_separator_menu_item_new();

    } else {
        unsigned int shortcut;

        action = verb->get_action(view);

        if (!action) return NULL;

        shortcut = sp_shortcut_get_primary(verb);
        if (shortcut != GDK_VoidSymbol) {
            gchar* c = sp_shortcut_get_label(shortcut);
            GtkWidget *const hb = gtk_hbox_new(FALSE, 16);
            GtkWidget *const name_lbl = gtk_label_new("");
            gtk_label_set_markup_with_mnemonic(GTK_LABEL(name_lbl), action->name);
            gtk_misc_set_alignment((GtkMisc *) name_lbl, 0.0, 0.5);
            gtk_box_pack_start((GtkBox *) hb, name_lbl, TRUE, TRUE, 0);
            GtkWidget *const accel_lbl = gtk_label_new(c);
            gtk_misc_set_alignment((GtkMisc *) accel_lbl, 1.0, 0.5);
            gtk_box_pack_end((GtkBox *) hb, accel_lbl, FALSE, FALSE, 0);
            gtk_widget_show_all(hb);
            if (radio) {
                item = gtk_radio_menu_item_new (group);
            } else {
                item = gtk_image_menu_item_new();
            }
            gtk_container_add((GtkContainer *) item, hb);
            g_free(c);
        } else {
            if (radio) {
                item = gtk_radio_menu_item_new (group);
            } else {
                item = gtk_image_menu_item_new ();
            }
            GtkWidget *const name_lbl = gtk_label_new("");
            gtk_label_set_markup_with_mnemonic(GTK_LABEL(name_lbl), action->name);
            gtk_misc_set_alignment((GtkMisc *) name_lbl, 0.0, 0.5);
            gtk_container_add((GtkContainer *) item, name_lbl);
        }

        nr_active_object_add_listener((NRActiveObject *)action, (NRObjectEventVector *)&menu_item_event_vector, sizeof(SPActionEventVector), item);
        if (!action->sensitive) {
            gtk_widget_set_sensitive(item, FALSE);
        }

        if (action->image) {
            sp_ui_menuitem_add_icon(item, action->image);
        }
        gtk_widget_set_events(item, GDK_KEY_PRESS_MASK);
        g_object_set_data(G_OBJECT(item), "view", (gpointer) view);
        g_signal_connect( G_OBJECT(item), "activate", G_CALLBACK(sp_ui_menu_activate), action );
        g_signal_connect( G_OBJECT(item), "select", G_CALLBACK(sp_ui_menu_select_action), action );
        g_signal_connect( G_OBJECT(item), "deselect", G_CALLBACK(sp_ui_menu_deselect_action), action );
    }

    gtk_widget_show(item);
    gtk_menu_append(GTK_MENU(menu), item);

    return item;

} // end of sp_ui_menu_append_item_from_verb


static Glib::ustring getLayoutPrefPath( Inkscape::UI::View::View *view )
{
    Glib::ustring prefPath;

    if (reinterpret_cast<SPDesktop*>(view)->is_focusMode()) {
        prefPath = "/focus/";
    } else if (reinterpret_cast<SPDesktop*>(view)->is_fullscreen()) {
        prefPath = "/fullscreen/";
    } else {
        prefPath = "/window/";
    }

    return prefPath;
}

static void
checkitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data)
{
    gchar const *pref = (gchar const *) user_data;
    Inkscape::UI::View::View *view = (Inkscape::UI::View::View *) g_object_get_data(G_OBJECT(menuitem), "view");

    Glib::ustring pref_path = getLayoutPrefPath( view );
    pref_path += pref;
    pref_path += "/state";

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gboolean checked = gtk_check_menu_item_get_active(menuitem);
    prefs->setBool(pref_path, checked);

    reinterpret_cast<SPDesktop*>(view)->layoutWidget();
}

static gboolean
checkitem_update(GtkWidget *widget, GdkEventExpose */*event*/, gpointer user_data)
{
    GtkCheckMenuItem *menuitem=GTK_CHECK_MENU_ITEM(widget);

    gchar const *pref = (gchar const *) user_data;
    Inkscape::UI::View::View *view = (Inkscape::UI::View::View *) g_object_get_data(G_OBJECT(menuitem), "view");

    Glib::ustring pref_path = getLayoutPrefPath( view );
    pref_path += pref;
    pref_path += "/state";

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool ison = prefs->getBool(pref_path, true);

    g_signal_handlers_block_by_func(G_OBJECT(menuitem), (gpointer)(GCallback)checkitem_toggled, user_data);
    gtk_check_menu_item_set_active(menuitem, ison);
    g_signal_handlers_unblock_by_func(G_OBJECT(menuitem), (gpointer)(GCallback)checkitem_toggled, user_data);

    return FALSE;
}

static void taskToggled(GtkCheckMenuItem *menuitem, gpointer userData)
{
    if ( gtk_check_menu_item_get_active(menuitem) ) {
        gint taskNum = GPOINTER_TO_INT(userData);
        taskNum = (taskNum < 0) ? 0 : (taskNum > 2) ? 2 : taskNum;

        Inkscape::UI::View::View *view = (Inkscape::UI::View::View *) g_object_get_data(G_OBJECT(menuitem), "view");

        // note: this will change once more options are in the task set support:
        Inkscape::UI::UXManager::getInstance()->setTask( dynamic_cast<SPDesktop*>(view), taskNum );
    }
}


/**
 *  \brief Callback function to update the status of the radio buttons in the View -> Display mode menu (Normal, No Filters, Outline)
 */

static gboolean
update_view_menu(GtkWidget *widget, GdkEventExpose */*event*/, gpointer user_data)
{
	SPAction *action = (SPAction *) user_data;
	g_assert(action->id != NULL);

	Inkscape::UI::View::View *view = (Inkscape::UI::View::View *) g_object_get_data(G_OBJECT(widget), "view");
    SPDesktop *dt = static_cast<SPDesktop*>(view);
	Inkscape::RenderMode mode = dt->getMode();

	bool new_state = false;
	if (!strcmp(action->id, "ViewModeNormal")) {
    	new_state = mode == Inkscape::RENDERMODE_NORMAL;
	} else if (!strcmp(action->id, "ViewModeNoFilters")) {
    	new_state = mode == Inkscape::RENDERMODE_NO_FILTERS;
    } else if (!strcmp(action->id, "ViewModeOutline")) {
    	new_state = mode == Inkscape::RENDERMODE_OUTLINE;
    } else if (!strcmp(action->id, "ViewModePrintColorsPreview")) {
    	new_state = mode == Inkscape::RENDERMODE_PRINT_COLORS_PREVIEW;
    } else {
    	g_warning("update_view_menu does not handle this verb");
    }

	if (new_state) { //only one of the radio buttons has to be activated; the others will automatically be deactivated
		if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
			// When the GtkMenuItem version of the "activate" signal has been emitted by a GtkRadioMenuItem, there is a second
			// emission as the most recently active item is toggled to inactive. This is dealt with before the original signal is handled.
			// This emission however should not invoke any actions, hence we block it here:
			temporarily_block_actions = true;
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
			temporarily_block_actions = false;
		}
	}

	return FALSE;
}

void
sp_ui_menu_append_check_item_from_verb(GtkMenu *menu, Inkscape::UI::View::View *view, gchar const *label, gchar const *tip, gchar const *pref,
                                       void (*callback_toggle)(GtkCheckMenuItem *, gpointer user_data),
                                       gboolean (*callback_update)(GtkWidget *widget, GdkEventExpose *event, gpointer user_data),
                                       Inkscape::Verb *verb)
{
    unsigned int shortcut = (verb) ? sp_shortcut_get_primary(verb) : 0;
    SPAction *action = (verb) ? verb->get_action(view) : 0;
    GtkWidget *item = gtk_check_menu_item_new();

    if (verb && (shortcut != GDK_VoidSymbol)) {
        gchar* c = sp_shortcut_get_label(shortcut);

        GtkWidget *hb = gtk_hbox_new(FALSE, 16);

        {
            GtkWidget *l = gtk_label_new_with_mnemonic(action ? action->name : label);
            gtk_misc_set_alignment((GtkMisc *) l, 0.0, 0.5);
            gtk_box_pack_start((GtkBox *) hb, l, TRUE, TRUE, 0);
        }

        {
            GtkWidget *l = gtk_label_new(c);
            gtk_misc_set_alignment((GtkMisc *) l, 1.0, 0.5);
            gtk_box_pack_end((GtkBox *) hb, l, FALSE, FALSE, 0);
        }

        gtk_widget_show_all(hb);

        gtk_container_add((GtkContainer *) item, hb);
        g_free(c);
    } else {
        GtkWidget *l = gtk_label_new_with_mnemonic(action ? action->name : label);
        gtk_misc_set_alignment((GtkMisc *) l, 0.0, 0.5);
        gtk_container_add((GtkContainer *) item, l);
    }
#if 0
    nr_active_object_add_listener((NRActiveObject *)action, (NRObjectEventVector *)&menu_item_event_vector, sizeof(SPActionEventVector), item);
    if (!action->sensitive) {
        gtk_widget_set_sensitive(item, FALSE);
    }
#endif
    gtk_widget_show(item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    g_object_set_data(G_OBJECT(item), "view", (gpointer) view);

    g_signal_connect( G_OBJECT(item), "toggled", (GCallback) callback_toggle, (void *) pref);
    g_signal_connect( G_OBJECT(item), "expose_event", (GCallback) callback_update, (void *) pref);

    g_signal_connect( G_OBJECT(item), "select", G_CALLBACK(sp_ui_menu_select), (gpointer) (action ? action->tip : tip));
    g_signal_connect( G_OBJECT(item), "deselect", G_CALLBACK(sp_ui_menu_deselect), NULL);
}

static void
sp_recent_open(GtkRecentChooser *recent_menu, gpointer /*user_data*/)
{
    // dealing with the bizarre filename convention in Inkscape for now
    gchar *uri = gtk_recent_chooser_get_current_uri(GTK_RECENT_CHOOSER(recent_menu));
    gchar *local_fn = g_filename_from_uri(uri, NULL, NULL);
    gchar *utf8_fn = g_filename_to_utf8(local_fn, -1, NULL, NULL, NULL);
    sp_file_open(utf8_fn, NULL);
    g_free(utf8_fn);
    g_free(local_fn);
    g_free(uri);
}

static void
sp_file_new_from_template(GtkWidget */*widget*/, gchar const *uri)
{
    sp_file_new(uri);
}

void
sp_menu_append_new_templates(GtkWidget *menu, Inkscape::UI::View::View *view)
{
    std::list<gchar *> sources;
    sources.push_back( profile_path("templates") ); // first try user's local dir
    sources.push_back( g_strdup(INKSCAPE_TEMPLATESDIR) ); // then the system templates dir

    // Use this loop to iterate through a list of possible document locations.
    while (!sources.empty()) {
        gchar *dirname = sources.front();

        if ( Inkscape::IO::file_test( dirname, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR) ) ) {
            GError *err = 0;
            GDir *dir = g_dir_open(dirname, 0, &err);

            if (dir) {
                for (gchar const *file = g_dir_read_name(dir); file != NULL; file = g_dir_read_name(dir)) {
                    if (!g_str_has_suffix(file, ".svg") && !g_str_has_suffix(file, ".svgz"))
                        continue; // skip non-svg files

                    gchar *basename = g_path_get_basename(file);
                    if (g_str_has_suffix(basename, ".svg") && g_str_has_prefix(basename, "default."))
                        continue; // skip default.*.svg (i.e. default.svg and translations) - it's in the menu already

                    gchar const *filepath = g_build_filename(dirname, file, NULL);
                    gchar *dupfile = g_strndup(file, strlen(file) - 4);
                    gchar *filename =  g_filename_to_utf8(dupfile,  -1, NULL, NULL, NULL);
                    g_free(dupfile);
                    GtkWidget *item = gtk_menu_item_new_with_label(filename);
                    g_free(filename);

                    gtk_widget_show(item);
                    // how does "filepath" ever get freed?
                    g_signal_connect(G_OBJECT(item),
                                     "activate",
                                     G_CALLBACK(sp_file_new_from_template),
                                     (gpointer) filepath);

                    if (view) {
                        // set null tip for now; later use a description from the template file
                        g_object_set_data(G_OBJECT(item), "view", (gpointer) view);
                        g_signal_connect( G_OBJECT(item), "select", G_CALLBACK(sp_ui_menu_select), (gpointer) NULL );
                        g_signal_connect( G_OBJECT(item), "deselect", G_CALLBACK(sp_ui_menu_deselect), NULL);
                    }

                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
                }
                g_dir_close(dir);
            }
        }

        // toss the dirname
        g_free(dirname);
        sources.pop_front();
    }
}

void
sp_ui_checkboxes_menus(GtkMenu *m, Inkscape::UI::View::View *view)
{
    //sp_ui_menu_append_check_item_from_verb(m, view, _("_Menu"), _("Show or hide the menu bar"), "menu",
    //                                       checkitem_toggled, checkitem_update, 0);
    sp_ui_menu_append_check_item_from_verb(m, view, _("Commands Bar"), _("Show or hide the Commands bar (under the menu)"), "commands",
                                           checkitem_toggled, checkitem_update, 0);
    sp_ui_menu_append_check_item_from_verb(m, view, _("Snap Controls Bar"), _("Show or hide the snapping controls"), "snaptoolbox",
                                           checkitem_toggled, checkitem_update, 0);
    sp_ui_menu_append_check_item_from_verb(m, view, _("Tool Controls Bar"), _("Show or hide the Tool Controls bar"), "toppanel",
                                           checkitem_toggled, checkitem_update, 0);
    sp_ui_menu_append_check_item_from_verb(m, view, _("_Toolbox"), _("Show or hide the main toolbox (on the left)"), "toolbox",
                                           checkitem_toggled, checkitem_update, 0);
    sp_ui_menu_append_check_item_from_verb(m, view, NULL, NULL, "rulers",
                                           checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_RULERS));
    sp_ui_menu_append_check_item_from_verb(m, view, NULL, NULL, "scrollbars",
                                           checkitem_toggled, checkitem_update, Inkscape::Verb::get(SP_VERB_TOGGLE_SCROLLBARS));
    sp_ui_menu_append_check_item_from_verb(m, view, _("_Palette"), _("Show or hide the color palette"), "panels",
                                           checkitem_toggled, checkitem_update, 0);
    sp_ui_menu_append_check_item_from_verb(m, view, _("_Statusbar"), _("Show or hide the statusbar (at the bottom of the window)"), "statusbar",
                                           checkitem_toggled, checkitem_update, 0);
}


void addTaskMenuItems(GtkMenu *menu, Inkscape::UI::View::View *view)
{
    gchar const* data[] = {
        _("Default"), _("Default interface setup"),
        _("Custom"), _("Set the custom task"),
        _("Wide"), _("Setup for widescreen work"),
        0, 0
    };

    GSList *group = 0;
    int count = 0;
    gint active = Inkscape::UI::UXManager::getInstance()->getDefaultTask( dynamic_cast<SPDesktop*>(view) );
    for (gchar const **strs = data; strs[0]; strs += 2, count++)
    {
        GtkWidget *item = gtk_radio_menu_item_new_with_label( group, strs[0] );
        group = gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM(item) );
        if ( count == active )
        {
            gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(item), TRUE );
        }

        g_object_set_data( G_OBJECT(item), "view", view );
        g_signal_connect( G_OBJECT(item), "toggled", reinterpret_cast<GCallback>(taskToggled), GINT_TO_POINTER(count) );
        g_signal_connect( G_OBJECT(item), "select", G_CALLBACK(sp_ui_menu_select), const_cast<gchar*>(strs[1]) );
        g_signal_connect( G_OBJECT(item), "deselect", G_CALLBACK(sp_ui_menu_deselect), 0 );

        gtk_widget_show( item );
        gtk_menu_shell_append( GTK_MENU_SHELL(menu), item );
    }
}


/** @brief Observer that updates the recent list's max document count */
class MaxRecentObserver : public Inkscape::Preferences::Observer {
public:
    MaxRecentObserver(GtkWidget *recent_menu) :
        Observer("/options/maxrecentdocuments/value"),
        _rm(recent_menu)
    {}
    virtual void notify(Inkscape::Preferences::Entry const &e) {
        gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER(_rm), e.getInt());
        // hack: the recent menu doesn't repopulate after changing the limit, so we force it
        g_signal_emit_by_name((gpointer) gtk_recent_manager_get_default(), "changed");
    }
private:
    GtkWidget *_rm;
};

/** \brief  This function turns XML into a menu
    \param  menus  This is the XML that defines the menu
    \param  menu   Menu to be added to
    \param  view   The View that this menu is being built for

    This function is realitively simple as it just goes through the XML
    and parses the individual elements.  In the case of a submenu, it
    just calls itself recursively.  Because it is only reasonable to have
    a couple of submenus, it is unlikely this will go more than two or
    three times.

    In the case of an unrecognized verb, a menu item is made to identify
    the verb that is missing, and display that.  The menu item is also made
    insensitive.
*/
void
sp_ui_build_dyn_menus(Inkscape::XML::Node *menus, GtkWidget *menu, Inkscape::UI::View::View *view)
{
    if (menus == NULL) return;
    if (menu == NULL)  return;
    GSList *group = NULL;

    for (Inkscape::XML::Node *menu_pntr = menus;
         menu_pntr != NULL;
         menu_pntr = menu_pntr->next()) {
        if (!strcmp(menu_pntr->name(), "submenu")) {
            GtkWidget *mitem = gtk_menu_item_new_with_mnemonic(_(menu_pntr->attribute("name")));
            GtkWidget *submenu = gtk_menu_new();
            sp_ui_build_dyn_menus(menu_pntr->firstChild(), submenu, view);
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem), GTK_WIDGET(submenu));
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), mitem);
            continue;
        }
        if (!strcmp(menu_pntr->name(), "verb")) {
            gchar const *verb_name = menu_pntr->attribute("verb-id");
            Inkscape::Verb *verb = Inkscape::Verb::getbyid(verb_name);

            if (verb != NULL) {
                if (menu_pntr->attribute("radio") != NULL) {
                    GtkWidget *item = sp_ui_menu_append_item_from_verb (GTK_MENU(menu), verb, view, true, group);
                    group = gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM(item));
                    if (menu_pntr->attribute("default") != NULL) {
                        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
                    }
                    if (verb->get_code() != SP_VERB_NONE) {
                    	SPAction *action = verb->get_action(view);
                    	g_signal_connect( G_OBJECT(item), "expose_event", (GCallback) update_view_menu, (void *) action);
                    }
                } else {
                    sp_ui_menu_append_item_from_verb(GTK_MENU(menu), verb, view);
                    group = NULL;
                }
            } else {
                gchar string[120];
                g_snprintf(string, 120, _("Verb \"%s\" Unknown"), verb_name);
                string[119] = '\0'; /* may not be terminated */
                GtkWidget *item = gtk_menu_item_new_with_label(string);
                gtk_widget_set_sensitive(item, false);
                gtk_widget_show(item);
                gtk_menu_append(GTK_MENU(menu), item);
            }
            continue;
        }
        if (!strcmp(menu_pntr->name(), "separator")
                // This was spelt wrong in the original version
                // and so this is for backward compatibility.  It can
                // probably be dropped after the 0.44 release.
             || !strcmp(menu_pntr->name(), "seperator")) {
            GtkWidget *item = gtk_separator_menu_item_new();
            gtk_widget_show(item);
            gtk_menu_append(GTK_MENU(menu), item);
            continue;
        }
        if (!strcmp(menu_pntr->name(), "template-list")) {
            sp_menu_append_new_templates(menu, view);
            continue;
        }
        if (!strcmp(menu_pntr->name(), "recent-file-list")) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();

            // create recent files menu
            int max_recent = prefs->getInt("/options/maxrecentdocuments/value");
            GtkWidget *recent_menu = gtk_recent_chooser_menu_new_for_manager(gtk_recent_manager_get_default());
            gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER(recent_menu), max_recent);
            // sort most recently used documents first to preserve previous behavior
            gtk_recent_chooser_set_sort_type(GTK_RECENT_CHOOSER(recent_menu), GTK_RECENT_SORT_MRU);
            g_signal_connect(G_OBJECT(recent_menu), "item-activated", G_CALLBACK(sp_recent_open), (gpointer) NULL);

            // add filter to only open files added by Inkscape
            GtkRecentFilter *inkscape_only_filter = gtk_recent_filter_new();
            gtk_recent_filter_add_application(inkscape_only_filter, g_get_prgname());
            gtk_recent_chooser_add_filter(GTK_RECENT_CHOOSER(recent_menu), inkscape_only_filter);

            gtk_recent_chooser_set_show_tips (GTK_RECENT_CHOOSER(recent_menu), TRUE);
            gtk_recent_chooser_set_show_not_found (GTK_RECENT_CHOOSER(recent_menu), FALSE);

            GtkWidget *recent_item = gtk_menu_item_new_with_mnemonic(_("Open _Recent"));
            gtk_menu_item_set_submenu(GTK_MENU_ITEM(recent_item), recent_menu);

            gtk_menu_append(GTK_MENU(menu), GTK_WIDGET(recent_item));
            // this will just sit and update the list's item count
            static MaxRecentObserver *mro = new MaxRecentObserver(recent_menu);
            prefs->addObserver(*mro);
            continue;
        }
        if (!strcmp(menu_pntr->name(), "objects-checkboxes")) {
            sp_ui_checkboxes_menus(GTK_MENU(menu), view);
            continue;
        }
        if (!strcmp(menu_pntr->name(), "task-checkboxes")) {
            addTaskMenuItems(GTK_MENU(menu), view);
            continue;
        }
    }
}

/** \brief  Build the main tool bar
    \param  view  View to build the bar for

    Currently the main tool bar is built as a dynamic XML menu using
    \c sp_ui_build_dyn_menus.  This function builds the bar, and then
    pass it to get items attached to it.
*/
GtkWidget *
sp_ui_main_menubar(Inkscape::UI::View::View *view)
{
    GtkWidget *mbar = gtk_menu_bar_new();
    sp_ui_build_dyn_menus(inkscape_get_menus(INKSCAPE), mbar, view);
    return mbar;
}

static void leave_group(GtkMenuItem *, SPDesktop *desktop) {
    desktop->setCurrentLayer(SP_OBJECT_PARENT(desktop->currentLayer()));
}

static void enter_group(GtkMenuItem *mi, SPDesktop *desktop) {
    desktop->setCurrentLayer(reinterpret_cast<SPObject *>(g_object_get_data(G_OBJECT(mi), "group")));
    sp_desktop_selection(desktop)->clear();
}

GtkWidget *
sp_ui_context_menu(Inkscape::UI::View::View *view, SPItem *item)
{
    GtkWidget *m;
    SPDesktop *dt;

    dt = static_cast<SPDesktop*>(view);

    m = gtk_menu_new();

    /* Undo and Redo */
    sp_ui_menu_append_item_from_verb(GTK_MENU(m), Inkscape::Verb::get(SP_VERB_EDIT_UNDO), view);
    sp_ui_menu_append_item_from_verb(GTK_MENU(m), Inkscape::Verb::get(SP_VERB_EDIT_REDO), view);

    /* Separator */
    sp_ui_menu_append_item(GTK_MENU(m), NULL, NULL, NULL, NULL, NULL, NULL);

    sp_ui_menu_append_item_from_verb(GTK_MENU(m), Inkscape::Verb::get(SP_VERB_EDIT_CUT), view);
    sp_ui_menu_append_item_from_verb(GTK_MENU(m), Inkscape::Verb::get(SP_VERB_EDIT_COPY), view);
    sp_ui_menu_append_item_from_verb(GTK_MENU(m), Inkscape::Verb::get(SP_VERB_EDIT_PASTE), view);

    /* Separator */
    sp_ui_menu_append_item(GTK_MENU(m), NULL, NULL, NULL, NULL, NULL, NULL);

    sp_ui_menu_append_item_from_verb(GTK_MENU(m), Inkscape::Verb::get(SP_VERB_EDIT_DUPLICATE), view);
    sp_ui_menu_append_item_from_verb(GTK_MENU(m), Inkscape::Verb::get(SP_VERB_EDIT_DELETE), view);

    /* Item menu */
    if (item) {
        sp_ui_menu_append_item(GTK_MENU(m), NULL, NULL, NULL, NULL, NULL, NULL);
        sp_object_menu((SPObject *) item, dt, GTK_MENU(m));
    }

    /* layer menu */
    SPGroup *group=NULL;
    if (item) {
        if (SP_IS_GROUP(item)) {
            group = SP_GROUP(item);
        } else if ( item != dt->currentRoot() && SP_IS_GROUP(SP_OBJECT_PARENT(item)) ) {
            group = SP_GROUP(SP_OBJECT_PARENT(item));
        }
    }

    if (( group && group != dt->currentLayer() ) ||
        ( dt->currentLayer() != dt->currentRoot() && SP_OBJECT_PARENT(dt->currentLayer()) != dt->currentRoot() ) ) {
        /* Separator */
        sp_ui_menu_append_item(GTK_MENU(m), NULL, NULL, NULL, NULL, NULL, NULL);
    }

    if ( group && group != dt->currentLayer() ) {
        /* TRANSLATORS: #%s is the id of the group e.g. <g id="#g7">, not a number. */
        gchar *label=g_strdup_printf(_("Enter group #%s"), group->getId());
        GtkWidget *w = gtk_menu_item_new_with_label(label);
        g_free(label);
        g_object_set_data(G_OBJECT(w), "group", group);
        g_signal_connect(G_OBJECT(w), "activate", GCallback(enter_group), dt);
        gtk_widget_show(w);
        gtk_menu_shell_append(GTK_MENU_SHELL(m), w);
    }

    if ( dt->currentLayer() != dt->currentRoot() ) {
        if ( SP_OBJECT_PARENT(dt->currentLayer()) != dt->currentRoot() ) {
            GtkWidget *w = gtk_menu_item_new_with_label(_("Go to parent"));
            g_signal_connect(G_OBJECT(w), "activate", GCallback(leave_group), dt);
            gtk_widget_show(w);
            gtk_menu_shell_append(GTK_MENU_SHELL(m), w);

        }
    }

    return m;
}

/* Drag and Drop */
void
sp_ui_drag_data_received(GtkWidget *widget,
                         GdkDragContext *drag_context,
                         gint x, gint y,
                         GtkSelectionData *data,
                         guint info,
                         guint /*event_time*/,
                         gpointer /*user_data*/)
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    switch (info) {
#if ENABLE_MAGIC_COLORS
        case APP_X_INKY_COLOR:
        {
            int destX = 0;
            int destY = 0;
            gtk_widget_translate_coordinates( widget, &(desktop->canvas->widget), x, y, &destX, &destY );
            Geom::Point where( sp_canvas_window_to_world( desktop->canvas, Geom::Point( destX, destY ) ) );

            SPItem *item = desktop->item_at_point( where, true );
            if ( item )
            {
                bool fillnotstroke = (drag_context->action != GDK_ACTION_MOVE);

                if ( data->length >= 8 ) {
                    cmsHPROFILE srgbProf = cmsCreate_sRGBProfile();

                    gchar c[64] = {0};
                    // Careful about endian issues.
                    guint16* dataVals = (guint16*)data->data;
                    sp_svg_write_color( c, sizeof(c),
                                        SP_RGBA32_U_COMPOSE(
                                            0x0ff & (dataVals[0] >> 8),
                                            0x0ff & (dataVals[1] >> 8),
                                            0x0ff & (dataVals[2] >> 8),
                                            0xff // can't have transparency in the color itself
                                            //0x0ff & (data->data[3] >> 8),
                                            ));
                    SPCSSAttr *css = sp_repr_css_attr_new();
                    bool updatePerformed = false;

                    if ( data->length > 14 ) {
                        int flags = dataVals[4];

                        // piggie-backed palette entry info
                        int index = dataVals[5];
                        Glib::ustring palName;
                        for ( int i = 0; i < dataVals[6]; i++ ) {
                            palName += (gunichar)dataVals[7+i];
                        }

                        // Now hook in a magic tag of some sort.
                        if ( !palName.empty() && (flags & 1) ) {
                            gchar* str = g_strdup_printf("%d|", index);
                            palName.insert( 0, str );
                            g_free(str);
                            str = 0;

                            sp_object_setAttribute( SP_OBJECT(item),
                                                    fillnotstroke ? "inkscape:x-fill-tag":"inkscape:x-stroke-tag",
                                                    palName.c_str(),
                                                    NULL );
                            item->updateRepr();

                            sp_repr_css_set_property( css, fillnotstroke ? "fill":"stroke", c );
                            updatePerformed = true;
                        }
                    }

                    if ( !updatePerformed ) {
                        sp_repr_css_set_property( css, fillnotstroke ? "fill":"stroke", c );
                    }

                    sp_desktop_apply_css_recursive( item, css, true );
                    item->updateRepr();

                    sp_document_done( doc , SP_VERB_NONE,
                                      _("Drop color"));

                    if ( srgbProf ) {
                        cmsCloseProfile( srgbProf );
                    }
                }
            }
        }
        break;
#endif // ENABLE_MAGIC_COLORS

        case APP_X_COLOR:
        {
            int destX = 0;
            int destY = 0;
            gtk_widget_translate_coordinates( widget, SP_GTK_CANVAS_WIDGET(desktop->canvas), x, y, &destX, &destY );
            Geom::Point where( desktop->canvas->window_to_world(Geom::Point( destX, destY ) ) );
            Geom::Point const button_dt(desktop->w2d(where));
            Geom::Point const button_doc(desktop->dt2doc(button_dt));

            if ( data->length == 8 ) {
                gchar colorspec[64] = {0};
                // Careful about endian issues.
                guint16* dataVals = (guint16*)data->data;
                sp_svg_write_color( colorspec, sizeof(colorspec),
                                    SP_RGBA32_U_COMPOSE(
                                        0x0ff & (dataVals[0] >> 8),
                                        0x0ff & (dataVals[1] >> 8),
                                        0x0ff & (dataVals[2] >> 8),
                                        0xff // can't have transparency in the color itself
                                        //0x0ff & (data->data[3] >> 8),
                                        ));

                SPItem *item = desktop->item_at_point( where, true );

                bool consumed = false;
                if (desktop->event_context && desktop->event_context->get_drag()) {
                    consumed = desktop->event_context->get_drag()->dropColor(item, colorspec, button_dt);
                    if (consumed) {
                        sp_document_done( doc , SP_VERB_NONE, _("Drop color on gradient"));
                        desktop->event_context->get_drag()->updateDraggers();
                    }
                }

                //if (!consumed && tools_active(desktop, TOOLS_TEXT)) {
                //    consumed = sp_text_context_drop_color(c, button_doc);
                //    if (consumed) {
                //        sp_document_done( doc , SP_VERB_NONE, _("Drop color on gradient stop"));
                //    }
                //}

                if (!consumed && item) {
                    bool fillnotstroke = (drag_context->action != GDK_ACTION_MOVE);
                    if (fillnotstroke &&
                        (SP_IS_SHAPE(item) || SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item))) {
                        Path *livarot_path = Path_for_item(item, true, true);
                        livarot_path->ConvertWithBackData(0.04);

                        boost::optional<Path::cut_position> position = get_nearest_position_on_Path(livarot_path, button_doc);
                        if (position) {
                            Geom::Point nearest = get_point_on_Path(livarot_path, position->piece, position->t);
                            Geom::Point delta = nearest - button_doc;
                            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                            delta = desktop->d2w(delta);
                            double stroke_tolerance =
                                ( !SP_OBJECT_STYLE(item)->stroke.isNone() ?
                                  desktop->current_zoom() *
                                  SP_OBJECT_STYLE (item)->stroke_width.computed *
                                  to_2geom(sp_item_i2d_affine(item)).descrim() * 0.5
                                  : 0.0)
                                + prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

                            if (Geom::L2 (delta) < stroke_tolerance) {
                                fillnotstroke = false;
                            }
                        }
                        delete livarot_path;
                    }

                    SPCSSAttr *css = sp_repr_css_attr_new();
                    sp_repr_css_set_property( css, fillnotstroke ? "fill":"stroke", colorspec );

                    sp_desktop_apply_css_recursive( item, css, true );
                    item->updateRepr();

                    sp_document_done( doc , SP_VERB_NONE,
                                      _("Drop color"));
                }
            }
        }
        break;

        case APP_OSWB_COLOR:
        {
            bool worked = false;
            Glib::ustring colorspec;
            if ( data->format == 8 ) {
                ege::PaintDef color;
                worked = color.fromMIMEData("application/x-oswb-color",
                                            reinterpret_cast<char*>(data->data),
                                            data->length,
                                            data->format);
                if ( worked ) {
                    if ( color.getType() == ege::PaintDef::CLEAR ) {
                        colorspec = ""; // TODO check if this is sufficient
                    } else if ( color.getType() == ege::PaintDef::NONE ) {
                        colorspec = "none";
                    } else {
                        unsigned int r = color.getR();
                        unsigned int g = color.getG();
                        unsigned int b = color.getB();

                        SPGradient* matches = 0;
                        const GSList *gradients = sp_document_get_resource_list(doc, "gradient");
                        for (const GSList *item = gradients; item; item = item->next) {
                            SPGradient* grad = SP_GRADIENT(item->data);
                            if ( color.descr == grad->getId() ) {
                                if ( grad->hasStops() ) {
                                    matches = grad;
                                    break;
                                }
                            }
                        }
                        if (matches) {
                            colorspec = "url(#";
                            colorspec += matches->getId();
                            colorspec += ")";
                        } else {
                            gchar* tmp = g_strdup_printf("#%02x%02x%02x", r, g, b);
                            colorspec = tmp;
                            g_free(tmp);
                        }
                    }
                }
            }
            if ( worked ) {
                int destX = 0;
                int destY = 0;
                gtk_widget_translate_coordinates( widget, SP_GTK_CANVAS_WIDGET(desktop->canvas), x, y, &destX, &destY );
                Geom::Point where( desktop->canvas->window_to_world(Geom::Point( destX, destY ) ) );
                Geom::Point const button_dt(desktop->w2d(where));
                Geom::Point const button_doc(desktop->dt2doc(button_dt));

                SPItem *item = desktop->item_at_point( where, true );

                bool consumed = false;
                if (desktop->event_context && desktop->event_context->get_drag()) {
                    consumed = desktop->event_context->get_drag()->dropColor(item, colorspec.c_str(), button_dt);
                    if (consumed) {
                        sp_document_done( doc , SP_VERB_NONE, _("Drop color on gradient"));
                        desktop->event_context->get_drag()->updateDraggers();
                    }
                }

                if (!consumed && item) {
                    bool fillnotstroke = (drag_context->action != GDK_ACTION_MOVE);
                    if (fillnotstroke &&
                        (SP_IS_SHAPE(item) || SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item))) {
                        Path *livarot_path = Path_for_item(item, true, true);
                        livarot_path->ConvertWithBackData(0.04);

                        boost::optional<Path::cut_position> position = get_nearest_position_on_Path(livarot_path, button_doc);
                        if (position) {
                            Geom::Point nearest = get_point_on_Path(livarot_path, position->piece, position->t);
                            Geom::Point delta = nearest - button_doc;
                            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                            delta = desktop->d2w(delta);
                            double stroke_tolerance =
                                ( !SP_OBJECT_STYLE(item)->stroke.isNone() ?
                                  desktop->current_zoom() *
                                  SP_OBJECT_STYLE (item)->stroke_width.computed *
                                  to_2geom(sp_item_i2d_affine(item)).descrim() * 0.5
                                  : 0.0)
                                + prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

                            if (Geom::L2 (delta) < stroke_tolerance) {
                                fillnotstroke = false;
                            }
                        }
                        delete livarot_path;
                    }

                    SPCSSAttr *css = sp_repr_css_attr_new();
                    sp_repr_css_set_property( css, fillnotstroke ? "fill":"stroke", colorspec.c_str() );

                    sp_desktop_apply_css_recursive( item, css, true );
                    item->updateRepr();

                    sp_document_done( doc , SP_VERB_NONE,
                                      _("Drop color"));
                }
            }
        }
        break;

        case SVG_DATA:
        case SVG_XML_DATA: {
            gchar *svgdata = (gchar *)data->data;

            Inkscape::XML::Document *rnewdoc = sp_repr_read_mem(svgdata, data->length, SP_SVG_NS_URI);

            if (rnewdoc == NULL) {
                sp_ui_error_dialog(_("Could not parse SVG data"));
                return;
            }

            Inkscape::XML::Node *repr = rnewdoc->root();
            gchar const *style = repr->attribute("style");

            Inkscape::XML::Node *newgroup = rnewdoc->createElement("svg:g");
            newgroup->setAttribute("style", style);

            Inkscape::XML::Document * xml_doc =  sp_document_repr_doc(doc);
            for (Inkscape::XML::Node *child = repr->firstChild(); child != NULL; child = child->next()) {
                Inkscape::XML::Node *newchild = child->duplicate(xml_doc);
                newgroup->appendChild(newchild);
            }

            Inkscape::GC::release(rnewdoc);

            // Add it to the current layer

            // Greg's edits to add intelligent positioning of svg drops
            SPObject *new_obj = NULL;
            new_obj = desktop->currentLayer()->appendChildRepr(newgroup);

            Inkscape::Selection *selection = sp_desktop_selection(desktop);
            selection->set(SP_ITEM(new_obj));

            // move to mouse pointer
            {
                sp_document_ensure_up_to_date(sp_desktop_document(desktop));
                Geom::OptRect sel_bbox = selection->bounds();
                if (sel_bbox) {
                    Geom::Point m( desktop->point() - sel_bbox->midpoint() );
                    sp_selection_move_relative(selection, m, false);
                }
            }

            Inkscape::GC::release(newgroup);
            sp_document_done(doc, SP_VERB_NONE,
                             _("Drop SVG"));
            break;
        }

        case URI_LIST: {
            gchar *uri = (gchar *)data->data;
            sp_ui_import_files(uri);
            break;
        }

        case PNG_DATA:
        case JPEG_DATA:
        case IMAGE_DATA: {
            const char *mime = (info == JPEG_DATA ? "image/jpeg" : "image/png");

            Inkscape::Extension::DB::InputList o;
            Inkscape::Extension::db.get_input_list(o);
            Inkscape::Extension::DB::InputList::const_iterator i = o.begin();
            while (i != o.end() && strcmp( (*i)->get_mimetype(), mime ) != 0) {
                ++i;
            }
            Inkscape::Extension::Extension *ext = *i;
            bool save = (strcmp(ext->get_param_optiongroup("link"), "embed") == 0);
            ext->set_param_optiongroup("link", "embed");
            ext->set_gui(false);

            gchar *filename = g_build_filename( g_get_tmp_dir(), "inkscape-dnd-import", NULL );
            g_file_set_contents(filename, reinterpret_cast<gchar const *>(data->data), data->length, NULL);
            file_import(doc, filename, ext);
            g_free(filename);

            ext->set_param_optiongroup("link", save ? "embed" : "link");
            ext->set_gui(true);
            sp_document_done( doc , SP_VERB_NONE,
                              _("Drop bitmap image"));
            break;
        }
    }
}

#include "gradient-context.h"

void sp_ui_drag_motion( GtkWidget */*widget*/,
                        GdkDragContext */*drag_context*/,
                        gint /*x*/, gint /*y*/,
                        GtkSelectionData */*data*/,
                        guint /*info*/,
                        guint /*event_time*/,
                        gpointer /*user_data*/)
{
//     SPDocument *doc = SP_ACTIVE_DOCUMENT;
//     SPDesktop *desktop = SP_ACTIVE_DESKTOP;


//     g_message("drag-n-drop motion (%4d, %4d)  at %d", x, y, event_time);
}

static void sp_ui_drag_leave( GtkWidget */*widget*/,
                              GdkDragContext */*drag_context*/,
                              guint /*event_time*/,
                              gpointer /*user_data*/ )
{
//     g_message("drag-n-drop leave                at %d", event_time);
}

static void
sp_ui_import_files(gchar *buffer)
{
    GList *list = gnome_uri_list_extract_filenames(buffer);
    if (!list)
        return;
    g_list_foreach(list, sp_ui_import_one_file_with_check, NULL);
    g_list_foreach(list, (GFunc) g_free, NULL);
    g_list_free(list);
}

static void
sp_ui_import_one_file_with_check(gpointer filename, gpointer /*unused*/)
{
    if (filename) {
        if (strlen((char const *)filename) > 2)
            sp_ui_import_one_file((char const *)filename);
    }
}

static void
sp_ui_import_one_file(char const *filename)
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (!doc) return;

    if (filename == NULL) return;

    // Pass off to common implementation
    // TODO might need to get the proper type of Inkscape::Extension::Extension
    file_import( doc, filename, NULL );
}

void
sp_ui_error_dialog(gchar const *message)
{
    GtkWidget *dlg;
    gchar *safeMsg = Inkscape::IO::sanitizeString(message);

    dlg = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
                                 GTK_BUTTONS_CLOSE, "%s", safeMsg);
    sp_transientize(dlg);
    gtk_window_set_resizable(GTK_WINDOW(dlg), FALSE);
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
    g_free(safeMsg);
}

bool
sp_ui_overwrite_file(gchar const *filename)
{
    bool return_value = FALSE;

    if (Inkscape::IO::file_test(filename, G_FILE_TEST_EXISTS)) {
        Gtk::Window *window = SP_ACTIVE_DESKTOP->getToplevel();
        gchar* baseName = g_path_get_basename( filename );
        gchar* dirName = g_path_get_dirname( filename );
        GtkWidget* dialog = gtk_message_dialog_new_with_markup( window->gobj(),
                                                                (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                                                GTK_MESSAGE_QUESTION,
                                                                GTK_BUTTONS_NONE,
                                                                _( "<span weight=\"bold\" size=\"larger\">A file named \"%s\" already exists. Do you want to replace it?</span>\n\n"
                                                                   "The file already exists in \"%s\". Replacing it will overwrite its contents." ),
                                                                baseName,
                                                                dirName
            );
        gtk_dialog_add_buttons( GTK_DIALOG(dialog),
                                GTK_STOCK_CANCEL, GTK_RESPONSE_NO,
                                _("Replace"), GTK_RESPONSE_YES,
                                NULL );
        gtk_dialog_set_default_response( GTK_DIALOG(dialog), GTK_RESPONSE_YES );

        if ( gtk_dialog_run( GTK_DIALOG(dialog) ) == GTK_RESPONSE_YES ) {
            return_value = TRUE;
        } else {
            return_value = FALSE;
        }
        gtk_widget_destroy(dialog);
        g_free( baseName );
        g_free( dirName );
    } else {
        return_value = TRUE;
    }

    return return_value;
}

static void
sp_ui_menu_item_set_sensitive(SPAction */*action*/, unsigned int sensitive, void *data)
{
    return gtk_widget_set_sensitive(GTK_WIDGET(data), sensitive);
}

static void
sp_ui_menu_item_set_name(SPAction */*action*/, Glib::ustring name, void *data)
{
    void *child = GTK_BIN (data)->child;
    //child is either
    //- a GtkHBox, whose first child is a label displaying name if the menu
    //item has an accel key
    //- a GtkLabel if the menu has no accel key
    if (child != NULL){
        if (GTK_IS_LABEL(child)) {
            gtk_label_set_markup_with_mnemonic(GTK_LABEL (child), name.c_str());
        } else if (GTK_IS_HBOX(child)) {
            gtk_label_set_markup_with_mnemonic(
            GTK_LABEL (gtk_container_get_children(GTK_CONTAINER (child))->data),
            name.c_str());
        }//else sp_ui_menu_append_item_from_verb has been modified and can set
        //a menu item in yet another way...
    }
}

void injectRenamedIcons()
{
    Glib::RefPtr<Gtk::IconTheme> iconTheme = Gtk::IconTheme::get_default();

    std::vector< std::pair<Glib::ustring, Glib::ustring> > renamed;
    renamed.push_back(std::make_pair("gtk-file", "document-x-generic"));
    renamed.push_back(std::make_pair("gtk-directory", "folder"));

    for ( std::vector< std::pair<Glib::ustring, Glib::ustring> >::iterator it = renamed.begin(); it < renamed.end(); ++it ) {
        bool hasIcon = iconTheme->has_icon(it->first);
        bool hasSecondIcon = iconTheme->has_icon(it->second);

        if ( !hasIcon && hasSecondIcon ) {
            Glib::ArrayHandle<int> sizes = iconTheme->get_icon_sizes(it->second);
            for ( Glib::ArrayHandle<int>::iterator it2 = sizes.begin(); it2 < sizes.end(); ++it2 ) {
                Glib::RefPtr<Gdk::Pixbuf> pb = iconTheme->load_icon( it->second, *it2 );
                if ( pb ) {
                    // install a private copy of the pixbuf to avoid pinning a theme
                    Glib::RefPtr<Gdk::Pixbuf> pbCopy = pb->copy();
                    Gtk::IconTheme::add_builtin_icon( it->first, *it2, pbCopy );
                }
            }
        }
    }
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
