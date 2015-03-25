/** @file
 * @brief A simple dialog with information about extensions
 */
/* Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2005 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h> //for GTK_RESPONSE* types
#include <gtkmm/scrolledwindow.h>

#include "extension/db.h"
#include "extensions.h"


namespace Inkscape {
namespace UI {
namespace Dialogs {

using Inkscape::Extension::Extension;

ExtensionsPanel &ExtensionsPanel::getInstance()
{
    ExtensionsPanel &instance = *new ExtensionsPanel();

    instance.rescan();

    return instance;
}



/**
 * Constructor
 */
ExtensionsPanel::ExtensionsPanel() :
        _showAll(false)
{
    Gtk::ScrolledWindow* scroller = new Gtk::ScrolledWindow();

    _view.set_editable(false);

    scroller->add(_view);
    add(*scroller);

    rescan();

    show_all_children();
}

void ExtensionsPanel::set_full(bool full)
{
    if ( full != _showAll ) {
        _showAll = full;
        rescan();
    }
}

void ExtensionsPanel::listCB( Inkscape::Extension::Extension * in_plug, gpointer in_data )
{
    ExtensionsPanel * self = (ExtensionsPanel*)in_data;

    const char* stateStr;
    Extension::state_t state = in_plug->get_state();
    switch ( state ) {
        case Extension::STATE_LOADED:
        {
            stateStr = "loaded";
        }
        break;
        case Extension::STATE_UNLOADED:
        {
            stateStr = "unloaded";
        }
        break;
        case Extension::STATE_DEACTIVATED:
        {
            stateStr = "deactivated";
        }
        break;
        default:
            stateStr = "unknown";
    }

    if ( self->_showAll || in_plug->deactivated() ) {
//         gchar* line = g_strdup_printf( " extension   %c %c  %s   |%s|%s|",
//                                        (in_plug->loaded() ? 'X' : '-'),
//                                        (in_plug->deactivated() ? 'X' : '-'),
//                                        stateStr, in_plug->get_id(),
//                                        in_plug->get_name() );
        gchar* line = g_strdup_printf( "%s   %s\n        \"%s\"", stateStr, in_plug->get_name(), in_plug->get_id() );

        self->_view.get_buffer()->insert( self->_view.get_buffer()->end(), line );
        self->_view.get_buffer()->insert( self->_view.get_buffer()->end(), "\n" );
        //g_message( "%s", line );
    }



    return;
}

void ExtensionsPanel::rescan()
{
    _view.get_buffer()->set_text("Extensions:\n");
//     g_message("/------------------");

    Inkscape::Extension::db.foreach(listCB, (gpointer)this);

//     g_message("\\------------------");
}

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

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
