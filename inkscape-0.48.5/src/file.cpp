/** @file
 * @brief File/Print operations
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Chema Celorio <chema@celorio.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Bruno Dilly <bruno.dilly@gmail.com>
 *   Stephen Silver <sasilver@users.sourceforge.net>
 *   David Xiong
 *
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 1999-2012 Authors
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/** @file
 * @note This file needs to be cleaned up extensively.
 * What it probably needs is to have one .h file for
 * the API, and two or more .cpp files for the implementations.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtk/gtk.h>
#include <glib.h>
#include <glibmm/i18n.h>
#include <libnr/nr-pixops.h>

#include "application/application.h"
#include "application/editor.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "dialogs/export.h"
#include "dir-util.h"
#include "document-private.h"
#include "extension/db.h"
#include "extension/input.h"
#include "extension/output.h"
#include "extension/system.h"
#include "file.h"
#include "helper/png-write.h"
#include "id-clash.h"
#include "inkscape.h"
#include "inkscape.h"
#include "interface.h"
#include "io/sys.h"
#include "message.h"
#include "message-stack.h"
#include "path-prefix.h"
#include "preferences.h"
#include "print.h"
#include "rdf.h"
#include "selection-chemistry.h"
#include "selection.h"
#include "sp-namedview.h"
#include "style.h"
#include "ui/dialog/ocaldialogs.h"
#include "ui/view/view-widget.h"
#include "uri.h"
#include "xml/rebase-hrefs.h"

#ifdef WITH_GNOME_VFS
# include <libgnomevfs/gnome-vfs.h>
#endif

//#ifdef WITH_INKBOARD
//#include "jabber_whiteboard/session-manager.h"
//#endif

#ifdef WIN32
#include <windows.h>
#endif

//#define INK_DUMP_FILENAME_CONV 1
#undef INK_DUMP_FILENAME_CONV

//#define INK_DUMP_FOPEN 1
#undef INK_DUMP_FOPEN

void dump_str(gchar const *str, gchar const *prefix);
void dump_ustr(Glib::ustring const &ustr);

// what gets passed here is not actually an URI... it is an UTF-8 encoded filename (!)
static void sp_file_add_recent(gchar const *uri)
{
    if(uri == NULL) {
        g_warning("sp_file_add_recent: uri == NULL");
        return;
    }
    GtkRecentManager *recent = gtk_recent_manager_get_default();
    gchar *fn = g_filename_from_utf8(uri, -1, NULL, NULL, NULL);
    if (fn) {
        gchar *uri_to_add = g_filename_to_uri(fn, NULL, NULL);
        if (uri_to_add) {
            gtk_recent_manager_add_item(recent, uri_to_add);
            g_free(uri_to_add);
        }
        g_free(fn);
    }
}


/*######################
## N E W
######################*/

/**
 * Create a blank document and add it to the desktop
 */
SPDesktop*
sp_file_new(const Glib::ustring &templ)
{
    char *templName = NULL;
    if (templ.size()>0)
        templName = (char *)templ.c_str();
    SPDocument *doc = sp_document_new(templName, TRUE, true);
    g_return_val_if_fail(doc != NULL, NULL);

    SPDesktop *dt;
    if (Inkscape::NSApplication::Application::getNewGui())
    {
        dt = Inkscape::NSApplication::Editor::createDesktop (doc);
    } else {
        SPViewWidget *dtw = sp_desktop_widget_new(sp_document_namedview(doc, NULL));
        g_return_val_if_fail(dtw != NULL, NULL);
        sp_document_unref(doc);

        sp_create_window(dtw, TRUE);
        dt = static_cast<SPDesktop*>(dtw->view);
        sp_namedview_window_from_document(dt);
        sp_namedview_update_layers_from_document(dt);
    }
    return dt;
}

SPDesktop* sp_file_new_default()
{
    std::list<gchar *> sources;
    sources.push_back( profile_path("templates") ); // first try user's local dir
    sources.push_back( g_strdup(INKSCAPE_TEMPLATESDIR) ); // then the system templates dir
    std::list<gchar const*> baseNames;
    gchar const* localized = _("default.svg");
    if (strcmp("default.svg", localized) != 0) {
        baseNames.push_back(localized);
    }
    baseNames.push_back("default.svg");
    gchar *foundTemplate = 0;

    for (std::list<gchar const*>::iterator nameIt = baseNames.begin(); (nameIt != baseNames.end()) && !foundTemplate; ++nameIt) {
        for (std::list<gchar *>::iterator it = sources.begin(); (it != sources.end()) && !foundTemplate; ++it) {
            gchar *dirname = *it;
            if ( Inkscape::IO::file_test( dirname, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR) ) ) {

                // TRANSLATORS: default.svg is localizable - this is the name of the default document
                //  template. This way you can localize the default pagesize, translate the name of
                //  the default layer, etc. If you wish to localize this file, please create a
                //  localized share/templates/default.xx.svg file, where xx is your language code.
                char *tmp = g_build_filename(dirname, *nameIt, NULL);
                if (Inkscape::IO::file_test(tmp, G_FILE_TEST_IS_REGULAR)) {
                    foundTemplate = tmp;
                } else {
                    g_free(tmp);
                }
            }
        }
    }

    for (std::list<gchar *>::iterator it = sources.begin(); it != sources.end(); ++it) {
        g_free(*it);
    }

    SPDesktop* desk = sp_file_new(foundTemplate ? foundTemplate : "");
    if (foundTemplate) {
        g_free(foundTemplate);
        foundTemplate = 0;
    }
    return desk;
}


/*######################
## D E L E T E
######################*/

/**
 *  Perform document closures preceding an exit()
 */
void
sp_file_exit()
{
    sp_ui_close_all();
    // no need to call inkscape_exit here; last document being closed will take care of that
}


/*######################
## O P E N
######################*/

/**
 *  Open a file, add the document to the desktop
 *
 *  \param replace_empty if true, and the current desktop is empty, this document
 *  will replace the empty one.
 */
bool
sp_file_open(const Glib::ustring &uri,
             Inkscape::Extension::Extension *key,
             bool add_to_recent, bool replace_empty)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop)
        desktop->setWaitingCursor();

    SPDocument *doc = NULL;
    try {
        doc = Inkscape::Extension::open(key, uri.c_str());
    } catch (Inkscape::Extension::Input::no_extension_found &e) {
        doc = NULL;
    } catch (Inkscape::Extension::Input::open_failed &e) {
        doc = NULL;
    }

    if (desktop)
        desktop->clearWaitingCursor();

    if (doc) {
        SPDocument *existing = desktop ? sp_desktop_document(desktop) : NULL;

        if (existing && existing->virgin && replace_empty) {
            // If the current desktop is empty, open the document there
            sp_document_ensure_up_to_date (doc);
            desktop->change_document(doc);
            sp_document_resized_signal_emit (doc, sp_document_width(doc), sp_document_height(doc));
        } else {
            if (!Inkscape::NSApplication::Application::getNewGui()) {
                // create a whole new desktop and window
                SPViewWidget *dtw = sp_desktop_widget_new(sp_document_namedview(doc, NULL));
                sp_create_window(dtw, TRUE);
                desktop = static_cast<SPDesktop*>(dtw->view);
            } else {
                desktop = Inkscape::NSApplication::Editor::createDesktop (doc);
            }
        }

        doc->virgin = FALSE;
        // everyone who cares now has a reference, get rid of ours
        sp_document_unref(doc);
        // resize the window to match the document properties
        sp_namedview_window_from_document(desktop);
        sp_namedview_update_layers_from_document(desktop);

        if (add_to_recent) {
            sp_file_add_recent(SP_DOCUMENT_URI(doc));
        }

        return TRUE;
    } else {
        gchar *safeUri = Inkscape::IO::sanitizeString(uri.c_str());
        gchar *text = g_strdup_printf(_("Failed to load the requested file %s"), safeUri);
        sp_ui_error_dialog(text);
        g_free(text);
        g_free(safeUri);
        return FALSE;
    }
}

/**
 *  Handle prompting user for "do you want to revert"?  Revert on "OK"
 */
void
sp_file_revert_dialog()
{
    SPDesktop  *desktop = SP_ACTIVE_DESKTOP;
    g_assert(desktop != NULL);

    SPDocument *doc = sp_desktop_document(desktop);
    g_assert(doc != NULL);

    Inkscape::XML::Node     *repr = sp_document_repr_root(doc);
    g_assert(repr != NULL);

    gchar const *uri = doc->uri;
    if (!uri) {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved yet.  Cannot revert."));
        return;
    }

    bool do_revert = true;
    if (doc->isModifiedSinceSave()) {
        gchar *text = g_strdup_printf(_("Changes will be lost!  Are you sure you want to reload document %s?"), uri);

        bool response = desktop->warnDialog (text);
        g_free(text);

        if (!response) {
            do_revert = false;
        }
    }

    bool reverted;
    if (do_revert) {
        // Allow overwriting of current document.
        doc->virgin = TRUE;

        // remember current zoom and view
        double zoom = desktop->current_zoom();
        Geom::Point c = desktop->get_display_area().midpoint();

        reverted = sp_file_open(uri,NULL);
        if (reverted) {
            // restore zoom and view
            desktop->zoom_absolute(c[Geom::X], c[Geom::Y], zoom);
        }
    } else {
        reverted = false;
    }

    if (reverted) {
        desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Document reverted."));
    } else {
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not reverted."));
    }
}

void dump_str(gchar const *str, gchar const *prefix)
{
    Glib::ustring tmp;
    tmp = prefix;
    tmp += " [";
    size_t const total = strlen(str);
    for (unsigned i = 0; i < total; i++) {
        gchar *const tmp2 = g_strdup_printf(" %02x", (0x0ff & str[i]));
        tmp += tmp2;
        g_free(tmp2);
    }

    tmp += "]";
    g_message("%s", tmp.c_str());
}

void dump_ustr(Glib::ustring const &ustr)
{
    char const *cstr = ustr.c_str();
    char const *data = ustr.data();
    Glib::ustring::size_type const byteLen = ustr.bytes();
    Glib::ustring::size_type const dataLen = ustr.length();
    Glib::ustring::size_type const cstrLen = strlen(cstr);

    g_message("   size: %lu\n   length: %lu\n   bytes: %lu\n    clen: %lu",
              gulong(ustr.size()), gulong(dataLen), gulong(byteLen), gulong(cstrLen) );
    g_message( "  ASCII? %s", (ustr.is_ascii() ? "yes":"no") );
    g_message( "  UTF-8? %s", (ustr.validate() ? "yes":"no") );

    try {
        Glib::ustring tmp;
        for (Glib::ustring::size_type i = 0; i < ustr.bytes(); i++) {
            tmp = "    ";
            if (i < dataLen) {
                Glib::ustring::value_type val = ustr.at(i);
                gchar* tmp2 = g_strdup_printf( (((val & 0xff00) == 0) ? "  %02x" : "%04x"), val );
                tmp += tmp2;
                g_free( tmp2 );
            } else {
                tmp += "    ";
            }

            if (i < byteLen) {
                int val = (0x0ff & data[i]);
                gchar *tmp2 = g_strdup_printf("    %02x", val);
                tmp += tmp2;
                g_free( tmp2 );
                if ( val > 32 && val < 127 ) {
                    tmp2 = g_strdup_printf( "   '%c'", (gchar)val );
                    tmp += tmp2;
                    g_free( tmp2 );
                } else {
                    tmp += "    . ";
                }
            } else {
                tmp += "       ";
            }

            if ( i < cstrLen ) {
                int val = (0x0ff & cstr[i]);
                gchar* tmp2 = g_strdup_printf("    %02x", val);
                tmp += tmp2;
                g_free(tmp2);
                if ( val > 32 && val < 127 ) {
                    tmp2 = g_strdup_printf("   '%c'", (gchar) val);
                    tmp += tmp2;
                    g_free( tmp2 );
                } else {
                    tmp += "    . ";
                }
            } else {
                tmp += "            ";
            }

            g_message( "%s", tmp.c_str() );
        }
    } catch (...) {
        g_message("XXXXXXXXXXXXXXXXXX Exception" );
    }
    g_message("---------------");
}

/**
 *  Display an file Open selector.  Open a document if OK is pressed.
 *  Can select single or multiple files for opening.
 */
void
sp_file_open_dialog(Gtk::Window &parentWindow, gpointer /*object*/, gpointer /*data*/)
{
    //# Get the current directory for finding files
    static Glib::ustring open_path;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if(open_path.empty())
    {
        Glib::ustring attr = prefs->getString("/dialogs/open/path");
        if (!attr.empty()) open_path = attr;
    }

    //# Test if the open_path directory exists
    if (!Inkscape::IO::file_test(open_path.c_str(),
              (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)))
        open_path = "";

#ifdef WIN32
    //# If no open path, default to our win32 documents folder
    if (open_path.empty())
    {
        // The path to the My Documents folder is read from the
        // value "HKEY_CURRENT_USER\Software\Windows\CurrentVersion\Explorer\Shell Folders\Personal"
        HKEY key = NULL;
        if(RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
            0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
        {
            WCHAR utf16path[_MAX_PATH];
            DWORD value_type;
            DWORD data_size = sizeof(utf16path);
            if(RegQueryValueExW(key, L"Personal", NULL, &value_type,
                (BYTE*)utf16path, &data_size) == ERROR_SUCCESS)
            {
                g_assert(value_type == REG_SZ);
                gchar *utf8path = g_utf16_to_utf8(
                    (const gunichar2*)utf16path, -1, NULL, NULL, NULL);
                if(utf8path)
                {
                    open_path = Glib::ustring(utf8path);
                    g_free(utf8path);
                }
            }
        }
    }
#endif

    //# If no open path, default to our home directory
    if (open_path.empty())
    {
        open_path = g_get_home_dir();
        open_path.append(G_DIR_SEPARATOR_S);
    }

    //# Create a dialog
    Inkscape::UI::Dialog::FileOpenDialog *openDialogInstance =
              Inkscape::UI::Dialog::FileOpenDialog::create(
                 parentWindow, open_path,
                 Inkscape::UI::Dialog::SVG_TYPES,
                 _("Select file to open"));

    //# Show the dialog
    bool const success = openDialogInstance->show();

    //# Save the folder the user selected for later
    open_path = openDialogInstance->getCurrentDirectory();

    if (!success)
    {
        delete openDialogInstance;
        return;
    }

    //# User selected something.  Get name and type
    Glib::ustring fileName = openDialogInstance->getFilename();

    Inkscape::Extension::Extension *selection =
            openDialogInstance->getSelectionType();

    //# Code to check & open if multiple files.
    std::vector<Glib::ustring> flist = openDialogInstance->getFilenames();

    //# We no longer need the file dialog object - delete it
    delete openDialogInstance;
    openDialogInstance = NULL;

    //# Iterate through filenames if more than 1
    if (flist.size() > 1)
    {
        for (unsigned int i = 0; i < flist.size(); i++)
        {
            fileName = flist[i];

            Glib::ustring newFileName = Glib::filename_to_utf8(fileName);
            if ( newFileName.size() > 0 )
                fileName = newFileName;
            else
                g_warning( "ERROR CONVERTING OPEN FILENAME TO UTF-8" );

#ifdef INK_DUMP_FILENAME_CONV
            g_message("Opening File %s\n", fileName.c_str());
#endif
            sp_file_open(fileName, selection);
        }

        return;
    }


    if (!fileName.empty())
    {
        Glib::ustring newFileName = Glib::filename_to_utf8(fileName);

        if ( newFileName.size() > 0)
            fileName = newFileName;
        else
            g_warning( "ERROR CONVERTING OPEN FILENAME TO UTF-8" );

        open_path = Glib::path_get_dirname (fileName);
        open_path.append(G_DIR_SEPARATOR_S);
        prefs->setString("/dialogs/open/path", open_path);

        sp_file_open(fileName, selection);
    }

    return;
}


/*######################
## V A C U U M
######################*/

/**
 * Remove unreferenced defs from the defs section of the document.
 */


void
sp_file_vacuum()
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    unsigned int diff = vacuum_document (doc);

    sp_document_done(doc, SP_VERB_FILE_VACUUM,
                     _("Vacuum &lt;defs&gt;"));

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (diff > 0) {
        dt->messageStack()->flashF(Inkscape::NORMAL_MESSAGE,
                ngettext("Removed <b>%i</b> unused definition in &lt;defs&gt;.",
                         "Removed <b>%i</b> unused definitions in &lt;defs&gt;.",
                         diff),
                diff);
    } else {
        dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE,  _("No unused definitions in &lt;defs&gt;."));
    }
}



/*######################
## S A V E
######################*/

/**
 * This 'save' function called by the others below
 *
 * \param    official  whether to set :output_module and :modified in the
 *                     document; is true for normal save, false for temporary saves
 */
static bool
file_save(Gtk::Window &parentWindow, SPDocument *doc, const Glib::ustring &uri,
          Inkscape::Extension::Extension *key, bool checkoverwrite, bool official,
          Inkscape::Extension::FileSaveMethod save_method)
{
    if (!doc || uri.size()<1) //Safety check
        return false;

    try {
        Inkscape::Extension::save(key, doc, uri.c_str(),
                                  false,
                                  checkoverwrite, official,
                                  save_method);
    } catch (Inkscape::Extension::Output::no_extension_found &e) {
        gchar *safeUri = Inkscape::IO::sanitizeString(uri.c_str());
        gchar *text = g_strdup_printf(_("No Inkscape extension found to save document (%s).  This may have been caused by an unknown filename extension."), safeUri);
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved."));
        sp_ui_error_dialog(text);
        g_free(text);
        g_free(safeUri);
        return FALSE;
    } catch (Inkscape::Extension::Output::file_read_only &e) {
        gchar *safeUri = Inkscape::IO::sanitizeString(uri.c_str());
        gchar *text = g_strdup_printf(_("File %s is write protected. Please remove write protection and try again."), safeUri);
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved."));
        sp_ui_error_dialog(text);
        g_free(text);
        g_free(safeUri);
        return FALSE;
    } catch (Inkscape::Extension::Output::save_failed &e) {
        gchar *safeUri = Inkscape::IO::sanitizeString(uri.c_str());
        gchar *text = g_strdup_printf(_("File %s could not be saved."), safeUri);
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved."));
        sp_ui_error_dialog(text);
        g_free(text);
        g_free(safeUri);
        return FALSE;
    } catch (Inkscape::Extension::Output::save_cancelled &e) {
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved."));
        return FALSE;
    } catch (Inkscape::Extension::Output::no_overwrite &e) {
        return sp_file_save_dialog(parentWindow, doc, save_method);
    } catch (...) {
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved."));
        return FALSE;
    }

    SP_ACTIVE_DESKTOP->event_log->rememberFileSave();
    SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Document saved."));
    return true;
}

/*
 * Used only for remote saving using VFS and a specific uri. Gets the file at the /tmp.
 */
bool
file_save_remote(SPDocument */*doc*/,
    #ifdef WITH_GNOME_VFS
                 const Glib::ustring &uri,
    #else
                 const Glib::ustring &/*uri*/,
    #endif
                 Inkscape::Extension::Extension */*key*/, bool /*saveas*/, bool /*official*/)
{
#ifdef WITH_GNOME_VFS

#define BUF_SIZE 8192
    gnome_vfs_init();

    GnomeVFSHandle    *from_handle = NULL;
    GnomeVFSHandle    *to_handle = NULL;
    GnomeVFSFileSize  bytes_read;
    GnomeVFSFileSize  bytes_written;
    GnomeVFSResult    result;
    guint8 buffer[8192];

    gchar* uri_local = g_filename_from_utf8( uri.c_str(), -1, NULL, NULL, NULL);

    if ( uri_local == NULL ) {
        g_warning( "Error converting filename to locale encoding.");
    }

    // Gets the temp file name.
    Glib::ustring fileName = Glib::get_tmp_dir ();
    fileName.append(G_DIR_SEPARATOR_S);
    fileName.append((gnome_vfs_uri_extract_short_name(gnome_vfs_uri_new(uri_local))));

    // Open the temp file to send.
    result = gnome_vfs_open (&from_handle, fileName.c_str(), GNOME_VFS_OPEN_READ);

    if (result != GNOME_VFS_OK) {
        g_warning("Could not find the temp saving.");
        return false;
    }

    result = gnome_vfs_create (&to_handle, uri_local, GNOME_VFS_OPEN_WRITE, FALSE, GNOME_VFS_PERM_USER_ALL);
    result = gnome_vfs_open (&to_handle, uri_local, GNOME_VFS_OPEN_WRITE);

    if (result != GNOME_VFS_OK) {
        g_warning("file creating: %s", gnome_vfs_result_to_string(result));
        return false;
    }

    while (1) {

        result = gnome_vfs_read (from_handle, buffer, 8192, &bytes_read);

        if ((result == GNOME_VFS_ERROR_EOF) &&(!bytes_read)){
            result = gnome_vfs_close (from_handle);
            result = gnome_vfs_close (to_handle);
            return true;
        }

        if (result != GNOME_VFS_OK) {
            g_warning("%s", gnome_vfs_result_to_string(result));
            return false;
        }
        result = gnome_vfs_write (to_handle, buffer, bytes_read, &bytes_written);
        if (result != GNOME_VFS_OK) {
            g_warning("%s", gnome_vfs_result_to_string(result));
            return false;
        }


        if (bytes_read != bytes_written){
            return false;
        }

    }
    return true;
#else
    // in case we do not have GNOME_VFS
    return false;
#endif

}


/**
 * Check if a string ends with another string.
 * \todo Find a better code file to put this general purpose method
 */
static bool hasEnding (Glib::ustring const &fullString, Glib::ustring const &ending)
{
    if (fullString.length() > ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

/**
 *  Display a SaveAs dialog.  Save the document if OK pressed.
 */
bool
sp_file_save_dialog(Gtk::Window &parentWindow, SPDocument *doc, Inkscape::Extension::FileSaveMethod save_method)
{
    Inkscape::Extension::Output *extension = 0;
    bool is_copy = (save_method == Inkscape::Extension::FILE_SAVE_METHOD_SAVE_COPY);

    // Note: default_extension has the format "org.inkscape.output.svg.inkscape", whereas
    //       filename_extension only uses ".svg"
    Glib::ustring default_extension;
    Glib::ustring filename_extension = ".svg";

    default_extension= Inkscape::Extension::get_file_save_extension(save_method);
    //g_message("%s: extension name: '%s'", __FUNCTION__, default_extension);

    extension = dynamic_cast<Inkscape::Extension::Output *>
        (Inkscape::Extension::db.get(default_extension.c_str()));

    if (extension)
        filename_extension = extension->get_extension();

    Glib::ustring save_path;
    Glib::ustring save_loc;

    save_path = Inkscape::Extension::get_file_save_path(doc, save_method);

    if (!Inkscape::IO::file_test(save_path.c_str(),
          (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)))
        save_path = "";

    if (save_path.size()<1)
        save_path = g_get_home_dir();

    save_loc = save_path;
    save_loc.append(G_DIR_SEPARATOR_S);

    char formatBuf[256];
    int i = 1;
    if (!doc->uri) {
        // We are saving for the first time; create a unique default filename
        snprintf(formatBuf, 255, _("drawing%s"), filename_extension.c_str());
        save_loc.append(formatBuf);

        while (Inkscape::IO::file_test(save_loc.c_str(), G_FILE_TEST_EXISTS)) {
            save_loc = save_path;
            save_loc.append(G_DIR_SEPARATOR_S);
            snprintf(formatBuf, 255, _("drawing-%d%s"), i++, filename_extension.c_str());
            save_loc.append(formatBuf);
        }
    } else {
        snprintf(formatBuf, 255, _("%s"), Glib::path_get_basename(doc->uri).c_str());
        save_loc.append(formatBuf);
    }

    // convert save_loc from utf-8 to locale
    // is this needed any more, now that everything is handled in
    // Inkscape::IO?
    Glib::ustring save_loc_local = Glib::filename_from_utf8(save_loc);

    if ( save_loc_local.size() > 0)
        save_loc = save_loc_local;

    //# Show the SaveAs dialog
    char const * dialog_title;
    if (is_copy) {
        dialog_title = (char const *) _("Select file to save a copy to");
    } else {
        dialog_title = (char const *) _("Select file to save to");
    }
    gchar* doc_title = doc->root->title();
    Inkscape::UI::Dialog::FileSaveDialog *saveDialog =
        Inkscape::UI::Dialog::FileSaveDialog::create(
            parentWindow,
            save_loc,
            Inkscape::UI::Dialog::SVG_TYPES,
            dialog_title,
            default_extension,
            doc_title ? doc_title : "",
            save_method
            );

    saveDialog->setSelectionType(extension);

    bool success = saveDialog->show();
    if (!success) {
        delete saveDialog;
        return success;
    }

    // set new title here (call RDF to ensure metadata and title element are updated)
    rdf_set_work_entity(doc, rdf_find_entity("title"), saveDialog->getDocTitle().c_str());
    // free up old string
    if(doc_title) g_free(doc_title);

    Glib::ustring fileName = saveDialog->getFilename();
    Inkscape::Extension::Extension *selectionType = saveDialog->getSelectionType();

    delete saveDialog;

    saveDialog = 0;

    if (fileName.size() > 0) {
        Glib::ustring newFileName = Glib::filename_to_utf8(fileName);

        if ( newFileName.size()>0 )
            fileName = newFileName;
        else
            g_warning( "Error converting save filename to UTF-8." );

        Inkscape::Extension::Output *omod = dynamic_cast<Inkscape::Extension::Output *>(selectionType);
        if (omod) {
            Glib::ustring save_extension = (std::string)omod->get_extension();
            if ( !hasEnding(fileName, save_extension.c_str()) ) {
                fileName += save_extension.c_str();
            }
        }

        // FIXME: does the argument !is_copy really convey the correct meaning here?
        success = file_save(parentWindow, doc, fileName, selectionType, TRUE, !is_copy, save_method);

        if (success && SP_DOCUMENT_URI(doc)) {
            sp_file_add_recent(SP_DOCUMENT_URI(doc));
        }

        save_path = Glib::path_get_dirname(fileName);
        Inkscape::Extension::store_save_path_in_prefs(save_path, save_method);

        return success;
    }


    return false;
}


/**
 * Save a document, displaying a SaveAs dialog if necessary.
 */
bool
sp_file_save_document(Gtk::Window &parentWindow, SPDocument *doc)
{
    bool success = true;

    if (doc->isModifiedSinceSave()) {
        if ( doc->uri == NULL )
        {
            // Hier sollte in Argument mitgegeben werden, das anzeigt, da� das Dokument das erste
            // Mal gespeichert wird, so da� als default .svg ausgew�hlt wird und nicht die zuletzt
            // benutzte "Save as ..."-Endung
            return sp_file_save_dialog(parentWindow, doc, Inkscape::Extension::FILE_SAVE_METHOD_INKSCAPE_SVG);
        } else {
            Glib::ustring extension = Inkscape::Extension::get_file_save_extension(Inkscape::Extension::FILE_SAVE_METHOD_SAVE_AS);
            Glib::ustring fn = g_strdup(doc->uri);
            // Try to determine the extension from the uri; this may not lead to a valid extension,
            // but this case is caught in the file_save method below (or rather in Extension::save()
            // further down the line).
            Glib::ustring ext = "";
            Glib::ustring::size_type pos = fn.rfind('.');
            if (pos != Glib::ustring::npos) {
                // FIXME: this could/should be more sophisticated (see FileSaveDialog::appendExtension()),
                // but hopefully it's a reasonable workaround for now
                ext = fn.substr( pos );
            }
            success = file_save(parentWindow, doc, fn, Inkscape::Extension::db.get(ext.c_str()), FALSE, TRUE, Inkscape::Extension::FILE_SAVE_METHOD_SAVE_AS);
            if (success == false) {
                // give the user the chance to change filename or extension
                return sp_file_save_dialog(parentWindow, doc, Inkscape::Extension::FILE_SAVE_METHOD_INKSCAPE_SVG);
            }
        }
    } else {
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("No changes need to be saved."));
        success = TRUE;
    }

    return success;
}


/**
 * Save a document.
 */
bool
sp_file_save(Gtk::Window &parentWindow, gpointer /*object*/, gpointer /*data*/)
{
    if (!SP_ACTIVE_DOCUMENT)
        return false;

    SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Saving document..."));

    sp_namedview_document_from_window(SP_ACTIVE_DESKTOP);
    return sp_file_save_document(parentWindow, SP_ACTIVE_DOCUMENT);
}


/**
 *  Save a document, always displaying the SaveAs dialog.
 */
bool
sp_file_save_as(Gtk::Window &parentWindow, gpointer /*object*/, gpointer /*data*/)
{
    if (!SP_ACTIVE_DOCUMENT)
        return false;
    sp_namedview_document_from_window(SP_ACTIVE_DESKTOP);
    return sp_file_save_dialog(parentWindow, SP_ACTIVE_DOCUMENT, Inkscape::Extension::FILE_SAVE_METHOD_SAVE_AS);
}



/**
 *  Save a copy of a document, always displaying a sort of SaveAs dialog.
 */
bool
sp_file_save_a_copy(Gtk::Window &parentWindow, gpointer /*object*/, gpointer /*data*/)
{
    if (!SP_ACTIVE_DOCUMENT)
        return false;
    sp_namedview_document_from_window(SP_ACTIVE_DESKTOP);
    return sp_file_save_dialog(parentWindow, SP_ACTIVE_DOCUMENT, Inkscape::Extension::FILE_SAVE_METHOD_SAVE_COPY);
}


/*######################
## I M P O R T
######################*/

/**
 *  Import a resource.  Called by sp_file_import()
 */
void
file_import(SPDocument *in_doc, const Glib::ustring &uri,
               Inkscape::Extension::Extension *key)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    //DEBUG_MESSAGE( fileImport, "file_import( in_doc:%p uri:[%s], key:%p", in_doc, uri, key );
    SPDocument *doc;
    try {
        doc = Inkscape::Extension::open(key, uri.c_str());
    } catch (Inkscape::Extension::Input::no_extension_found &e) {
        doc = NULL;
    } catch (Inkscape::Extension::Input::open_failed &e) {
        doc = NULL;
    }

    if (doc != NULL) {
        Inkscape::XML::rebase_hrefs(doc, in_doc->base, true);
        Inkscape::XML::Document *xml_in_doc = sp_document_repr_doc(in_doc);

        prevent_id_clashes(doc, in_doc);

        SPObject *in_defs = SP_DOCUMENT_DEFS(in_doc);
        Inkscape::XML::Node *last_def = SP_OBJECT_REPR(in_defs)->lastChild();

        SPCSSAttr *style = sp_css_attr_from_object(SP_DOCUMENT_ROOT(doc));

        // Count the number of top-level items in the imported document.
        guint items_count = 0;
        for (SPObject *child = sp_object_first_child(SP_DOCUMENT_ROOT(doc));
             child != NULL; child = SP_OBJECT_NEXT(child))
        {
            if (SP_IS_ITEM(child)) items_count++;
        }

        // Create a new group if necessary.
        Inkscape::XML::Node *newgroup = NULL;
        if ((style && style->attributeList()) || items_count > 1) {
            newgroup = xml_in_doc->createElement("svg:g");
            sp_repr_css_set(newgroup, style, "style");
        }

        // Determine the place to insert the new object.
        // This will be the current layer, if possible.
        // FIXME: If there's no desktop (command line run?) we need
        //        a document:: method to return the current layer.
        //        For now, we just use the root in this case.
        SPObject *place_to_insert;
        if (desktop) {
            place_to_insert = desktop->currentLayer();
        } else {
            place_to_insert = SP_DOCUMENT_ROOT(in_doc);
        }

        // Construct a new object representing the imported image,
        // and insert it into the current document.
        SPObject *new_obj = NULL;
        for (SPObject *child = sp_object_first_child(SP_DOCUMENT_ROOT(doc));
             child != NULL; child = SP_OBJECT_NEXT(child) )
        {
            if (SP_IS_ITEM(child)) {
                Inkscape::XML::Node *newitem = SP_OBJECT_REPR(child)->duplicate(xml_in_doc);

                // convert layers to groups, and make sure they are unlocked
                // FIXME: add "preserve layers" mode where each layer from
                //        import is copied to the same-named layer in host
                newitem->setAttribute("inkscape:groupmode", NULL);
                newitem->setAttribute("sodipodi:insensitive", NULL);

                if (newgroup) newgroup->appendChild(newitem);
                else new_obj = place_to_insert->appendChildRepr(newitem);
            }

            // don't lose top-level defs or style elements
            else if (SP_OBJECT_REPR(child)->type() == Inkscape::XML::ELEMENT_NODE) {
                const gchar *tag = SP_OBJECT_REPR(child)->name();
                if (!strcmp(tag, "svg:defs")) {
                    for (SPObject *x = sp_object_first_child(child);
                         x != NULL; x = SP_OBJECT_NEXT(x))
                    {
                        SP_OBJECT_REPR(in_defs)->addChild(SP_OBJECT_REPR(x)->duplicate(xml_in_doc), last_def);
                    }
                }
                else if (!strcmp(tag, "svg:style")) {
                    SP_DOCUMENT_ROOT(in_doc)->appendChildRepr(SP_OBJECT_REPR(child)->duplicate(xml_in_doc));
                }
            }
        }
        if (newgroup) new_obj = place_to_insert->appendChildRepr(newgroup);

        // release some stuff
        if (newgroup) Inkscape::GC::release(newgroup);
        if (style) sp_repr_css_attr_unref(style);

        // select and move the imported item
        if (new_obj && SP_IS_ITEM(new_obj)) {
            Inkscape::Selection *selection = sp_desktop_selection(desktop);
            selection->set(SP_ITEM(new_obj));

            // preserve parent and viewBox transformations
            // c2p is identity matrix at this point unless sp_document_ensure_up_to_date is called
            sp_document_ensure_up_to_date(doc);
            Geom::Matrix affine = SP_ROOT(SP_DOCUMENT_ROOT(doc))->c2p * sp_item_i2doc_affine(SP_ITEM(place_to_insert)).inverse();
            sp_selection_apply_affine(selection, desktop->dt2doc() * affine * desktop->doc2dt(), true, false);

            // move to mouse pointer
            {
                sp_document_ensure_up_to_date(sp_desktop_document(desktop));
                Geom::OptRect sel_bbox = selection->bounds();
                if (sel_bbox) {
                    Geom::Point m( desktop->point() - sel_bbox->midpoint() );
                    sp_selection_move_relative(selection, m, false);
                }
            }
        }

        sp_document_unref(doc);
        sp_document_done(in_doc, SP_VERB_FILE_IMPORT,
                         _("Import"));

    } else {
        gchar *text = g_strdup_printf(_("Failed to load the requested file %s"), uri.c_str());
        sp_ui_error_dialog(text);
        g_free(text);
    }

    return;
}


/**
 *  Display an Open dialog, import a resource if OK pressed.
 */
void
sp_file_import(Gtk::Window &parentWindow)
{
    static Glib::ustring import_path;

    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (!doc)
        return;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if(import_path.empty())
    {
        Glib::ustring attr = prefs->getString("/dialogs/import/path");
        if (!attr.empty()) import_path = attr;
    }

    //# Test if the import_path directory exists
    if (!Inkscape::IO::file_test(import_path.c_str(),
              (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)))
        import_path = "";

    //# If no open path, default to our home directory
    if (import_path.empty())
    {
        import_path = g_get_home_dir();
        import_path.append(G_DIR_SEPARATOR_S);
    }

    // Create new dialog (don't use an old one, because parentWindow has probably changed)
    Inkscape::UI::Dialog::FileOpenDialog *importDialogInstance =
             Inkscape::UI::Dialog::FileOpenDialog::create(
                 parentWindow,
                 import_path,
                 Inkscape::UI::Dialog::IMPORT_TYPES,
                 (char const *)_("Select file to import"));

    bool success = importDialogInstance->show();
    if (!success) {
        delete importDialogInstance;
        return;
    }

    //# Get file name and extension type
    Glib::ustring fileName = importDialogInstance->getFilename();
    Inkscape::Extension::Extension *selection = importDialogInstance->getSelectionType();

    delete importDialogInstance;
    importDialogInstance = NULL;

    if (fileName.size() > 0) {

        Glib::ustring newFileName = Glib::filename_to_utf8(fileName);

        if ( newFileName.size() > 0)
            fileName = newFileName;
        else
            g_warning( "ERROR CONVERTING OPEN FILENAME TO UTF-8" );

        import_path = Glib::path_get_dirname (fileName);
        import_path.append(G_DIR_SEPARATOR_S);
        prefs->setString("/dialogs/import/path", import_path);

        file_import(doc, fileName, selection);
    }

    return;
}



/*######################
## E X P O R T
######################*/


#ifdef NEW_EXPORT_DIALOG

/**
 *  Display an Export dialog, export as the selected type if OK pressed
 */
bool
sp_file_export_dialog(Gtk::Window &parentWindow)
{
    //# temp hack for 'doc' until we can switch to this dialog
    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    Glib::ustring export_path;
    Glib::ustring export_loc;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Inkscape::Extension::Output *extension;

    //# Get the default extension name
    Glib::ustring default_extension = prefs->getString("/dialogs/save_export/default");
    if(default_extension.empty()) {
        // FIXME: Is this a good default? Should there be a macro for the string?
        default_extension = "org.inkscape.output.png.cairo";
    }
    //g_message("%s: extension name: '%s'", __FUNCTION__, default_extension);

    if (doc->uri == NULL)
        {
        char formatBuf[256];

        Glib::ustring filename_extension = ".svg";
        extension = dynamic_cast<Inkscape::Extension::Output *>
              (Inkscape::Extension::db.get(default_extension.c_str()));
        //g_warning("%s: extension ptr: 0x%x", __FUNCTION__, (unsigned int)extension);
        if (extension)
            filename_extension = extension->get_extension();

        Glib::ustring attr3 = prefs->getString("/dialogs/save_export/path");
        if (!attr3.empty())
            export_path = attr3;

        if (!Inkscape::IO::file_test(export_path.c_str(),
              (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)))
            export_path = "";

        if (export_path.size()<1)
            export_path = g_get_home_dir();

        export_loc = export_path;
        export_loc.append(G_DIR_SEPARATOR_S);
        snprintf(formatBuf, 255, _("drawing%s"), filename_extension.c_str());
        export_loc.append(formatBuf);

        }
    else
        {
        export_path = Glib::path_get_dirname(doc->uri);
        }

    // convert save_loc from utf-8 to locale
    // is this needed any more, now that everything is handled in
    // Inkscape::IO?
    Glib::ustring export_path_local = Glib::filename_from_utf8(export_path);
    if ( export_path_local.size() > 0)
        export_path = export_path_local;

    //# Show the Export dialog
    Inkscape::UI::Dialog::FileExportDialog *exportDialogInstance =
        Inkscape::UI::Dialog::FileExportDialog::create(
            parentWindow,
            export_path,
            Inkscape::UI::Dialog::EXPORT_TYPES,
            (char const *) _("Select file to export to"),
            default_extension
        );

    bool success = exportDialogInstance->show();
    if (!success) {
        delete exportDialogInstance;
        return success;
    }

    Glib::ustring fileName = exportDialogInstance->getFilename();

    Inkscape::Extension::Extension *selectionType =
        exportDialogInstance->getSelectionType();

    delete exportDialogInstance;
    exportDialogInstance = NULL;

    if (fileName.size() > 0) {
        Glib::ustring newFileName = Glib::filename_to_utf8(fileName);

        if ( newFileName.size()>0 )
            fileName = newFileName;
        else
            g_warning( "Error converting save filename to UTF-8." );

        success = file_save(parentWindow, doc, fileName, selectionType, TRUE, FALSE, Inkscape::Extension::FILE_SAVE_METHOD_EXPORT);

        if (success) {
            Glib::RefPtr<Gtk::RecentManager> recent = Gtk::RecentManager::get_default();
            recent->add_item(SP_DOCUMENT_URI(doc));
        }

        export_path = fileName;
        prefs->setString("/dialogs/save_export/path", export_path);

        return success;
    }


    return false;
}

#else

/**
 *
 */
bool
sp_file_export_dialog(Gtk::Window &/*parentWindow*/)
{
    sp_export_dialog();
    return true;
}

#endif

/*######################
## E X P O R T  T O  O C A L
######################*/

/**
 *  Display an Export dialog, export as the selected type if OK pressed
 */
/*
bool
sp_file_export_to_ocal_dialog(Gtk::Window &parentWindow)
{

   if (!SP_ACTIVE_DOCUMENT)
        return false;

    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    Glib::ustring export_path;
    Glib::ustring export_loc;
    Glib::ustring fileName;
    Inkscape::Extension::Extension *selectionType;

    bool success = false;

    static bool gotSuccess = false;

    Inkscape::XML::Node *repr = sp_document_repr_root(doc);
    (void)repr;

    if (!doc->uri && !doc->isModifiedSinceSave())
        return false;

    //  Get the default extension name
    Glib::ustring default_extension = "org.inkscape.output.svg.inkscape";
    char formatBuf[256];

    Glib::ustring filename_extension = ".svg";
    selectionType = Inkscape::Extension::db.get(default_extension.c_str());

    export_path = Glib::get_tmp_dir ();

    export_loc = export_path;
    export_loc.append(G_DIR_SEPARATOR_S);
    snprintf(formatBuf, 255, _("drawing%s"), filename_extension.c_str());
    export_loc.append(formatBuf);

    // convert save_loc from utf-8 to locale
    // is this needed any more, now that everything is handled in
    // Inkscape::IO?
    Glib::ustring export_path_local = Glib::filename_from_utf8(export_path);
    if ( export_path_local.size() > 0)
        export_path = export_path_local;

    // Show the Export To OCAL dialog
    Inkscape::UI::Dialog::FileExportToOCALDialog *exportDialogInstance =
        new Inkscape::UI::Dialog::FileExportToOCALDialog(
                parentWindow,
                Inkscape::UI::Dialog::EXPORT_TYPES,
                (char const *) _("Select file to export to")
                );

    success = exportDialogInstance->show();
    if (!success) {
        delete exportDialogInstance;
        return success;
    }

    fileName = exportDialogInstance->getFilename();

    delete exportDialogInstance;
    exportDialogInstance = NULL;;

    fileName.append(filename_extension.c_str());
    if (fileName.size() > 0) {
        Glib::ustring newFileName = Glib::filename_to_utf8(fileName);

        if ( newFileName.size()>0 )
            fileName = newFileName;
        else
            g_warning( "Error converting save filename to UTF-8." );
    }
    Glib::ustring filePath = export_path;
    filePath.append(G_DIR_SEPARATOR_S);
    filePath.append(Glib::path_get_basename(fileName));

    fileName = filePath;

    success = file_save(parentWindow, doc, filePath, selectionType, FALSE, FALSE, Inkscape::Extension::FILE_SAVE_METHOD_EXPORT);

    if (!success){
        gchar *text = g_strdup_printf(_("Error saving a temporary copy"));
        sp_ui_error_dialog(text);

        return success;
    }

    // Start now the submition

    // Create the uri
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring uri = "dav://";
    Glib::ustring username = prefs->getString("/options/ocalusername/str");
    Glib::ustring password = prefs->getString("/options/ocalpassword/str");
    if (username.empty() || password.empty())
    {
        Inkscape::UI::Dialog::FileExportToOCALPasswordDialog *exportPasswordDialogInstance = NULL;
        if(!gotSuccess)
        {
            exportPasswordDialogInstance = new Inkscape::UI::Dialog::FileExportToOCALPasswordDialog(
                    parentWindow,
                    (char const *) _("Open Clip Art Login"));
            success = exportPasswordDialogInstance->show();
            if (!success) {
                delete exportPasswordDialogInstance;
                return success;
            }
        }
        username = exportPasswordDialogInstance->getUsername();
        password = exportPasswordDialogInstance->getPassword();

        delete exportPasswordDialogInstance;
        exportPasswordDialogInstance = NULL;
    }
    uri.append(username);
    uri.append(":");
    uri.append(password);
    uri.append("@");
    uri.append(prefs->getString("/options/ocalurl/str"));
    uri.append("/dav.php/");
    uri.append(Glib::path_get_basename(fileName));

    // Save as a remote file using the dav protocol.
    success = file_save_remote(doc, uri, selectionType, FALSE, FALSE);
    remove(fileName.c_str());
    if (!success)
    {
        gchar *text = g_strdup_printf(_("Error exporting the document. Verify if the server name, username and password are correct, if the server has support for webdav and verify if you didn't forget to choose a license."));
        sp_ui_error_dialog(text);
    }
    else
        gotSuccess = true;

    return success;
}
*/
/**
 * Export the current document to OCAL
 */
/*
void
sp_file_export_to_ocal(Gtk::Window &parentWindow)
{

    // Try to execute the new code and return;
    if (!SP_ACTIVE_DOCUMENT)
        return;
    bool success = sp_file_export_to_ocal_dialog(parentWindow);
    if (success)
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Document exported..."));
}
*/

/*######################
## I M P O R T  F R O M  O C A L
######################*/

/**
 * Display an ImportToOcal Dialog, and the selected document from OCAL
 */
void
sp_file_import_from_ocal(Gtk::Window &parentWindow)
{
    static Glib::ustring import_path;

    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (!doc)
        return;

    Inkscape::UI::Dialog::FileImportFromOCALDialog *importDialogInstance = NULL;

    if (!importDialogInstance) {
        importDialogInstance = new
             Inkscape::UI::Dialog::FileImportFromOCALDialog(
                 parentWindow,
                 import_path,
                 Inkscape::UI::Dialog::IMPORT_TYPES,
                 (char const *)_("Import From Open Clip Art Library"));
    }

    bool success = importDialogInstance->show();
    if (!success) {
        delete importDialogInstance;
        return;
    }

    // Get file name and extension type
    Glib::ustring fileName = importDialogInstance->getFilename();
    Inkscape::Extension::Extension *selection = importDialogInstance->getSelectionType();

    delete importDialogInstance;
    importDialogInstance = NULL;

    if (fileName.size() > 0) {

        Glib::ustring newFileName = Glib::filename_to_utf8(fileName);

        if ( newFileName.size() > 0)
            fileName = newFileName;
        else
            g_warning( "ERROR CONVERTING OPEN FILENAME TO UTF-8" );

        import_path = fileName;
        if (import_path.size()>0)
            import_path.append(G_DIR_SEPARATOR_S);

        file_import(doc, fileName, selection);
    }

    return;
}

/*######################
## P R I N T
######################*/


/**
 *  Print the current document, if any.
 */
void
sp_file_print(Gtk::Window& parentWindow)
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (doc)
        sp_print_document(parentWindow, doc);
}

/**
 * Display what the drawing would look like, if
 * printed.
 */
void
sp_file_print_preview(gpointer /*object*/, gpointer /*data*/)
{

    SPDocument *doc = SP_ACTIVE_DOCUMENT;
    if (doc)
        sp_print_preview_document(doc);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
