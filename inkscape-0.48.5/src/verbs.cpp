#define __SP_VERBS_C__
/**
 * \file verbs.cpp
 *
 * \brief Actions for inkscape
 *
 * This file implements routines necessary to deal with verbs.  A verb
 * is a numeric identifier used to retrieve standard SPActions for particular
 * views.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *   MenTaLguY <mental@rydia.net>
 *   David Turner <novalis@gnu.org>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) (date unspecified) Authors
 * This code is in public domain.
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <gtk/gtk.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>
#include <string>

#include "bind/javabind.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "dialogs/clonetiler.h"
#include "dialogs/find.h"
#include "dialogs/item-properties.h"
#include "dialogs/spellcheck.h"
#include "dialogs/text-edit.h"
#include "dialogs/xml-tree.h"
#include "display/curve.h"
#include "document.h"
#include "draw-context.h"
#include "extension/effect.h"
#include "file.h"
#include "gradient-drag.h"
#include "helper/action.h"
#include "help.h"
#include "inkscape-private.h"
#include "interface.h"
#include "layer-fns.h"
#include "layer-manager.h"
#include "message-stack.h"
#include "path-chemistry.h"
#include "preferences.h"
#include "select-context.h"
#include "selection-chemistry.h"
#include "seltrans.h"
#include "shape-editor.h"
#include "shortcuts.h"
#include "sp-flowtext.h"
#include "sp-guide.h"
#include "splivarot.h"
#include "sp-namedview.h"
#include "text-chemistry.h"
#include "tools-switch.h"
#include "ui/dialog/dialog-manager.h"
#include "ui/dialog/document-properties.h"
#include "ui/dialog/extensions.h"
#include "ui/dialog/glyphs.h"
#include "ui/dialog/icon-preview.h"
#include "ui/dialog/inkscape-preferences.h"
#include "ui/dialog/layer-properties.h"
#include "ui/dialog/layers.h"
#include "ui/dialog/swatches.h"
#include "ui/icon-names.h"
#include "ui/tool/node-tool.h"

//#ifdef WITH_INKBOARD
//#include "jabber_whiteboard/session-manager.h"
//#endif

/**
 * \brief Return the name without underscores and ellipsis, for use in dialog
 * titles, etc. Allocated memory must be freed by caller.
 */
gchar *
sp_action_get_title(SPAction const *action)
{
    char const *src = action->name;
    gchar *ret = g_new(gchar, strlen(src) + 1);
    unsigned ri = 0;

    for (unsigned si = 0 ; ; si++)  {
        int const c = src[si];
        if ( c != '_' && c != '.' ) {
            ret[ri] = c;
            ri++;
            if (c == '\0') {
                return ret;
            }
        }
    }

} // end of sp_action_get_title()

namespace Inkscape {

/** \brief A class to encompass all of the verbs which deal with
           file operations. */
class FileVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    FileVerb(unsigned int const code,
             gchar const *id,
             gchar const *name,
             gchar const *tip,
             gchar const *image) :
        Verb(code, id, name, tip, image)
    { }
}; /* FileVerb class */

/** \brief A class to encompass all of the verbs which deal with
           edit operations. */
class EditVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    EditVerb(unsigned int const code,
             gchar const *id,
             gchar const *name,
             gchar const *tip,
             gchar const *image) :
        Verb(code, id, name, tip, image)
    { }
}; /* EditVerb class */

/** \brief A class to encompass all of the verbs which deal with
           selection operations. */
class SelectionVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    SelectionVerb(unsigned int const code,
                  gchar const *id,
                  gchar const *name,
                  gchar const *tip,
                  gchar const *image) :
        Verb(code, id, name, tip, image)
    { }
}; /* SelectionVerb class */

/** \brief A class to encompass all of the verbs which deal with
           layer operations. */
class LayerVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    LayerVerb(unsigned int const code,
              gchar const *id,
              gchar const *name,
              gchar const *tip,
              gchar const *image) :
        Verb(code, id, name, tip, image)
    { }
}; /* LayerVerb class */

/** \brief A class to encompass all of the verbs which deal with
           operations related to objects. */
class ObjectVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    ObjectVerb(unsigned int const code,
               gchar const *id,
               gchar const *name,
               gchar const *tip,
               gchar const *image) :
        Verb(code, id, name, tip, image)
    { }
}; /* ObjectVerb class */

/** \brief A class to encompass all of the verbs which deal with
           operations relative to context. */
class ContextVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    ContextVerb(unsigned int const code,
                gchar const *id,
                gchar const *name,
                gchar const *tip,
                gchar const *image) :
        Verb(code, id, name, tip, image)
    { }
}; /* ContextVerb class */

/** \brief A class to encompass all of the verbs which deal with
           zoom operations. */
class ZoomVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    ZoomVerb(unsigned int const code,
             gchar const *id,
             gchar const *name,
             gchar const *tip,
             gchar const *image) :
        Verb(code, id, name, tip, image)
    { }
}; /* ZoomVerb class */


/** \brief A class to encompass all of the verbs which deal with
           dialog operations. */
class DialogVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    DialogVerb(unsigned int const code,
               gchar const *id,
               gchar const *name,
               gchar const *tip,
               gchar const *image) :
        Verb(code, id, name, tip, image)
    { }
}; /* DialogVerb class */

/** \brief A class to encompass all of the verbs which deal with
           help operations. */
class HelpVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    HelpVerb(unsigned int const code,
             gchar const *id,
             gchar const *name,
             gchar const *tip,
             gchar const *image) :
        Verb(code, id, name, tip, image)
    { }
}; /* HelpVerb class */

/** \brief A class to encompass all of the verbs which deal with
           tutorial operations. */
class TutorialVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    TutorialVerb(unsigned int const code,
                 gchar const *id,
                 gchar const *name,
                 gchar const *tip,
                 gchar const *image) :
        Verb(code, id, name, tip, image)
    { }
}; /* TutorialVerb class */

/** \brief A class to encompass all of the verbs which deal with
           text operations. */
class TextVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    TextVerb(unsigned int const code,
              gchar const *id,
              gchar const *name,
              gchar const *tip,
              gchar const *image) :
        Verb(code, id, name, tip, image)
    { }
}; //TextVerb : public Verb

Verb::VerbTable Verb::_verbs;
Verb::VerbIDTable Verb::_verb_ids;

/** \brief  Create a verb without a code.

    This function calls the other constructor for all of the parameters,
    but generates the code.  It is important to READ THE OTHER DOCUMENTATION
    it has important details in it.  To generate the code a static is
    used which starts at the last static value: \c SP_VERB_LAST.  For
    each call it is incremented.  The list of allocated verbs is kept
    in the \c _verbs hashtable which is indexed by the \c code.
*/
Verb::Verb(gchar const *id, gchar const *name, gchar const *tip, gchar const *image) :
    _actions(0),
    _id(id),
    _name(name),
    _tip(tip),
    _full_tip(0),
    _image(image),
    _code(0),
    _default_sensitive(false)
{
    static int count = SP_VERB_LAST;

    count++;
    _code = count;
    _verbs.insert(VerbTable::value_type(count, this));
    _verb_ids.insert(VerbIDTable::value_type(_id, this));
}

/** \brief  Destroy a verb.

      The only allocated variable is the _actions variable.  If it has
    been allocated it is deleted.
*/
Verb::~Verb(void)
{
    /// \todo all the actions need to be cleaned up first.
    if (_actions != NULL) {
        delete _actions;
    }
    if (_full_tip) {
        g_free(_full_tip);
        _full_tip = 0;
    }

}

/** \brief  Verbs are no good without actions.  This is a place holder
            for a function that every subclass should write.  Most
            can be written using \c make_action_helper.
    \param  view  Which view the action should be created for.
    \return NULL to represent error (this function shouldn't ever be called)
*/
SPAction *
Verb::make_action(Inkscape::UI::View::View */*view*/)
{
    //std::cout << "make_action" << std::endl;
    return NULL;
}

/** \brief  Create an action for a \c FileVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
FileVerb::make_action(Inkscape::UI::View::View *view)
{
    //std::cout << "fileverb: make_action: " << &vector << std::endl;
    return make_action_helper(view, &vector);
}

/** \brief  Create an action for a \c EditVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
EditVerb::make_action(Inkscape::UI::View::View *view)
{
    //std::cout << "editverb: make_action: " << &vector << std::endl;
    return make_action_helper(view, &vector);
}

/** \brief  Create an action for a \c SelectionVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
SelectionVerb::make_action(Inkscape::UI::View::View *view)
{
    return make_action_helper(view, &vector);
}

/** \brief  Create an action for a \c LayerVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
LayerVerb::make_action(Inkscape::UI::View::View *view)
{
    return make_action_helper(view, &vector);
}

/** \brief  Create an action for a \c ObjectVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
ObjectVerb::make_action(Inkscape::UI::View::View *view)
{
    return make_action_helper(view, &vector);
}

/** \brief  Create an action for a \c ContextVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
ContextVerb::make_action(Inkscape::UI::View::View *view)
{
    return make_action_helper(view, &vector);
}

/** \brief  Create an action for a \c ZoomVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
ZoomVerb::make_action(Inkscape::UI::View::View *view)
{
    return make_action_helper(view, &vector);
}

/** \brief  Create an action for a \c DialogVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
DialogVerb::make_action(Inkscape::UI::View::View *view)
{
    return make_action_helper(view, &vector);
}

/** \brief  Create an action for a \c HelpVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
HelpVerb::make_action(Inkscape::UI::View::View *view)
{
    return make_action_helper(view, &vector);
}

/** \brief  Create an action for a \c TutorialVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
TutorialVerb::make_action(Inkscape::UI::View::View *view)
{
    return make_action_helper(view, &vector);
}

/** \brief  Create an action for a \c TextVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
TextVerb::make_action(Inkscape::UI::View::View *view)
{
    return make_action_helper(view, &vector);
}

/** \brief A quick little convience function to make building actions
           a little bit easier.
    \param  view    Which view the action should be created for.
    \param  vector  The function vector for the verb.
    \return The created action.

    This function does a couple of things.  The most obvious is that
    it allocates and creates the action.  When it does this it
    translates the \c _name and \c _tip variables.  This allows them
    to be staticly allocated easily, and get translated in the end.  Then,
    if the action gets crated, a listener is added to the action with
    the vector that is passed in.
*/
SPAction *
Verb::make_action_helper(Inkscape::UI::View::View *view, SPActionEventVector *vector, void *in_pntr)
{
    SPAction *action;

    //std::cout << "Adding action: " << _code << std::endl;
    action = sp_action_new(view, _id, _(_name),
                           _(_tip), _image, this);

    if (action != NULL) {
        if (in_pntr == NULL) {
            nr_active_object_add_listener(
                (NRActiveObject *) action,
                (NRObjectEventVector *) vector,
                sizeof(SPActionEventVector),
                reinterpret_cast<void *>(_code)
            );
        } else {
            nr_active_object_add_listener(
                (NRActiveObject *) action,
                (NRObjectEventVector *) vector,
                sizeof(SPActionEventVector),
                in_pntr
            );
        }
    }

    return action;
}

/** \brief  A function to get an action if it exists, or otherwise to
            build it.
    \param  view  The view which this action would relate to
    \return The action, or NULL if there is an error.

    This function will get the action for a given view for this verb.  It
    will create the verb if it can't be found in the ActionTable.  Also,
    if the \c ActionTable has not been created, it gets created by this
    function.

    If the action is created, it's sensitivity must be determined.  The
    default for a new action is that it is sensitive.  If the value in
    \c _default_sensitive is \c false, then the sensitivity must be
    removed.  Also, if the view being created is based on the same
    document as a view already created, the sensitivity should be the
    same as views on that document.  A view with the same document is
    looked for, and the sensitivity is matched.  Unfortunately, this is
    currently a linear search.
*/
SPAction *
Verb::get_action(Inkscape::UI::View::View *view)
{
    SPAction *action = NULL;

    if ( _actions == NULL ) {
        _actions = new ActionTable;
    }
    ActionTable::iterator action_found = _actions->find(view);

    if (action_found != _actions->end()) {
        action = action_found->second;
    } else {
        action = this->make_action(view);

        // if (action == NULL) printf("Hmm, NULL in %s\n", _name);
        if (action == NULL) printf("Hmm, NULL in %s\n", _name);
        if (!_default_sensitive) {
            sp_action_set_sensitive(action, 0);
        } else {
            for (ActionTable::iterator cur_action = _actions->begin();
                 cur_action != _actions->end() && view != NULL;
                 cur_action++) {
                if (cur_action->first != NULL && cur_action->first->doc() == view->doc()) {
                    sp_action_set_sensitive(action, cur_action->second->sensitive);
                    break;
                }
            }
        }

        _actions->insert(ActionTable::value_type(view, action));
    }

    return action;
}

void
Verb::sensitive(SPDocument *in_doc, bool in_sensitive)
{
    // printf("Setting sensitivity of \"%s\" to %d\n", _name, in_sensitive);
    if (_actions != NULL) {
        for (ActionTable::iterator cur_action = _actions->begin();
             cur_action != _actions->end();
             cur_action++) {
            if (in_doc == NULL || (cur_action->first != NULL && cur_action->first->doc() == in_doc)) {
                sp_action_set_sensitive(cur_action->second, in_sensitive ? 1 : 0);
            }
        }
    }

    if (in_doc == NULL) {
        _default_sensitive = in_sensitive;
    }

    return;
}

/** \brief Accessor to get the tooltip for verb as localised string */
gchar const *Verb::get_tip(void)
{
    gchar const *result = 0;
    if (_tip) {
        unsigned int shortcut = sp_shortcut_get_primary(this);
        if ( (shortcut != _shortcut) || !_full_tip) {
            if (_full_tip) {
                g_free(_full_tip);
                _full_tip = 0;
            }
            _shortcut = shortcut;
            gchar* shortcutString = sp_shortcut_get_label(shortcut);
            if (shortcutString) {
                _full_tip = g_strdup_printf("%s (%s)", _(_tip), shortcutString);
                g_free(shortcutString);
                shortcutString = 0;
            } else {
	        _full_tip = g_strdup(_(_tip));
            }
        }
        result = _full_tip;
    }

    return result;
}

void
Verb::name(SPDocument *in_doc, Glib::ustring in_name)
{
    if (_actions != NULL) {
        for (ActionTable::iterator cur_action = _actions->begin();
             cur_action != _actions->end();
             cur_action++) {
            if (in_doc == NULL || (cur_action->first != NULL && cur_action->first->doc() == in_doc)) {
                sp_action_set_name(cur_action->second, in_name);
            }
        }
    }
}

/** \brief  A function to remove the action associated with a view.
    \param  view  Which view's actions should be removed.
    \return None

    This function looks for the action in \c _actions.  If it is
    found then it is unreferenced and the entry in the action
    table is erased.
*/
void
Verb::delete_view(Inkscape::UI::View::View *view)
{
    if (_actions == NULL) return;
    if (_actions->empty()) return;

#if 0
    static int count = 0;
    std::cout << count++ << std::endl;
#endif

    ActionTable::iterator action_found = _actions->find(view);

    if (action_found != _actions->end()) {
        SPAction *action = action_found->second;
        nr_object_unref(NR_OBJECT(action));
        _actions->erase(action_found);
    }

    return;
}

/** \brief  A function to delete a view from all verbs
    \param  view  Which view's actions should be removed.
    \return None

    This function first looks through _base_verbs and deteles
    the view from all of those views.  If \c _verbs is not empty
    then all of the entries in that table have all of the views
    deleted also.
*/
void
Verb::delete_all_view(Inkscape::UI::View::View *view)
{
    for (int i = 0; i <= SP_VERB_LAST; i++) {
        if (_base_verbs[i])
          _base_verbs[i]->delete_view(view);
    }

    if (!_verbs.empty()) {
        for (VerbTable::iterator thisverb = _verbs.begin();
             thisverb != _verbs.end(); thisverb++) {
            Inkscape::Verb *verbpntr = thisverb->second;
            // std::cout << "Delete In Verb: " << verbpntr->_name << std::endl;
            verbpntr->delete_view(view);
        }
    }

    return;
}

/** \brief  A function to turn a \c code into a Verb for dynamically
            created Verbs.
    \param  code  What code is being looked for
    \return The found Verb of NULL if none is found.

    This function basically just looks through the \c _verbs hash
    table.  STL does all the work.
*/
Verb *
Verb::get_search(unsigned int code)
{
    Verb *verb = NULL;
    VerbTable::iterator verb_found = _verbs.find(code);

    if (verb_found != _verbs.end()) {
        verb = verb_found->second;
    }

    return verb;
}

/** \brief  Find a Verb using it's ID
    \param  id  Which id to search for

    This function uses the \c _verb_ids has table to find the
    verb by it's id.  Should be much faster than previous
    implementations.
*/
Verb *
Verb::getbyid(gchar const *id)
{
    Verb *verb = NULL;
    VerbIDTable::iterator verb_found = _verb_ids.find(id);

    if (verb_found != _verb_ids.end()) {
        verb = verb_found->second;
    }

    if (verb == NULL)
        printf("Unable to find: %s\n", id);

    return verb;
}

/** \brief  Decode the verb code and take appropriate action */
void
FileVerb::perform(SPAction *action, void *data, void */*pdata*/)
{
#if 0
    /* These aren't used, but are here to remind people not to use
       the CURRENT_DOCUMENT macros unless they really have to. */
    Inkscape::UI::View::View *current_view = sp_action_get_view(action);
    SPDocument *current_document = current_view->doc();
#endif

    SPDesktop *desktop = dynamic_cast<SPDesktop*>(sp_action_get_view(action));
    g_assert(desktop != NULL);
    Gtk::Window *parent = desktop->getToplevel();
    g_assert(parent != NULL);

    switch (reinterpret_cast<std::size_t>(data)) {
        case SP_VERB_FILE_NEW:
            sp_file_new_default();
            break;
        case SP_VERB_FILE_OPEN:
            sp_file_open_dialog(*parent, NULL, NULL);
            break;
        case SP_VERB_FILE_REVERT:
            sp_file_revert_dialog();
            break;
        case SP_VERB_FILE_SAVE:
            sp_file_save(*parent, NULL, NULL);
            break;
        case SP_VERB_FILE_SAVE_AS:
            sp_file_save_as(*parent, NULL, NULL);
            break;
        case SP_VERB_FILE_SAVE_A_COPY:
            sp_file_save_a_copy(*parent, NULL, NULL);
            break;
        case SP_VERB_FILE_PRINT:
            sp_file_print(*parent);
            break;
        case SP_VERB_FILE_VACUUM:
            sp_file_vacuum();
            break;
        case SP_VERB_FILE_PRINT_PREVIEW:
            sp_file_print_preview(NULL, NULL);
            break;
        case SP_VERB_FILE_IMPORT:
            sp_file_import(*parent);
            break;
        case SP_VERB_FILE_EXPORT:
            sp_file_export_dialog(*parent);
            break;
        case SP_VERB_FILE_IMPORT_FROM_OCAL:
            sp_file_import_from_ocal(*parent);
            break;
//        case SP_VERB_FILE_EXPORT_TO_OCAL:
//            sp_file_export_to_ocal(*parent);
//            break;
        case SP_VERB_FILE_NEXT_DESKTOP:
            inkscape_switch_desktops_next();
            break;
        case SP_VERB_FILE_PREV_DESKTOP:
            inkscape_switch_desktops_prev();
            break;
        case SP_VERB_FILE_CLOSE_VIEW:
            sp_ui_close_view(NULL);
            break;
        case SP_VERB_FILE_QUIT:
            sp_file_exit();
            break;
        default:
            break;
    }


} // end of sp_verb_action_file_perform()

/** \brief  Decode the verb code and take appropriate action */
void
EditVerb::perform(SPAction *action, void *data, void */*pdata*/)
{
    SPDesktop *dt = static_cast<SPDesktop*>(sp_action_get_view(action));
    if (!dt)
        return;

    switch (reinterpret_cast<std::size_t>(data)) {
        case SP_VERB_EDIT_UNDO:
            sp_undo(dt, sp_desktop_document(dt));
            break;
        case SP_VERB_EDIT_REDO:
            sp_redo(dt, sp_desktop_document(dt));
            break;
        case SP_VERB_EDIT_CUT:
            sp_selection_cut(dt);
            break;
        case SP_VERB_EDIT_COPY:
            sp_selection_copy(dt);
            break;
        case SP_VERB_EDIT_PASTE:
            sp_selection_paste(dt, false);
            break;
        case SP_VERB_EDIT_PASTE_STYLE:
            sp_selection_paste_style(dt);
            break;
        case SP_VERB_EDIT_PASTE_SIZE:
            sp_selection_paste_size(dt, true, true);
            break;
        case SP_VERB_EDIT_PASTE_SIZE_X:
            sp_selection_paste_size(dt, true, false);
            break;
        case SP_VERB_EDIT_PASTE_SIZE_Y:
            sp_selection_paste_size(dt, false, true);
            break;
        case SP_VERB_EDIT_PASTE_SIZE_SEPARATELY:
            sp_selection_paste_size_separately(dt, true, true);
            break;
        case SP_VERB_EDIT_PASTE_SIZE_SEPARATELY_X:
            sp_selection_paste_size_separately(dt, true, false);
            break;
        case SP_VERB_EDIT_PASTE_SIZE_SEPARATELY_Y:
            sp_selection_paste_size_separately(dt, false, true);
            break;
        case SP_VERB_EDIT_PASTE_IN_PLACE:
            sp_selection_paste(dt, true);
            break;
        case SP_VERB_EDIT_PASTE_LIVEPATHEFFECT:
            sp_selection_paste_livepatheffect(dt);
            break;
        case SP_VERB_EDIT_REMOVE_LIVEPATHEFFECT:
            sp_selection_remove_livepatheffect(dt);
            break;
        case SP_VERB_EDIT_REMOVE_FILTER:
            sp_selection_remove_filter(dt);
            break;
        case SP_VERB_EDIT_DELETE:
            sp_selection_delete(dt);
            break;
        case SP_VERB_EDIT_DUPLICATE:
            sp_selection_duplicate(dt);
            break;
        case SP_VERB_EDIT_CLONE:
            sp_selection_clone(dt);
            break;
        case SP_VERB_EDIT_UNLINK_CLONE:
            sp_selection_unlink(dt);
            break;
        case SP_VERB_EDIT_RELINK_CLONE:
            sp_selection_relink(dt);
            break;
        case SP_VERB_EDIT_CLONE_SELECT_ORIGINAL:
            sp_select_clone_original(dt);
            break;
        case SP_VERB_EDIT_SELECTION_2_MARKER:
            sp_selection_to_marker(dt);
            break;
        case SP_VERB_EDIT_SELECTION_2_GUIDES:
            sp_selection_to_guides(dt);
            break;
        case SP_VERB_EDIT_TILE:
            sp_selection_tile(dt);
            break;
        case SP_VERB_EDIT_UNTILE:
            sp_selection_untile(dt);
            break;
        case SP_VERB_EDIT_CLEAR_ALL:
            sp_edit_clear_all(dt);
            break;
        case SP_VERB_EDIT_SELECT_ALL:
            SelectionHelper::selectAll(dt);
            break;
        case SP_VERB_EDIT_INVERT:
            SelectionHelper::invert(dt);
            break;
        case SP_VERB_EDIT_SELECT_ALL_IN_ALL_LAYERS:
            SelectionHelper::selectAllInAll(dt);
            break;
        case SP_VERB_EDIT_INVERT_IN_ALL_LAYERS:
            SelectionHelper::invertAllInAll(dt);
            break;
        case SP_VERB_EDIT_SELECT_NEXT:
            SelectionHelper::selectNext(dt);
            break;
        case SP_VERB_EDIT_SELECT_PREV:
            SelectionHelper::selectPrev(dt);
            break;
        case SP_VERB_EDIT_DESELECT:
            SelectionHelper::selectNone(dt);
            break;
        case SP_VERB_EDIT_GUIDES_AROUND_PAGE:
            sp_guide_create_guides_around_page(dt);
            break;

        case SP_VERB_EDIT_NEXT_PATHEFFECT_PARAMETER:
            sp_selection_next_patheffect_param(dt);
            break;
        case SP_VERB_EDIT_LINK_COLOR_PROFILE:
            break;
        case SP_VERB_EDIT_REMOVE_COLOR_PROFILE:
            break;
        default:
            break;
    }

} // end of sp_verb_action_edit_perform()

/** \brief  Decode the verb code and take appropriate action */
void
SelectionVerb::perform(SPAction *action, void *data, void */*pdata*/)
{
    SPDesktop *dt = static_cast<SPDesktop*>(sp_action_get_view(action));

    if (!dt)
        return;

    g_assert(dt->_dlg_mgr != NULL);

    switch (reinterpret_cast<std::size_t>(data)) {
        case SP_VERB_SELECTION_TO_FRONT:
            sp_selection_raise_to_top(dt);
            break;
        case SP_VERB_SELECTION_TO_BACK:
            sp_selection_lower_to_bottom(dt);
            break;
        case SP_VERB_SELECTION_RAISE:
            sp_selection_raise(dt);
            break;
        case SP_VERB_SELECTION_LOWER:
            sp_selection_lower(dt);
            break;
        case SP_VERB_SELECTION_GROUP:
            sp_selection_group(dt);
            break;
        case SP_VERB_SELECTION_UNGROUP:
            sp_selection_ungroup(dt);
            break;

        case SP_VERB_SELECTION_TEXTTOPATH:
            text_put_on_path();
            break;
        case SP_VERB_SELECTION_TEXTFROMPATH:
            text_remove_from_path();
            break;
        case SP_VERB_SELECTION_REMOVE_KERNS:
            text_remove_all_kerns();
            break;

        case SP_VERB_SELECTION_UNION:
            sp_selected_path_union(dt);
            break;
        case SP_VERB_SELECTION_INTERSECT:
            sp_selected_path_intersect(dt);
            break;
        case SP_VERB_SELECTION_DIFF:
            sp_selected_path_diff(dt);
            break;
        case SP_VERB_SELECTION_SYMDIFF:
            sp_selected_path_symdiff(dt);
            break;

        case SP_VERB_SELECTION_CUT:
            sp_selected_path_cut(dt);
            break;
        case SP_VERB_SELECTION_SLICE:
            sp_selected_path_slice(dt);
            break;

        case SP_VERB_SELECTION_OFFSET:
            sp_selected_path_offset(dt);
            break;
        case SP_VERB_SELECTION_OFFSET_SCREEN:
            sp_selected_path_offset_screen(dt, 1);
            break;
        case SP_VERB_SELECTION_OFFSET_SCREEN_10:
            sp_selected_path_offset_screen(dt, 10);
            break;
        case SP_VERB_SELECTION_INSET:
            sp_selected_path_inset(dt);
            break;
        case SP_VERB_SELECTION_INSET_SCREEN:
            sp_selected_path_inset_screen(dt, 1);
            break;
        case SP_VERB_SELECTION_INSET_SCREEN_10:
            sp_selected_path_inset_screen(dt, 10);
            break;
        case SP_VERB_SELECTION_DYNAMIC_OFFSET:
            sp_selected_path_create_offset_object_zero(dt);
            tools_switch(dt, TOOLS_NODES);
            break;
        case SP_VERB_SELECTION_LINKED_OFFSET:
            sp_selected_path_create_updating_offset_object_zero(dt);
            tools_switch(dt, TOOLS_NODES);
            break;
        case SP_VERB_SELECTION_OUTLINE:
            sp_selected_path_outline(dt);
            break;
        case SP_VERB_SELECTION_SIMPLIFY:
            sp_selected_path_simplify(dt);
            break;
        case SP_VERB_SELECTION_REVERSE:
            SelectionHelper::reverse(dt);
            break;
        case SP_VERB_SELECTION_TRACE:
            inkscape_dialogs_unhide();
            dt->_dlg_mgr->showDialog("Trace");
            break;
        case SP_VERB_SELECTION_CREATE_BITMAP:
            sp_selection_create_bitmap_copy(dt);
            break;

        case SP_VERB_SELECTION_COMBINE:
            sp_selected_path_combine(dt);
            break;
        case SP_VERB_SELECTION_BREAK_APART:
            sp_selected_path_break_apart(dt);
            break;
        case SP_VERB_SELECTION_GRIDTILE:
            inkscape_dialogs_unhide();
            dt->_dlg_mgr->showDialog("TileDialog");
            break;
        default:
            break;
    }

} // end of sp_verb_action_selection_perform()

/** \brief  Decode the verb code and take appropriate action */
void
LayerVerb::perform(SPAction *action, void *data, void */*pdata*/)
{
    SPDesktop *dt = static_cast<SPDesktop*>(sp_action_get_view(action));
    size_t verb = reinterpret_cast<std::size_t>(data);

    if ( !dt || !dt->currentLayer() ) {
        return;
    }

    switch (verb) {
        case SP_VERB_LAYER_NEW: {
            Inkscape::UI::Dialogs::LayerPropertiesDialog::showCreate(dt, dt->currentLayer());
            break;
        }
        case SP_VERB_LAYER_RENAME: {
            Inkscape::UI::Dialogs::LayerPropertiesDialog::showRename(dt, dt->currentLayer());
            break;
        }
        case SP_VERB_LAYER_NEXT: {
            SPObject *next=Inkscape::next_layer(dt->currentRoot(), dt->currentLayer());
            if (next) {
                dt->setCurrentLayer(next);
                sp_document_done(sp_desktop_document(dt), SP_VERB_LAYER_NEXT,
                                 _("Switch to next layer"));
                dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Switched to next layer."));
            } else {
                dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Cannot go past last layer."));
            }
            break;
        }
        case SP_VERB_LAYER_PREV: {
            SPObject *prev=Inkscape::previous_layer(dt->currentRoot(), dt->currentLayer());
            if (prev) {
                dt->setCurrentLayer(prev);
                sp_document_done(sp_desktop_document(dt), SP_VERB_LAYER_PREV,
                                 _("Switch to previous layer"));
                dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Switched to previous layer."));
            } else {
                dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Cannot go before first layer."));
            }
            break;
        }
        case SP_VERB_LAYER_MOVE_TO_NEXT: {
            sp_selection_to_next_layer(dt);
            break;
        }
        case SP_VERB_LAYER_MOVE_TO_PREV: {
            sp_selection_to_prev_layer(dt);
            break;
        }
        case SP_VERB_LAYER_TO_TOP:
        case SP_VERB_LAYER_TO_BOTTOM:
        case SP_VERB_LAYER_RAISE:
        case SP_VERB_LAYER_LOWER: {
            if ( dt->currentLayer() == dt->currentRoot() ) {
                dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
                return;
            }

            SPItem *layer=SP_ITEM(dt->currentLayer());
            g_return_if_fail(layer != NULL);

            SPObject *old_pos=SP_OBJECT_NEXT(layer);

            switch (verb) {
                case SP_VERB_LAYER_TO_TOP:
                    layer->raiseToTop();
                    break;
                case SP_VERB_LAYER_TO_BOTTOM:
                    layer->lowerToBottom();
                    break;
                case SP_VERB_LAYER_RAISE:
                    layer->raiseOne();
                    break;
                case SP_VERB_LAYER_LOWER:
                    layer->lowerOne();
                    break;
            }

            if ( SP_OBJECT_NEXT(layer) != old_pos ) {
                char const *message = NULL;
                Glib::ustring description = "";
                switch (verb) {
                    case SP_VERB_LAYER_TO_TOP:
                        message = g_strdup_printf(_("Raised layer <b>%s</b>."), layer->defaultLabel());
                        description = _("Layer to top");
                        break;
                    case SP_VERB_LAYER_RAISE:
                        message = g_strdup_printf(_("Raised layer <b>%s</b>."), layer->defaultLabel());
                        description = _("Raise layer");
                        break;
                    case SP_VERB_LAYER_TO_BOTTOM:
                        message = g_strdup_printf(_("Lowered layer <b>%s</b>."), layer->defaultLabel());
                        description = _("Layer to bottom");
                        break;
                    case SP_VERB_LAYER_LOWER:
                        message = g_strdup_printf(_("Lowered layer <b>%s</b>."), layer->defaultLabel());
                        description = _("Lower layer");
                        break;
                };
                sp_document_done(sp_desktop_document(dt), verb, description);
                if (message) {
                    dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, message);
                    g_free((void *) message);
                }
            } else {
                dt->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Cannot move layer any further."));
            }

            break;
        }
        case SP_VERB_LAYER_DUPLICATE: {
            if ( dt->currentLayer() != dt->currentRoot() ) {
                // Note with either approach:
                // Any clone masters are duplicated, their clones use the *original*,
                // but the duplicated master is not linked up as master nor clone of the original.
#if 0
                // Only copies selectable things, honoring locks, visibility, avoids sublayers.
                SPObject *new_layer = Inkscape::create_layer(dt->currentRoot(), dt->currentLayer(), LPOS_BELOW);
                if ( dt->currentLayer()->label() ) {
                    gchar* name = g_strdup_printf(_("%s copy"), dt->currentLayer()->label());
                    dt->layer_manager->renameLayer( new_layer, name, TRUE );
                    g_free(name);
                }
                sp_edit_select_all(dt);
                sp_selection_duplicate(dt, true);
                sp_selection_to_prev_layer(dt, true);
                dt->setCurrentLayer(new_layer);
                sp_edit_select_all(dt);
#else
                // Copies everything, regardless of locks, visibility, sublayers.
                Inkscape::XML::Node *selected = dt->currentLayer()->repr;
                Inkscape::XML::Node *parent = sp_repr_parent(selected);
                Inkscape::XML::Node *dup = selected->duplicate(parent->document());
                parent->addChild(dup, selected);
                SPObject *new_layer = dt->currentLayer()->next;
                if (new_layer) {
                    if (new_layer->label()) {
                        gchar* name = g_strdup_printf(_("%s copy"), new_layer->label());
                        dt->layer_manager->renameLayer( new_layer, name, TRUE );
                        g_free(name);
                    }
                    dt->setCurrentLayer(new_layer);
                }
#endif
                sp_document_done(sp_desktop_document(dt), SP_VERB_LAYER_DUPLICATE,
                                 _("Duplicate layer"));

                // TRANSLATORS: this means "The layer has been duplicated."
                dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Duplicated layer."));
            } else {
                dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
            }
            break;
        }
        case SP_VERB_LAYER_DELETE: {
            if ( dt->currentLayer() != dt->currentRoot() ) {
                sp_desktop_selection(dt)->clear();
                SPObject *old_layer=dt->currentLayer();

                sp_object_ref(old_layer, NULL);
                SPObject *survivor=Inkscape::next_layer(dt->currentRoot(), old_layer);
                if (!survivor) {
                    survivor = Inkscape::previous_layer(dt->currentRoot(), old_layer);
                }

                if (survivor == old_layer->lastChild()) {
                    // oops: layer_fns messed up. BADLY.
                    survivor = NULL;
                }

                /* Deleting the old layer before switching layers is a hack to trigger the
                 * listeners of the deletion event (as happens when old_layer is deleted using the
                 * xml editor).  See
                 * http://sourceforge.net/tracker/index.php?func=detail&aid=1339397&group_id=93438&atid=604306
                 */
                old_layer->deleteObject();
                sp_object_unref(old_layer, NULL);
                if (survivor) {
                    dt->setCurrentLayer(survivor);
                }

                sp_document_done(sp_desktop_document(dt), SP_VERB_LAYER_DELETE,
                                 _("Delete layer"));

                // TRANSLATORS: this means "The layer has been deleted."
                dt->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Deleted layer."));
            } else {
                dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
            }
            break;
        }
        case SP_VERB_LAYER_SOLO: {
            if ( dt->currentLayer() == dt->currentRoot() ) {
                dt->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("No current layer."));
            } else {
                dt->toggleLayerSolo( dt->currentLayer() );
                sp_document_maybe_done(sp_desktop_document(dt), "layer:solo", SP_VERB_LAYER_SOLO, _("Toggle layer solo"));
            }
            break;
        }
    }

    return;
} // end of sp_verb_action_layer_perform()

/** \brief  Decode the verb code and take appropriate action */
void
ObjectVerb::perform( SPAction *action, void *data, void */*pdata*/ )
{
    SPDesktop *dt = static_cast<SPDesktop*>(sp_action_get_view(action));
    if (!dt)
        return;

    SPEventContext *ec = dt->event_context;

    Inkscape::Selection *sel = sp_desktop_selection(dt);

    if (sel->isEmpty())
        return;

    Geom::OptRect bbox = sel->bounds();
    if (!bbox) {
        return;
    }
    // If the rotation center of the selection is visible, choose it as reference point
    // for horizontal and vertical flips. Otherwise, take the center of the bounding box.
    Geom::Point center;
    if (tools_isactive(dt, TOOLS_SELECT) && sel->center() && SP_SELECT_CONTEXT(ec)->_seltrans->centerIsVisible())
        center = *sel->center();
    else
        center = bbox->midpoint();

    switch (reinterpret_cast<std::size_t>(data)) {
        case SP_VERB_OBJECT_ROTATE_90_CW:
            sp_selection_rotate_90(dt, false);
            break;
        case SP_VERB_OBJECT_ROTATE_90_CCW:
            sp_selection_rotate_90(dt, true);
            break;
        case SP_VERB_OBJECT_FLATTEN:
            sp_selection_remove_transform(dt);
            break;
        case SP_VERB_OBJECT_TO_CURVE:
            sp_selected_path_to_curves(dt);
            break;
        case SP_VERB_OBJECT_FLOW_TEXT:
            text_flow_into_shape();
            break;
        case SP_VERB_OBJECT_UNFLOW_TEXT:
            text_unflow();
            break;
        case SP_VERB_OBJECT_FLOWTEXT_TO_TEXT:
            flowtext_to_text();
            break;
        case SP_VERB_OBJECT_FLIP_HORIZONTAL:
            sp_selection_scale_relative(sel, center, Geom::Scale(-1.0, 1.0));
            sp_document_done(sp_desktop_document(dt), SP_VERB_OBJECT_FLIP_HORIZONTAL,
                             _("Flip horizontally"));
            break;
        case SP_VERB_OBJECT_FLIP_VERTICAL:
            sp_selection_scale_relative(sel, center, Geom::Scale(1.0, -1.0));
            sp_document_done(sp_desktop_document(dt), SP_VERB_OBJECT_FLIP_VERTICAL,
                             _("Flip vertically"));
            break;
        case SP_VERB_OBJECT_SET_MASK:
            sp_selection_set_mask(dt, false, false);
            break;
        case SP_VERB_OBJECT_EDIT_MASK:
            sp_selection_edit_clip_or_mask(dt, false);
            break;
        case SP_VERB_OBJECT_UNSET_MASK:
            sp_selection_unset_mask(dt, false);
            break;
        case SP_VERB_OBJECT_SET_CLIPPATH:
            sp_selection_set_mask(dt, true, false);
            break;
        case SP_VERB_OBJECT_EDIT_CLIPPATH:
            sp_selection_edit_clip_or_mask(dt, true);
            break;
        case SP_VERB_OBJECT_UNSET_CLIPPATH:
            sp_selection_unset_mask(dt, true);
            break;
        default:
            break;
    }

} // end of sp_verb_action_object_perform()

/** \brief  Decode the verb code and take appropriate action */
void
ContextVerb::perform(SPAction *action, void *data, void */*pdata*/)
{
    SPDesktop *dt;
    sp_verb_t verb;
    int vidx;

    dt = static_cast<SPDesktop*>(sp_action_get_view(action));

    if (!dt)
        return;

    verb = (sp_verb_t)GPOINTER_TO_INT((gpointer)data);

    /** \todo !!! hopefully this can go away soon and actions can look after
     * themselves
     */
    for (vidx = SP_VERB_CONTEXT_SELECT; vidx <= SP_VERB_CONTEXT_PAINTBUCKET_PREFS; vidx++)
    {
        SPAction *tool_action= get((sp_verb_t)vidx)->get_action(dt);
        if (tool_action) {
            sp_action_set_active(tool_action, vidx == (int)verb);
        }
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    switch (verb) {
        case SP_VERB_CONTEXT_SELECT:
            tools_switch(dt, TOOLS_SELECT);
            break;
        case SP_VERB_CONTEXT_NODE:
            tools_switch(dt, TOOLS_NODES);
            break;
        case SP_VERB_CONTEXT_TWEAK:
            tools_switch(dt, TOOLS_TWEAK);
            break;
        case SP_VERB_CONTEXT_SPRAY:
            tools_switch(dt, TOOLS_SPRAY);
            break;
        case SP_VERB_CONTEXT_RECT:
            tools_switch(dt, TOOLS_SHAPES_RECT);
            break;
        case SP_VERB_CONTEXT_3DBOX:
            tools_switch(dt, TOOLS_SHAPES_3DBOX);
            break;
        case SP_VERB_CONTEXT_ARC:
            tools_switch(dt, TOOLS_SHAPES_ARC);
            break;
        case SP_VERB_CONTEXT_STAR:
            tools_switch(dt, TOOLS_SHAPES_STAR);
            break;
        case SP_VERB_CONTEXT_SPIRAL:
            tools_switch(dt, TOOLS_SHAPES_SPIRAL);
            break;
        case SP_VERB_CONTEXT_PENCIL:
            tools_switch(dt, TOOLS_FREEHAND_PENCIL);
            break;
        case SP_VERB_CONTEXT_PEN:
            tools_switch(dt, TOOLS_FREEHAND_PEN);
            break;
        case SP_VERB_CONTEXT_CALLIGRAPHIC:
            tools_switch(dt, TOOLS_CALLIGRAPHIC);
            break;
        case SP_VERB_CONTEXT_TEXT:
            tools_switch(dt, TOOLS_TEXT);
            break;
        case SP_VERB_CONTEXT_GRADIENT:
            tools_switch(dt, TOOLS_GRADIENT);
            break;
        case SP_VERB_CONTEXT_ZOOM:
            tools_switch(dt, TOOLS_ZOOM);
            break;
        case SP_VERB_CONTEXT_DROPPER:
            tools_switch(dt, TOOLS_DROPPER);
            break;
        case SP_VERB_CONTEXT_CONNECTOR:
            tools_switch(dt,  TOOLS_CONNECTOR);
            break;
        case SP_VERB_CONTEXT_PAINTBUCKET:
            tools_switch(dt, TOOLS_PAINTBUCKET);
            break;
        case SP_VERB_CONTEXT_ERASER:
            tools_switch(dt, TOOLS_ERASER);
            break;
        case SP_VERB_CONTEXT_LPETOOL:
            tools_switch(dt, TOOLS_LPETOOL);
            break;

        case SP_VERB_CONTEXT_SELECT_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_SELECTOR);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_NODE_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_NODE);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_TWEAK_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_TWEAK);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_SPRAY_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_SPRAY);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_RECT_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_SHAPES_RECT);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_3DBOX_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_SHAPES_3DBOX);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_ARC_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_SHAPES_ELLIPSE);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_STAR_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_SHAPES_STAR);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_SPIRAL_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_SHAPES_SPIRAL);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_PENCIL_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_PENCIL);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_PEN_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_PEN);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_CALLIGRAPHIC_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_CALLIGRAPHY);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_TEXT_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_TEXT);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_GRADIENT_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_GRADIENT);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_ZOOM_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_ZOOM);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_DROPPER_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_DROPPER);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_CONNECTOR_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_CONNECTOR);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_PAINTBUCKET_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_PAINTBUCKET);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_ERASER_PREFS:
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_ERASER);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_CONTEXT_LPETOOL_PREFS:
            g_print ("TODO: Create preferences page for LPETool\n");
            prefs->setInt("/dialogs/preferences/page", PREFS_PAGE_TOOLS_LPETOOL);
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;

        default:
            break;
    }

} // end of sp_verb_action_ctx_perform()

/** \brief  Decode the verb code and take appropriate action */
void
TextVerb::perform(SPAction *action, void */*data*/, void */*pdata*/)
{
    SPDesktop *dt = static_cast<SPDesktop*>(sp_action_get_view(action));
    if (!dt)
        return;

    SPDocument *doc = sp_desktop_document(dt);
    (void)doc;
    Inkscape::XML::Node *repr = SP_OBJECT_REPR(dt->namedview);
    (void)repr;
}

/** \brief  Decode the verb code and take appropriate action */
void
ZoomVerb::perform(SPAction *action, void *data, void */*pdata*/)
{
    SPDesktop *dt = static_cast<SPDesktop*>(sp_action_get_view(action));
    if (!dt)
        return;
    SPEventContext *ec = dt->event_context;

    SPDocument *doc = sp_desktop_document(dt);

    Inkscape::XML::Node *repr = SP_OBJECT_REPR(dt->namedview);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble zoom_inc =
        prefs->getDoubleLimited( "/options/zoomincrement/value", 1.414213562, 1.01, 10 );

    switch (reinterpret_cast<std::size_t>(data)) {
        case SP_VERB_ZOOM_IN:
        {
            gint mul = 1 + gobble_key_events(
                 GDK_KP_Add, 0); // with any mask
            // While drawing with the pen/pencil tool, zoom towards the end of the unfinished path
            if (tools_isactive(dt, TOOLS_FREEHAND_PENCIL) || tools_isactive(dt, TOOLS_FREEHAND_PEN)) {
                SPCurve *rc = SP_DRAW_CONTEXT(ec)->red_curve;
                if (!rc->is_empty()) {
                    Geom::Point const zoom_to (*rc->last_point());
                    dt->zoom_relative_keep_point(zoom_to, mul*zoom_inc);
                    break;
                }
            }

            Geom::Rect const d = dt->get_display_area();
            dt->zoom_relative( d.midpoint()[Geom::X], d.midpoint()[Geom::Y], mul*zoom_inc);
            break;
        }
        case SP_VERB_ZOOM_OUT:
        {
            gint mul = 1 + gobble_key_events(
                 GDK_KP_Subtract, 0); // with any mask
            // While drawing with the pen/pencil tool, zoom away from the end of the unfinished path
            if (tools_isactive(dt, TOOLS_FREEHAND_PENCIL) || tools_isactive(dt, TOOLS_FREEHAND_PEN)) {
                SPCurve *rc = SP_DRAW_CONTEXT(ec)->red_curve;
                if (!rc->is_empty()) {
                    Geom::Point const zoom_to (*rc->last_point());
                    dt->zoom_relative_keep_point(zoom_to, 1 / (mul*zoom_inc));
                    break;
                }
            }

            Geom::Rect const d = dt->get_display_area();
            dt->zoom_relative( d.midpoint()[Geom::X], d.midpoint()[Geom::Y], 1 / (mul*zoom_inc) );
            break;
        }
        case SP_VERB_ZOOM_1_1:
        {
            double zcorr = prefs->getDouble("/options/zoomcorrection/value", 1.0);
            Geom::Rect const d = dt->get_display_area();
            dt->zoom_absolute( d.midpoint()[Geom::X], d.midpoint()[Geom::Y], 1.0 * zcorr );
            break;
        }
        case SP_VERB_ZOOM_1_2:
        {
            double zcorr = prefs->getDouble("/options/zoomcorrection/value", 1.0);
            Geom::Rect const d = dt->get_display_area();
            dt->zoom_absolute( d.midpoint()[Geom::X], d.midpoint()[Geom::Y], 0.5 * zcorr );
            break;
        }
        case SP_VERB_ZOOM_2_1:
        {
            double zcorr = prefs->getDouble("/options/zoomcorrection/value", 1.0);
            Geom::Rect const d = dt->get_display_area();
            dt->zoom_absolute( d.midpoint()[Geom::X], d.midpoint()[Geom::Y], 2.0 * zcorr );
            break;
        }
        case SP_VERB_ZOOM_PAGE:
            dt->zoom_page();
            break;
        case SP_VERB_ZOOM_PAGE_WIDTH:
            dt->zoom_page_width();
            break;
        case SP_VERB_ZOOM_DRAWING:
            dt->zoom_drawing();
            break;
        case SP_VERB_ZOOM_SELECTION:
            dt->zoom_selection();
            break;
        case SP_VERB_ZOOM_NEXT:
            dt->next_zoom();
            break;
        case SP_VERB_ZOOM_PREV:
            dt->prev_zoom();
            break;
        case SP_VERB_TOGGLE_RULERS:
            dt->toggleRulers();
            break;
        case SP_VERB_TOGGLE_SCROLLBARS:
            dt->toggleScrollbars();
            break;
        case SP_VERB_TOGGLE_GUIDES:
            sp_namedview_toggle_guides(doc, repr);
            break;
        case SP_VERB_TOGGLE_SNAPPING:
            dt->toggleSnapGlobal();
            break;
        case SP_VERB_TOGGLE_GRID:
            dt->toggleGrids();
            break;
#ifdef HAVE_GTK_WINDOW_FULLSCREEN
        case SP_VERB_FULLSCREEN:
            dt->fullscreen();
            break;
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */
        case SP_VERB_FOCUSTOGGLE:
            dt->focusMode(!dt->is_focusMode());
            break;
        case SP_VERB_VIEW_NEW:
            sp_ui_new_view();
            break;
        case SP_VERB_VIEW_NEW_PREVIEW:
            sp_ui_new_view_preview();
            break;
        case SP_VERB_VIEW_MODE_NORMAL:
            dt->setDisplayModeNormal();
            break;
        case SP_VERB_VIEW_MODE_NO_FILTERS:
            dt->setDisplayModeNoFilters();
            break;
        case SP_VERB_VIEW_MODE_OUTLINE:
            dt->setDisplayModeOutline();
            break;
//        case SP_VERB_VIEW_MODE_PRINT_COLORS_PREVIEW:
//            dt->setDisplayModePrintColorsPreview();
//            break;
        case SP_VERB_VIEW_MODE_TOGGLE:
            dt->displayModeToggle();
            break;
        case SP_VERB_VIEW_CMS_TOGGLE:
            dt->toggleColorProfAdjust();
            break;
        case SP_VERB_VIEW_ICON_PREVIEW:
            inkscape_dialogs_unhide();
            dt->_dlg_mgr->showDialog("IconPreviewPanel");
            break;
        default:
            break;
    }

    dt->updateNow();

} // end of sp_verb_action_zoom_perform()

/** \brief  Decode the verb code and take appropriate action */
void
DialogVerb::perform(SPAction *action, void *data, void */*pdata*/)
{
    if (reinterpret_cast<std::size_t>(data) != SP_VERB_DIALOG_TOGGLE) {
        // unhide all when opening a new dialog
        inkscape_dialogs_unhide();
    }

    SPDesktop *dt = static_cast<SPDesktop*>(sp_action_get_view(action));
    g_assert(dt->_dlg_mgr != NULL);

    switch (reinterpret_cast<std::size_t>(data)) {
        case SP_VERB_DIALOG_DISPLAY:
            //sp_display_dialog();
            dt->_dlg_mgr->showDialog("InkscapePreferences");
            break;
        case SP_VERB_DIALOG_METADATA:
            // sp_desktop_dialog();
            dt->_dlg_mgr->showDialog("DocumentMetadata");
            break;
        case SP_VERB_DIALOG_NAMEDVIEW:
            // sp_desktop_dialog();
            dt->_dlg_mgr->showDialog("DocumentProperties");
            break;
        case SP_VERB_DIALOG_FILL_STROKE:
            dt->_dlg_mgr->showDialog("FillAndStroke");
            break;
        case SP_VERB_DIALOG_GLYPHS:
            dt->_dlg_mgr->showDialog("Glyphs");
            break;
        case SP_VERB_DIALOG_SWATCHES:
            dt->_dlg_mgr->showDialog("Swatches");
            break;
        case SP_VERB_DIALOG_TRANSFORM:
            dt->_dlg_mgr->showDialog("Transformation");
            break;
        case SP_VERB_DIALOG_ALIGN_DISTRIBUTE:
            dt->_dlg_mgr->showDialog("AlignAndDistribute");
            break;
        case SP_VERB_DIALOG_SPRAY_OPTION:
            dt->_dlg_mgr->showDialog("SprayOptionClass");
            break;
        case SP_VERB_DIALOG_TEXT:
            sp_text_edit_dialog();
            break;
        case SP_VERB_DIALOG_XML_EDITOR:
            sp_xml_tree_dialog();
            break;
        case SP_VERB_DIALOG_FIND:
            sp_find_dialog();
//              Please test the new find dialog if you have time:
//            dt->_dlg_mgr->showDialog("Find");
            break;
        case SP_VERB_DIALOG_FINDREPLACE:
            // not implemented yet
            break;
        case SP_VERB_DIALOG_SPELLCHECK:
            sp_spellcheck_dialog();
            break;
        case SP_VERB_DIALOG_DEBUG:
            dt->_dlg_mgr->showDialog("Messages");
            break;
        case SP_VERB_DIALOG_SCRIPT:
            //dt->_dlg_mgr->showDialog("Script");
            Inkscape::Bind::JavaBindery::getInstance()->showConsole();
            break;
        case SP_VERB_DIALOG_UNDO_HISTORY:
            dt->_dlg_mgr->showDialog("UndoHistory");
            break;
        case SP_VERB_DIALOG_TOGGLE:
            inkscape_dialogs_toggle();
            break;
        case SP_VERB_DIALOG_CLONETILER:
            clonetiler_dialog();
            break;
        case SP_VERB_DIALOG_ITEM:
            sp_item_dialog();
            break;
/*#ifdef WITH_INKBOARD
        case SP_VERB_XMPP_CLIENT:
        {
            Inkscape::Whiteboard::SessionManager::showClient();
            break;
        }
#endif*/
        case SP_VERB_DIALOG_INPUT:
            dt->_dlg_mgr->showDialog("InputDevices");
            break;
        case SP_VERB_DIALOG_EXTENSIONEDITOR:
            dt->_dlg_mgr->showDialog("ExtensionEditor");
            break;
        case SP_VERB_DIALOG_LAYERS:
            dt->_dlg_mgr->showDialog("LayersPanel");
            break;
        case SP_VERB_DIALOG_LIVE_PATH_EFFECT:
            dt->_dlg_mgr->showDialog("LivePathEffect");
            break;
        case SP_VERB_DIALOG_FILTER_EFFECTS:
            dt->_dlg_mgr->showDialog("FilterEffectsDialog");
            break;
        case SP_VERB_DIALOG_SVG_FONTS:
            dt->_dlg_mgr->showDialog("SvgFontsDialog");
            break;
        case SP_VERB_DIALOG_PRINT_COLORS_PREVIEW:
            dt->_dlg_mgr->showDialog("PrintColorsPreviewDialog");
            break;
        default:
            break;
    }
} // end of sp_verb_action_dialog_perform()

/** \brief  Decode the verb code and take appropriate action */
void
HelpVerb::perform(SPAction *action, void *data, void */*pdata*/)
{
    SPDesktop *dt = static_cast<SPDesktop*>(sp_action_get_view(action));
    g_assert(dt->_dlg_mgr != NULL);

    switch (reinterpret_cast<std::size_t>(data)) {
        case SP_VERB_HELP_ABOUT:
            sp_help_about();
            break;
        case SP_VERB_HELP_ABOUT_EXTENSIONS: {
            // Inkscape::UI::Dialogs::ExtensionsPanel *panel = new Inkscape::UI::Dialogs::ExtensionsPanel();
            // panel->set_full(true);
            // show_panel( *panel, "dialogs.aboutextensions", SP_VERB_HELP_ABOUT_EXTENSIONS );
            break;
        }

        /*
        case SP_VERB_SHOW_LICENSE:
            // TRANSLATORS: See "tutorial-basic.svg" comment.
            sp_help_open_tutorial(NULL, (gpointer) _("gpl-2.svg"));
            break;
        */

        case SP_VERB_HELP_MEMORY:
            inkscape_dialogs_unhide();
            dt->_dlg_mgr->showDialog("Memory");
            break;
        default:
            break;
    }
} // end of sp_verb_action_help_perform()

/** \brief  Decode the verb code and take appropriate action */
void
TutorialVerb::perform(SPAction */*action*/, void *data, void */*pdata*/)
{
    switch (reinterpret_cast<std::size_t>(data)) {
        case SP_VERB_TUTORIAL_BASIC:
            /* TRANSLATORS: If you have translated the tutorial-basic.en.svgz file to your language,
               then translate this string as "tutorial-basic.LANG.svgz" (where LANG is your language
               code); otherwise leave as "tutorial-basic.svg". */
            sp_help_open_tutorial(NULL, (gpointer)_("tutorial-basic.svg"));
            break;
        case SP_VERB_TUTORIAL_SHAPES:
            // TRANSLATORS: See "tutorial-basic.svg" comment.
            sp_help_open_tutorial(NULL, (gpointer)_("tutorial-shapes.svg"));
            break;
        case SP_VERB_TUTORIAL_ADVANCED:
            // TRANSLATORS: See "tutorial-basic.svg" comment.
            sp_help_open_tutorial(NULL, (gpointer)_("tutorial-advanced.svg"));
            break;
        case SP_VERB_TUTORIAL_TRACING:
            // TRANSLATORS: See "tutorial-basic.svg" comment.
            sp_help_open_tutorial(NULL, (gpointer)_("tutorial-tracing.svg"));
            break;
        case SP_VERB_TUTORIAL_CALLIGRAPHY:
            // TRANSLATORS: See "tutorial-basic.svg" comment.
            sp_help_open_tutorial(NULL, (gpointer)_("tutorial-calligraphy.svg"));
            break;
        case SP_VERB_TUTORIAL_INTERPOLATE:
            // TRANSLATORS: See "tutorial-basic.svg" comment.
            sp_help_open_tutorial(NULL, (gpointer)_("tutorial-interpolate.svg"));
            break;
        case SP_VERB_TUTORIAL_DESIGN:
            // TRANSLATORS: See "tutorial-basic.svg" comment.
            sp_help_open_tutorial(NULL, (gpointer)_("tutorial-elements.svg"));
            break;
        case SP_VERB_TUTORIAL_TIPS:
            // TRANSLATORS: See "tutorial-basic.svg" comment.
            sp_help_open_tutorial(NULL, (gpointer)_("tutorial-tips.svg"));
            break;
        default:
            break;
    }
} // end of sp_verb_action_tutorial_perform()


/**
 * Action vector to define functions called if a staticly defined file verb
 * is called.
 */
SPActionEventVector FileVerb::vector =
            {{NULL},FileVerb::perform, NULL, NULL, NULL, NULL};
/**
 * Action vector to define functions called if a staticly defined edit verb is
 * called.
 */
SPActionEventVector EditVerb::vector =
            {{NULL},EditVerb::perform, NULL, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined selection
 * verb is called
 */
SPActionEventVector SelectionVerb::vector =
            {{NULL},SelectionVerb::perform, NULL, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined layer
 * verb is called
 */
SPActionEventVector LayerVerb::vector =
            {{NULL}, LayerVerb::perform, NULL, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined object
 * editing verb is called
 */
SPActionEventVector ObjectVerb::vector =
            {{NULL},ObjectVerb::perform, NULL, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined context
 * verb is called
 */
SPActionEventVector ContextVerb::vector =
            {{NULL},ContextVerb::perform, NULL, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined zoom verb
 * is called
 */
SPActionEventVector ZoomVerb::vector =
            {{NULL},ZoomVerb::perform, NULL, NULL, NULL, NULL};


/**
 * Action vector to define functions called if a staticly defined dialog verb
 * is called
 */
SPActionEventVector DialogVerb::vector =
            {{NULL},DialogVerb::perform, NULL, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined help verb
 * is called
 */
SPActionEventVector HelpVerb::vector =
            {{NULL},HelpVerb::perform, NULL, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined tutorial verb
 * is called
 */
SPActionEventVector TutorialVerb::vector =
            {{NULL},TutorialVerb::perform, NULL, NULL, NULL, NULL};

/**
 * Action vector to define functions called if a staticly defined tutorial verb
 * is called
 */
SPActionEventVector TextVerb::vector =
            {{NULL},TextVerb::perform, NULL, NULL, NULL, NULL};


/* *********** Effect Last ********** */

/** \brief A class to represent the last effect issued */
class EffectLastVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    EffectLastVerb(unsigned int const code,
                   gchar const *id,
                   gchar const *name,
                   gchar const *tip,
                   gchar const *image) :
        Verb(code, id, name, tip, image)
    {
        set_default_sensitive(false);
    }
}; /* EffectLastVerb class */

/**
 * The vector to attach in the last effect verb.
 */
SPActionEventVector EffectLastVerb::vector =
            {{NULL},EffectLastVerb::perform, NULL, NULL, NULL, NULL};

/** \brief  Create an action for a \c EffectLastVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
EffectLastVerb::make_action(Inkscape::UI::View::View *view)
{
    return make_action_helper(view, &vector);
}

/** \brief  Decode the verb code and take appropriate action */
void
EffectLastVerb::perform(SPAction *action, void *data, void */*pdata*/)
{
    /* These aren't used, but are here to remind people not to use
       the CURRENT_DOCUMENT macros unless they really have to. */
    Inkscape::UI::View::View *current_view = sp_action_get_view(action);
    // SPDocument *current_document = SP_VIEW_DOCUMENT(current_view);
    Inkscape::Extension::Effect *effect = Inkscape::Extension::Effect::get_last_effect();

    if (effect == NULL) return;
    if (current_view == NULL) return;

    switch (reinterpret_cast<std::size_t>(data)) {
        case SP_VERB_EFFECT_LAST_PREF:
            effect->prefs(current_view);
            break;
        case SP_VERB_EFFECT_LAST:
            effect->effect(current_view);
            break;
        default:
            return;
    }

    return;
}
/* *********** End Effect Last ********** */

/* *********** Fit Canvas ********** */

/** \brief A class to represent the canvas fitting verbs */
class FitCanvasVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    FitCanvasVerb(unsigned int const code,
                   gchar const *id,
                   gchar const *name,
                   gchar const *tip,
                   gchar const *image) :
        Verb(code, id, name, tip, image)
    {
        set_default_sensitive(false);
    }
}; /* FitCanvasVerb class */

/**
 * The vector to attach in the fit canvas verb.
 */
SPActionEventVector FitCanvasVerb::vector =
            {{NULL},FitCanvasVerb::perform, NULL, NULL, NULL, NULL};

/** \brief  Create an action for a \c FitCanvasVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
FitCanvasVerb::make_action(Inkscape::UI::View::View *view)
{
    SPAction *action = make_action_helper(view, &vector);
    return action;
}

/** \brief  Decode the verb code and take appropriate action */
void
FitCanvasVerb::perform(SPAction *action, void *data, void */*pdata*/)
{
    SPDesktop *dt = static_cast<SPDesktop*>(sp_action_get_view(action));
    if (!dt) return;
    SPDocument *doc = sp_desktop_document(dt);
    if (!doc) return;

    switch (reinterpret_cast<std::size_t>(data)) {
        case SP_VERB_FIT_CANVAS_TO_SELECTION:
            verb_fit_canvas_to_selection(dt);
            break;
        case SP_VERB_FIT_CANVAS_TO_DRAWING:
            verb_fit_canvas_to_drawing(dt);
            break;
        case SP_VERB_FIT_CANVAS_TO_SELECTION_OR_DRAWING:
            fit_canvas_to_selection_or_drawing(dt);
            break;
        default:
            return;
    }

    return;
}
/* *********** End Fit Canvas ********** */


/* *********** Lock'N'Hide ********** */

/** \brief A class to represent the object unlocking and unhiding verbs */
class LockAndHideVerb : public Verb {
private:
    static void perform(SPAction *action, void *mydata, void *otherdata);
    static SPActionEventVector vector;
protected:
    virtual SPAction *make_action(Inkscape::UI::View::View *view);
public:
    /** \brief Use the Verb initializer with the same parameters. */
    LockAndHideVerb(unsigned int const code,
                   gchar const *id,
                   gchar const *name,
                   gchar const *tip,
                   gchar const *image) :
        Verb(code, id, name, tip, image)
    {
        set_default_sensitive(true);
    }
}; /* LockAndHideVerb class */

/**
 * The vector to attach in the lock'n'hide verb.
 */
SPActionEventVector LockAndHideVerb::vector =
            {{NULL},LockAndHideVerb::perform, NULL, NULL, NULL, NULL};

/** \brief  Create an action for a \c LockAndHideVerb
    \param  view  Which view the action should be created for
    \return The built action.

    Calls \c make_action_helper with the \c vector.
*/
SPAction *
LockAndHideVerb::make_action(Inkscape::UI::View::View *view)
{
    SPAction *action = make_action_helper(view, &vector);
    return action;
}

/** \brief  Decode the verb code and take appropriate action */
void
LockAndHideVerb::perform(SPAction *action, void *data, void */*pdata*/)
{
    SPDesktop *dt = static_cast<SPDesktop*>(sp_action_get_view(action));
    if (!dt) return;
    SPDocument *doc = sp_desktop_document(dt);
    if (!doc) return;

    switch (reinterpret_cast<std::size_t>(data)) {
        case SP_VERB_UNLOCK_ALL:
            unlock_all(dt);
            sp_document_done(doc, SP_VERB_UNLOCK_ALL, _("Unlock all objects in the current layer"));
            break;
        case SP_VERB_UNLOCK_ALL_IN_ALL_LAYERS:
            unlock_all_in_all_layers(dt);
            sp_document_done(doc, SP_VERB_UNLOCK_ALL_IN_ALL_LAYERS, _("Unlock all objects in all layers"));
            break;
        case SP_VERB_UNHIDE_ALL:
            unhide_all(dt);
            sp_document_done(doc, SP_VERB_UNHIDE_ALL, _("Unhide all objects in the current layer"));
            break;
        case SP_VERB_UNHIDE_ALL_IN_ALL_LAYERS:
            unhide_all_in_all_layers(dt);
            sp_document_done(doc, SP_VERB_UNHIDE_ALL_IN_ALL_LAYERS, _("Unhide all objects in all layers"));
            break;
        default:
            return;
    }

    return;
}
/* *********** End Lock'N'Hide ********** */


/* these must be in the same order as the SP_VERB_* enum in "verbs.h" */
Verb *Verb::_base_verbs[] = {
    /* Header */
    new Verb(SP_VERB_INVALID, NULL, NULL, NULL, NULL),
    new Verb(SP_VERB_NONE, "None", N_("None"), N_("Does nothing"), NULL),

    /* File */
    new FileVerb(SP_VERB_FILE_NEW, "FileNew", N_("Default"), N_("Create new document from the default template"),
                 GTK_STOCK_NEW ),
    new FileVerb(SP_VERB_FILE_OPEN, "FileOpen", N_("_Open..."),
                 N_("Open an existing document"), GTK_STOCK_OPEN ),
    new FileVerb(SP_VERB_FILE_REVERT, "FileRevert", N_("Re_vert"),
                 N_("Revert to the last saved version of document (changes will be lost)"), GTK_STOCK_REVERT_TO_SAVED ),
    new FileVerb(SP_VERB_FILE_SAVE, "FileSave", N_("_Save"), N_("Save document"),
                 GTK_STOCK_SAVE ),
    new FileVerb(SP_VERB_FILE_SAVE_AS, "FileSaveAs", N_("Save _As..."),
                 N_("Save document under a new name"), GTK_STOCK_SAVE_AS ),
    new FileVerb(SP_VERB_FILE_SAVE_A_COPY, "FileSaveACopy", N_("Save a Cop_y..."),
                 N_("Save a copy of the document under a new name"), NULL ),
    new FileVerb(SP_VERB_FILE_PRINT, "FilePrint", N_("_Print..."), N_("Print document"),
                 GTK_STOCK_PRINT ),
    // TRANSLATORS: "Vacuum Defs" means "Clean up defs" (so as to remove unused definitions)
    new FileVerb(SP_VERB_FILE_VACUUM, "FileVacuum", N_("Vac_uum Defs"), N_("Remove unused definitions (such as gradients or clipping paths) from the &lt;defs&gt; of the document"),
                 INKSCAPE_ICON_DOCUMENT_CLEANUP ),
    new FileVerb(SP_VERB_FILE_PRINT_PREVIEW, "FilePrintPreview", N_("Print Previe_w"),
                 N_("Preview document printout"), GTK_STOCK_PRINT_PREVIEW ),
    new FileVerb(SP_VERB_FILE_IMPORT, "FileImport", N_("_Import..."),
                 N_("Import a bitmap or SVG image into this document"), INKSCAPE_ICON_DOCUMENT_IMPORT),
    new FileVerb(SP_VERB_FILE_EXPORT, "FileExport", N_("_Export Bitmap..."),
                 N_("Export this document or a selection as a bitmap image"), INKSCAPE_ICON_DOCUMENT_EXPORT),
    new FileVerb(SP_VERB_FILE_IMPORT_FROM_OCAL, "FileImportFromOCAL", N_("Import From Open Clip Art Library"), N_("Import a document from Open Clip Art Library"), INKSCAPE_ICON_DOCUMENT_IMPORT_OCAL),
//    new FileVerb(SP_VERB_FILE_EXPORT_TO_OCAL, "FileExportToOCAL", N_("Export To Open Clip Art Library"), N_("Export this document to Open Clip Art Library"), INKSCAPE_ICON_DOCUMENT_EXPORT_OCAL),
    new FileVerb(SP_VERB_FILE_NEXT_DESKTOP, "NextWindow", N_("N_ext Window"),
                 N_("Switch to the next document window"), INKSCAPE_ICON_WINDOW_NEXT),
    new FileVerb(SP_VERB_FILE_PREV_DESKTOP, "PrevWindow", N_("P_revious Window"),
                 N_("Switch to the previous document window"), INKSCAPE_ICON_WINDOW_PREVIOUS),
    new FileVerb(SP_VERB_FILE_CLOSE_VIEW, "FileClose", N_("_Close"),
                 N_("Close this document window"), GTK_STOCK_CLOSE),
    new FileVerb(SP_VERB_FILE_QUIT, "FileQuit", N_("_Quit"), N_("Quit Inkscape"), GTK_STOCK_QUIT),

    /* Edit */
    new EditVerb(SP_VERB_EDIT_UNDO, "EditUndo", N_("_Undo"), N_("Undo last action"),
                 GTK_STOCK_UNDO),
    new EditVerb(SP_VERB_EDIT_REDO, "EditRedo", N_("_Redo"),
                 N_("Do again the last undone action"), GTK_STOCK_REDO),
    new EditVerb(SP_VERB_EDIT_CUT, "EditCut", N_("Cu_t"),
                 N_("Cut selection to clipboard"), GTK_STOCK_CUT),
    new EditVerb(SP_VERB_EDIT_COPY, "EditCopy", N_("_Copy"),
                 N_("Copy selection to clipboard"), GTK_STOCK_COPY),
    new EditVerb(SP_VERB_EDIT_PASTE, "EditPaste", N_("_Paste"),
                 N_("Paste objects from clipboard to mouse point, or paste text"), GTK_STOCK_PASTE),
    new EditVerb(SP_VERB_EDIT_PASTE_STYLE, "EditPasteStyle", N_("Paste _Style"),
                 N_("Apply the style of the copied object to selection"), INKSCAPE_ICON_EDIT_PASTE_STYLE),
    new EditVerb(SP_VERB_EDIT_PASTE_SIZE, "EditPasteSize", N_("Paste Si_ze"),
                 N_("Scale selection to match the size of the copied object"), NULL),
    new EditVerb(SP_VERB_EDIT_PASTE_SIZE_X, "EditPasteWidth", N_("Paste _Width"),
                 N_("Scale selection horizontally to match the width of the copied object"), NULL),
    new EditVerb(SP_VERB_EDIT_PASTE_SIZE_Y, "EditPasteHeight", N_("Paste _Height"),
                 N_("Scale selection vertically to match the height of the copied object"), NULL),
    new EditVerb(SP_VERB_EDIT_PASTE_SIZE_SEPARATELY, "EditPasteSizeSeparately", N_("Paste Size Separately"),
                 N_("Scale each selected object to match the size of the copied object"), NULL),
    new EditVerb(SP_VERB_EDIT_PASTE_SIZE_SEPARATELY_X, "EditPasteWidthSeparately", N_("Paste Width Separately"),
                 N_("Scale each selected object horizontally to match the width of the copied object"), NULL),
    new EditVerb(SP_VERB_EDIT_PASTE_SIZE_SEPARATELY_Y, "EditPasteHeightSeparately", N_("Paste Height Separately"),
                 N_("Scale each selected object vertically to match the height of the copied object"), NULL),
    new EditVerb(SP_VERB_EDIT_PASTE_IN_PLACE, "EditPasteInPlace", N_("Paste _In Place"),
                 N_("Paste objects from clipboard to the original location"), INKSCAPE_ICON_EDIT_PASTE_IN_PLACE),
    new EditVerb(SP_VERB_EDIT_PASTE_LIVEPATHEFFECT, "PasteLivePathEffect", N_("Paste Path _Effect"),
                 N_("Apply the path effect of the copied object to selection"), NULL),
    new EditVerb(SP_VERB_EDIT_REMOVE_LIVEPATHEFFECT, "RemoveLivePathEffect", N_("Remove Path _Effect"),
                 N_("Remove any path effects from selected objects"), NULL),
    new EditVerb(SP_VERB_EDIT_REMOVE_FILTER, "RemoveFilter", N_("Remove Filters"),
                 N_("Remove any filters from selected objects"), NULL),
    new EditVerb(SP_VERB_EDIT_DELETE, "EditDelete", N_("_Delete"),
                 N_("Delete selection"), GTK_STOCK_DELETE),
    new EditVerb(SP_VERB_EDIT_DUPLICATE, "EditDuplicate", N_("Duplic_ate"),
                 N_("Duplicate selected objects"), INKSCAPE_ICON_EDIT_DUPLICATE),
    new EditVerb(SP_VERB_EDIT_CLONE, "EditClone", N_("Create Clo_ne"),
                 N_("Create a clone (a copy linked to the original) of selected object"), INKSCAPE_ICON_EDIT_CLONE),
    new EditVerb(SP_VERB_EDIT_UNLINK_CLONE, "EditUnlinkClone", N_("Unlin_k Clone"),
                 N_("Cut the selected clones' links to the originals, turning them into standalone objects"), INKSCAPE_ICON_EDIT_CLONE_UNLINK),
    new EditVerb(SP_VERB_EDIT_RELINK_CLONE, "EditRelinkClone", N_("Relink to Copied"),
                 N_("Relink the selected clones to the object currently on the clipboard"), NULL),
    new EditVerb(SP_VERB_EDIT_CLONE_SELECT_ORIGINAL, "EditCloneSelectOriginal", N_("Select _Original"),
                 N_("Select the object to which the selected clone is linked"), INKSCAPE_ICON_EDIT_SELECT_ORIGINAL),
    new EditVerb(SP_VERB_EDIT_SELECTION_2_MARKER, "ObjectsToMarker", N_("Objects to _Marker"),
                 N_("Convert selection to a line marker"), NULL),
    new EditVerb(SP_VERB_EDIT_SELECTION_2_GUIDES, "ObjectsToGuides", N_("Objects to Gu_ides"),
                 N_("Convert selected objects to a collection of guidelines aligned with their edges"), NULL),
    new EditVerb(SP_VERB_EDIT_TILE, "ObjectsToPattern", N_("Objects to Patter_n"),
                 N_("Convert selection to a rectangle with tiled pattern fill"), NULL),
    new EditVerb(SP_VERB_EDIT_UNTILE, "ObjectsFromPattern", N_("Pattern to _Objects"),
                 N_("Extract objects from a tiled pattern fill"), NULL),
    new EditVerb(SP_VERB_EDIT_CLEAR_ALL, "EditClearAll", N_("Clea_r All"),
                 N_("Delete all objects from document"), NULL),
    new EditVerb(SP_VERB_EDIT_SELECT_ALL, "EditSelectAll", N_("Select Al_l"),
                 N_("Select all objects or all nodes"), GTK_STOCK_SELECT_ALL),
    new EditVerb(SP_VERB_EDIT_SELECT_ALL_IN_ALL_LAYERS, "EditSelectAllInAllLayers", N_("Select All in All La_yers"),
                 N_("Select all objects in all visible and unlocked layers"), INKSCAPE_ICON_EDIT_SELECT_ALL_LAYERS),
    new EditVerb(SP_VERB_EDIT_INVERT, "EditInvert", N_("In_vert Selection"),
                 N_("Invert selection (unselect what is selected and select everything else)"), INKSCAPE_ICON_EDIT_SELECT_INVERT),
    new EditVerb(SP_VERB_EDIT_INVERT_IN_ALL_LAYERS, "EditInvertInAllLayers", N_("Invert in All Layers"),
                 N_("Invert selection in all visible and unlocked layers"), NULL),
    new EditVerb(SP_VERB_EDIT_SELECT_NEXT, "EditSelectNext", N_("Select Next"),
                 N_("Select next object or node"), NULL),
    new EditVerb(SP_VERB_EDIT_SELECT_PREV, "EditSelectPrev", N_("Select Previous"),
                 N_("Select previous object or node"), NULL),
    new EditVerb(SP_VERB_EDIT_DESELECT, "EditDeselect", N_("D_eselect"),
                 N_("Deselect any selected objects or nodes"), INKSCAPE_ICON_EDIT_SELECT_NONE),
    new EditVerb(SP_VERB_EDIT_GUIDES_AROUND_PAGE, "EditGuidesAroundPage", N_("_Guides Around Page"),
                 N_("Create four guides aligned with the page borders"), NULL),
    new EditVerb(SP_VERB_EDIT_NEXT_PATHEFFECT_PARAMETER, "EditNextPathEffectParameter", N_("Next path effect parameter"),
                 N_("Show next editable path effect parameter"), INKSCAPE_ICON_PATH_EFFECT_PARAMETER_NEXT),

    /* Selection */
    new SelectionVerb(SP_VERB_SELECTION_TO_FRONT, "SelectionToFront", N_("Raise to _Top"),
                      N_("Raise selection to top"), INKSCAPE_ICON_SELECTION_TOP),
    new SelectionVerb(SP_VERB_SELECTION_TO_BACK, "SelectionToBack", N_("Lower to _Bottom"),
                      N_("Lower selection to bottom"), INKSCAPE_ICON_SELECTION_BOTTOM),
    new SelectionVerb(SP_VERB_SELECTION_RAISE, "SelectionRaise", N_("_Raise"),
                      N_("Raise selection one step"), INKSCAPE_ICON_SELECTION_RAISE),
    new SelectionVerb(SP_VERB_SELECTION_LOWER, "SelectionLower", N_("_Lower"),
                      N_("Lower selection one step"), INKSCAPE_ICON_SELECTION_LOWER),
    new SelectionVerb(SP_VERB_SELECTION_GROUP, "SelectionGroup", N_("_Group"),
                      N_("Group selected objects"), INKSCAPE_ICON_OBJECT_GROUP),
    new SelectionVerb(SP_VERB_SELECTION_UNGROUP, "SelectionUnGroup", N_("_Ungroup"),
                      N_("Ungroup selected groups"), INKSCAPE_ICON_OBJECT_UNGROUP),

    new SelectionVerb(SP_VERB_SELECTION_TEXTTOPATH, "SelectionTextToPath", N_("_Put on Path"),
                      N_("Put text on path"), INKSCAPE_ICON_TEXT_PUT_ON_PATH),
    new SelectionVerb(SP_VERB_SELECTION_TEXTFROMPATH, "SelectionTextFromPath", N_("_Remove from Path"),
                      N_("Remove text from path"), INKSCAPE_ICON_TEXT_REMOVE_FROM_PATH),
    new SelectionVerb(SP_VERB_SELECTION_REMOVE_KERNS, "SelectionTextRemoveKerns", N_("Remove Manual _Kerns"),
                      // TRANSLATORS: "glyph": An image used in the visual representation of characters;
                      //  roughly speaking, how a character looks. A font is a set of glyphs.
                      N_("Remove all manual kerns and glyph rotations from a text object"), INKSCAPE_ICON_TEXT_UNKERN),

    new SelectionVerb(SP_VERB_SELECTION_UNION, "SelectionUnion", N_("_Union"),
                      N_("Create union of selected paths"), INKSCAPE_ICON_PATH_UNION),
    new SelectionVerb(SP_VERB_SELECTION_INTERSECT, "SelectionIntersect", N_("_Intersection"),
                      N_("Create intersection of selected paths"), INKSCAPE_ICON_PATH_INTERSECTION),
    new SelectionVerb(SP_VERB_SELECTION_DIFF, "SelectionDiff", N_("_Difference"),
                      N_("Create difference of selected paths (bottom minus top)"), INKSCAPE_ICON_PATH_DIFFERENCE),
    new SelectionVerb(SP_VERB_SELECTION_SYMDIFF, "SelectionSymDiff", N_("E_xclusion"),
                      N_("Create exclusive OR of selected paths (those parts that belong to only one path)"), INKSCAPE_ICON_PATH_EXCLUSION),
    new SelectionVerb(SP_VERB_SELECTION_CUT, "SelectionDivide", N_("Di_vision"),
                      N_("Cut the bottom path into pieces"), INKSCAPE_ICON_PATH_DIVISION),
    // TRANSLATORS: "to cut a path" is not the same as "to break a path apart" - see the
    // Advanced tutorial for more info
    new SelectionVerb(SP_VERB_SELECTION_SLICE, "SelectionCutPath", N_("Cut _Path"),
                      N_("Cut the bottom path's stroke into pieces, removing fill"), INKSCAPE_ICON_PATH_CUT),
    // TRANSLATORS: "outset": expand a shape by offsetting the object's path,
    // i.e. by displacing it perpendicular to the path in each point.
    // See also the Advanced Tutorial for explanation.
    new SelectionVerb(SP_VERB_SELECTION_OFFSET, "SelectionOffset", N_("Outs_et"),
                      N_("Outset selected paths"), INKSCAPE_ICON_PATH_OUTSET),
    new SelectionVerb(SP_VERB_SELECTION_OFFSET_SCREEN, "SelectionOffsetScreen",
                      N_("O_utset Path by 1 px"),
                      N_("Outset selected paths by 1 px"), NULL),
    new SelectionVerb(SP_VERB_SELECTION_OFFSET_SCREEN_10, "SelectionOffsetScreen10",
                      N_("O_utset Path by 10 px"),
                      N_("Outset selected paths by 10 px"), NULL),
    // TRANSLATORS: "inset": contract a shape by offsetting the object's path,
    // i.e. by displacing it perpendicular to the path in each point.
    // See also the Advanced Tutorial for explanation.
    new SelectionVerb(SP_VERB_SELECTION_INSET, "SelectionInset", N_("I_nset"),
                      N_("Inset selected paths"), INKSCAPE_ICON_PATH_INSET),
    new SelectionVerb(SP_VERB_SELECTION_INSET_SCREEN, "SelectionInsetScreen",
                      N_("I_nset Path by 1 px"),
                      N_("Inset selected paths by 1 px"), NULL),
    new SelectionVerb(SP_VERB_SELECTION_INSET_SCREEN_10, "SelectionInsetScreen10",
                      N_("I_nset Path by 10 px"),
                      N_("Inset selected paths by 10 px"), NULL),
    new SelectionVerb(SP_VERB_SELECTION_DYNAMIC_OFFSET, "SelectionDynOffset",
                      N_("D_ynamic Offset"), N_("Create a dynamic offset object"), INKSCAPE_ICON_PATH_OFFSET_DYNAMIC),
    new SelectionVerb(SP_VERB_SELECTION_LINKED_OFFSET, "SelectionLinkedOffset",
                      N_("_Linked Offset"),
                      N_("Create a dynamic offset object linked to the original path"),
                      INKSCAPE_ICON_PATH_OFFSET_LINKED),
    new SelectionVerb(SP_VERB_SELECTION_OUTLINE, "StrokeToPath", N_("_Stroke to Path"),
                      N_("Convert selected object's stroke to paths"), INKSCAPE_ICON_STROKE_TO_PATH),
    new SelectionVerb(SP_VERB_SELECTION_SIMPLIFY, "SelectionSimplify", N_("Si_mplify"),
                      N_("Simplify selected paths (remove extra nodes)"), INKSCAPE_ICON_PATH_SIMPLIFY),
    new SelectionVerb(SP_VERB_SELECTION_REVERSE, "SelectionReverse", N_("_Reverse"),
                      N_("Reverse the direction of selected paths (useful for flipping markers)"), INKSCAPE_ICON_PATH_REVERSE),
    // TRANSLATORS: "to trace" means "to convert a bitmap to vector graphics" (to vectorize)
    new SelectionVerb(SP_VERB_SELECTION_TRACE, "SelectionTrace", N_("_Trace Bitmap..."),
                      N_("Create one or more paths from a bitmap by tracing it"), INKSCAPE_ICON_BITMAP_TRACE),
    new SelectionVerb(SP_VERB_SELECTION_CREATE_BITMAP, "SelectionCreateBitmap", N_("_Make a Bitmap Copy"),
                      N_("Export selection to a bitmap and insert it into document"), INKSCAPE_ICON_SELECTION_MAKE_BITMAP_COPY ),
    new SelectionVerb(SP_VERB_SELECTION_COMBINE, "SelectionCombine", N_("_Combine"),
                      N_("Combine several paths into one"), INKSCAPE_ICON_PATH_COMBINE),
    // TRANSLATORS: "to cut a path" is not the same as "to break a path apart" - see the
    // Advanced tutorial for more info
    new SelectionVerb(SP_VERB_SELECTION_BREAK_APART, "SelectionBreakApart", N_("Break _Apart"),
                      N_("Break selected paths into subpaths"), INKSCAPE_ICON_PATH_BREAK_APART),
    new SelectionVerb(SP_VERB_SELECTION_GRIDTILE, "DialogGridArrange", N_("Rows and Columns..."),
                      N_("Arrange selected objects in a table"), INKSCAPE_ICON_DIALOG_ROWS_AND_COLUMNS),
    /* Layer */
    new LayerVerb(SP_VERB_LAYER_NEW, "LayerNew", N_("_Add Layer..."),
                  N_("Create a new layer"), INKSCAPE_ICON_LAYER_NEW),
    new LayerVerb(SP_VERB_LAYER_RENAME, "LayerRename", N_("Re_name Layer..."),
                  N_("Rename the current layer"), INKSCAPE_ICON_LAYER_RENAME),
    new LayerVerb(SP_VERB_LAYER_NEXT, "LayerNext", N_("Switch to Layer Abov_e"),
                  N_("Switch to the layer above the current"), INKSCAPE_ICON_LAYER_PREVIOUS),
    new LayerVerb(SP_VERB_LAYER_PREV, "LayerPrev", N_("Switch to Layer Belo_w"),
                  N_("Switch to the layer below the current"), INKSCAPE_ICON_LAYER_NEXT),
    new LayerVerb(SP_VERB_LAYER_MOVE_TO_NEXT, "LayerMoveToNext", N_("Move Selection to Layer Abo_ve"),
                  N_("Move selection to the layer above the current"), INKSCAPE_ICON_SELECTION_MOVE_TO_LAYER_ABOVE),
    new LayerVerb(SP_VERB_LAYER_MOVE_TO_PREV, "LayerMoveToPrev", N_("Move Selection to Layer Bel_ow"),
                  N_("Move selection to the layer below the current"), INKSCAPE_ICON_SELECTION_MOVE_TO_LAYER_BELOW),
    new LayerVerb(SP_VERB_LAYER_TO_TOP, "LayerToTop", N_("Layer to _Top"),
                  N_("Raise the current layer to the top"), INKSCAPE_ICON_LAYER_TOP),
    new LayerVerb(SP_VERB_LAYER_TO_BOTTOM, "LayerToBottom", N_("Layer to _Bottom"),
                  N_("Lower the current layer to the bottom"), INKSCAPE_ICON_LAYER_BOTTOM),
    new LayerVerb(SP_VERB_LAYER_RAISE, "LayerRaise", N_("_Raise Layer"),
                  N_("Raise the current layer"), INKSCAPE_ICON_LAYER_RAISE),
    new LayerVerb(SP_VERB_LAYER_LOWER, "LayerLower", N_("_Lower Layer"),
                  N_("Lower the current layer"), INKSCAPE_ICON_LAYER_LOWER),
    new LayerVerb(SP_VERB_LAYER_DUPLICATE, "LayerDuplicate", N_("Duplicate Current Layer"),
                  N_("Duplicate an existing layer"), NULL),
    new LayerVerb(SP_VERB_LAYER_DELETE, "LayerDelete", N_("_Delete Current Layer"),
                  N_("Delete the current layer"), INKSCAPE_ICON_LAYER_DELETE),
    new LayerVerb(SP_VERB_LAYER_SOLO, "LayerSolo", N_("_Show/hide other layers"),
                  N_("Solo the current layer"), NULL),

    /* Object */
    new ObjectVerb(SP_VERB_OBJECT_ROTATE_90_CW, "ObjectRotate90", N_("Rotate _90&#176; CW"),
                   // This is shared between tooltips and statusbar, so they
                   // must use UTF-8, not HTML entities for special characters.
                   N_("Rotate selection 90\xc2\xb0 clockwise"), INKSCAPE_ICON_OBJECT_ROTATE_RIGHT),
    new ObjectVerb(SP_VERB_OBJECT_ROTATE_90_CCW, "ObjectRotate90CCW", N_("Rotate 9_0&#176; CCW"),
                   // This is shared between tooltips and statusbar, so they
                   // must use UTF-8, not HTML entities for special characters.
                   N_("Rotate selection 90\xc2\xb0 counter-clockwise"), INKSCAPE_ICON_OBJECT_ROTATE_LEFT),
    new ObjectVerb(SP_VERB_OBJECT_FLATTEN, "ObjectRemoveTransform", N_("Remove _Transformations"),
                   N_("Remove transformations from object"), NULL),
    new ObjectVerb(SP_VERB_OBJECT_TO_CURVE, "ObjectToPath", N_("_Object to Path"),
                   N_("Convert selected object to path"), INKSCAPE_ICON_OBJECT_TO_PATH),
    new ObjectVerb(SP_VERB_OBJECT_FLOW_TEXT, "ObjectFlowText", N_("_Flow into Frame"),
                   N_("Put text into a frame (path or shape), creating a flowed text linked to the frame object"), "text-flow-into-frame"),
    new ObjectVerb(SP_VERB_OBJECT_UNFLOW_TEXT, "ObjectUnFlowText", N_("_Unflow"),
                   N_("Remove text from frame (creates a single-line text object)"), INKSCAPE_ICON_TEXT_UNFLOW),
    new ObjectVerb(SP_VERB_OBJECT_FLOWTEXT_TO_TEXT, "ObjectFlowtextToText", N_("_Convert to Text"),
                   N_("Convert flowed text to regular text object (preserves appearance)"), INKSCAPE_ICON_TEXT_CONVERT_TO_REGULAR),
    new ObjectVerb(SP_VERB_OBJECT_FLIP_HORIZONTAL, "ObjectFlipHorizontally",
                   N_("Flip _Horizontal"), N_("Flip selected objects horizontally"),
                   INKSCAPE_ICON_OBJECT_FLIP_HORIZONTAL),
    new ObjectVerb(SP_VERB_OBJECT_FLIP_VERTICAL, "ObjectFlipVertically",
                   N_("Flip _Vertical"), N_("Flip selected objects vertically"),
                   INKSCAPE_ICON_OBJECT_FLIP_VERTICAL),
    new ObjectVerb(SP_VERB_OBJECT_SET_MASK, "ObjectSetMask", N_("_Set"),
                 N_("Apply mask to selection (using the topmost object as mask)"), NULL),
    new ObjectVerb(SP_VERB_OBJECT_EDIT_MASK, "ObjectEditMask", N_("_Edit"),
                 N_("Edit mask"), INKSCAPE_ICON_PATH_MASK_EDIT),
    new ObjectVerb(SP_VERB_OBJECT_UNSET_MASK, "ObjectUnSetMask", N_("_Release"),
                 N_("Remove mask from selection"), NULL),
    new ObjectVerb(SP_VERB_OBJECT_SET_CLIPPATH, "ObjectSetClipPath", N_("_Set"),
                 N_("Apply clipping path to selection (using the topmost object as clipping path)"), NULL),
    new ObjectVerb(SP_VERB_OBJECT_EDIT_CLIPPATH, "ObjectEditClipPath", N_("_Edit"),
                 N_("Edit clipping path"), INKSCAPE_ICON_PATH_CLIP_EDIT),
    new ObjectVerb(SP_VERB_OBJECT_UNSET_CLIPPATH, "ObjectUnSetClipPath", N_("_Release"),
                 N_("Remove clipping path from selection"), NULL),

    /* Tools */
    new ContextVerb(SP_VERB_CONTEXT_SELECT, "ToolSelector", N_("Select"),
                    N_("Select and transform objects"), INKSCAPE_ICON_TOOL_POINTER),
    new ContextVerb(SP_VERB_CONTEXT_NODE, "ToolNode", N_("Node Edit"),
                    N_("Edit paths by nodes"), INKSCAPE_ICON_TOOL_NODE_EDITOR),
    new ContextVerb(SP_VERB_CONTEXT_TWEAK, "ToolTweak", N_("Tweak"),
                    N_("Tweak objects by sculpting or painting"), INKSCAPE_ICON_TOOL_TWEAK),
    new ContextVerb(SP_VERB_CONTEXT_SPRAY, "ToolSpray", N_("Spray"),
                    N_("Spray objects by sculpting or painting"), INKSCAPE_ICON_TOOL_SPRAY), 
    new ContextVerb(SP_VERB_CONTEXT_RECT, "ToolRect", N_("Rectangle"),
                    N_("Create rectangles and squares"), INKSCAPE_ICON_DRAW_RECTANGLE),
    new ContextVerb(SP_VERB_CONTEXT_3DBOX, "Tool3DBox", N_("3D Box"),
                    N_("Create 3D boxes"), INKSCAPE_ICON_DRAW_CUBOID),
    new ContextVerb(SP_VERB_CONTEXT_ARC, "ToolArc", N_("Ellipse"),
                    N_("Create circles, ellipses, and arcs"), INKSCAPE_ICON_DRAW_ELLIPSE),
    new ContextVerb(SP_VERB_CONTEXT_STAR, "ToolStar", N_("Star"),
                    N_("Create stars and polygons"), INKSCAPE_ICON_DRAW_POLYGON_STAR),
    new ContextVerb(SP_VERB_CONTEXT_SPIRAL, "ToolSpiral", N_("Spiral"),
                    N_("Create spirals"), INKSCAPE_ICON_DRAW_SPIRAL),
    new ContextVerb(SP_VERB_CONTEXT_PENCIL, "ToolPencil", N_("Pencil"),
                    N_("Draw freehand lines"), INKSCAPE_ICON_DRAW_FREEHAND),
    new ContextVerb(SP_VERB_CONTEXT_PEN, "ToolPen", N_("Pen"),
                    N_("Draw Bezier curves and straight lines"), INKSCAPE_ICON_DRAW_PATH),
    new ContextVerb(SP_VERB_CONTEXT_CALLIGRAPHIC, "ToolCalligraphic", N_("Calligraphy"),
                    N_("Draw calligraphic or brush strokes"), INKSCAPE_ICON_DRAW_CALLIGRAPHIC),
    new ContextVerb(SP_VERB_CONTEXT_TEXT, "ToolText", N_("Text"),
                    N_("Create and edit text objects"), INKSCAPE_ICON_DRAW_TEXT),
    new ContextVerb(SP_VERB_CONTEXT_GRADIENT, "ToolGradient", N_("Gradient"),
                    N_("Create and edit gradients"), INKSCAPE_ICON_COLOR_GRADIENT),
    new ContextVerb(SP_VERB_CONTEXT_ZOOM, "ToolZoom", N_("Zoom"),
                    N_("Zoom in or out"), INKSCAPE_ICON_ZOOM),
    new ContextVerb(SP_VERB_CONTEXT_DROPPER, "ToolDropper", N_("Dropper"),
                    N_("Pick colors from image"), INKSCAPE_ICON_COLOR_PICKER),
    new ContextVerb(SP_VERB_CONTEXT_CONNECTOR, "ToolConnector", N_("Connector"),
                    N_("Create diagram connectors"), INKSCAPE_ICON_DRAW_CONNECTOR),
    new ContextVerb(SP_VERB_CONTEXT_PAINTBUCKET, "ToolPaintBucket", N_("Paint Bucket"),
                    N_("Fill bounded areas"), INKSCAPE_ICON_COLOR_FILL),
    new ContextVerb(SP_VERB_CONTEXT_LPE, "ToolLPE", N_("LPE Edit"),
                    N_("Edit Path Effect parameters"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_ERASER, "ToolEraser", N_("Eraser"),
                    N_("Erase existing paths"), INKSCAPE_ICON_DRAW_ERASER),
    new ContextVerb(SP_VERB_CONTEXT_LPETOOL, "ToolLPETool", N_("LPE Tool"),
                    N_("Do geometric constructions"), "draw-geometry"),
    /* Tool prefs */
    new ContextVerb(SP_VERB_CONTEXT_SELECT_PREFS, "SelectPrefs", N_("Selector Preferences"),
                    N_("Open Preferences for the Selector tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_NODE_PREFS, "NodePrefs", N_("Node Tool Preferences"),
                    N_("Open Preferences for the Node tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_TWEAK_PREFS, "TweakPrefs", N_("Tweak Tool Preferences"),
                    N_("Open Preferences for the Tweak tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_SPRAY_PREFS, "SprayPrefs", N_("Spray Tool Preferences"),
                    N_("Open Preferences for the Spray tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_RECT_PREFS, "RectPrefs", N_("Rectangle Preferences"),
                    N_("Open Preferences for the Rectangle tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_3DBOX_PREFS, "3DBoxPrefs", N_("3D Box Preferences"),
                    N_("Open Preferences for the 3D Box tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_ARC_PREFS, "ArcPrefs", N_("Ellipse Preferences"),
                    N_("Open Preferences for the Ellipse tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_STAR_PREFS, "StarPrefs", N_("Star Preferences"),
                    N_("Open Preferences for the Star tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_SPIRAL_PREFS, "SpiralPrefs", N_("Spiral Preferences"),
                    N_("Open Preferences for the Spiral tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_PENCIL_PREFS, "PencilPrefs", N_("Pencil Preferences"),
                    N_("Open Preferences for the Pencil tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_PEN_PREFS, "PenPrefs", N_("Pen Preferences"),
                    N_("Open Preferences for the Pen tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_CALLIGRAPHIC_PREFS, "CalligraphicPrefs", N_("Calligraphic Preferences"),
                    N_("Open Preferences for the Calligraphy tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_TEXT_PREFS, "TextPrefs", N_("Text Preferences"),
                    N_("Open Preferences for the Text tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_GRADIENT_PREFS, "GradientPrefs", N_("Gradient Preferences"),
                    N_("Open Preferences for the Gradient tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_ZOOM_PREFS, "ZoomPrefs", N_("Zoom Preferences"),
                    N_("Open Preferences for the Zoom tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_DROPPER_PREFS, "DropperPrefs", N_("Dropper Preferences"),
                    N_("Open Preferences for the Dropper tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_CONNECTOR_PREFS, "ConnectorPrefs", N_("Connector Preferences"),
                    N_("Open Preferences for the Connector tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_PAINTBUCKET_PREFS, "PaintBucketPrefs", N_("Paint Bucket Preferences"),
                    N_("Open Preferences for the Paint Bucket tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_ERASER_PREFS, "EraserPrefs", N_("Eraser Preferences"),
                    N_("Open Preferences for the Eraser tool"), NULL),
    new ContextVerb(SP_VERB_CONTEXT_LPETOOL_PREFS, "LPEToolPrefs", N_("LPE Tool Preferences"),
                    N_("Open Preferences for the LPETool tool"), NULL),

    /* Zoom/View */
    new ZoomVerb(SP_VERB_ZOOM_IN, "ZoomIn", N_("Zoom In"), N_("Zoom in"), INKSCAPE_ICON_ZOOM_IN),
    new ZoomVerb(SP_VERB_ZOOM_OUT, "ZoomOut", N_("Zoom Out"), N_("Zoom out"), INKSCAPE_ICON_ZOOM_OUT),
    new ZoomVerb(SP_VERB_TOGGLE_RULERS, "ToggleRulers", N_("_Rulers"), N_("Show or hide the canvas rulers"), NULL),
    new ZoomVerb(SP_VERB_TOGGLE_SCROLLBARS, "ToggleScrollbars", N_("Scroll_bars"), N_("Show or hide the canvas scrollbars"), NULL),
    new ZoomVerb(SP_VERB_TOGGLE_GRID, "ToggleGrid", N_("_Grid"), N_("Show or hide the grid"), INKSCAPE_ICON_SHOW_GRID),
    new ZoomVerb(SP_VERB_TOGGLE_GUIDES, "ToggleGuides", N_("G_uides"), N_("Show or hide guides (drag from a ruler to create a guide)"), INKSCAPE_ICON_SHOW_GUIDES),
    new ZoomVerb(SP_VERB_TOGGLE_SNAPPING, "ToggleSnapGlobal", N_("Snap"), N_("Enable snapping"), INKSCAPE_ICON_SNAP),
    new ZoomVerb(SP_VERB_ZOOM_NEXT, "ZoomNext", N_("Nex_t Zoom"), N_("Next zoom (from the history of zooms)"),
                 INKSCAPE_ICON_ZOOM_NEXT),
    new ZoomVerb(SP_VERB_ZOOM_PREV, "ZoomPrev", N_("Pre_vious Zoom"), N_("Previous zoom (from the history of zooms)"),
                 INKSCAPE_ICON_ZOOM_PREVIOUS),
    new ZoomVerb(SP_VERB_ZOOM_1_1, "Zoom1:0", N_("Zoom 1:_1"), N_("Zoom to 1:1"),
                 INKSCAPE_ICON_ZOOM_ORIGINAL),
    new ZoomVerb(SP_VERB_ZOOM_1_2, "Zoom1:2", N_("Zoom 1:_2"), N_("Zoom to 1:2"),
                 INKSCAPE_ICON_ZOOM_HALF_SIZE),
    new ZoomVerb(SP_VERB_ZOOM_2_1, "Zoom2:1", N_("_Zoom 2:1"), N_("Zoom to 2:1"),
                 INKSCAPE_ICON_ZOOM_DOUBLE_SIZE),
#ifdef HAVE_GTK_WINDOW_FULLSCREEN
    new ZoomVerb(SP_VERB_FULLSCREEN, "FullScreen", N_("_Fullscreen"), N_("Stretch this document window to full screen"),
                 INKSCAPE_ICON_VIEW_FULLSCREEN),
#endif /* HAVE_GTK_WINDOW_FULLSCREEN */
    new ZoomVerb(SP_VERB_FOCUSTOGGLE, "FocusToggle", N_("Toggle _Focus Mode"), N_("Remove excess toolbars to focus on drawing"),
                 NULL),
    new ZoomVerb(SP_VERB_VIEW_NEW, "ViewNew", N_("Duplic_ate Window"), N_("Open a new window with the same document"),
                 INKSCAPE_ICON_WINDOW_NEW),
    new ZoomVerb(SP_VERB_VIEW_NEW_PREVIEW, "ViewNewPreview", N_("_New View Preview"),
                 N_("New View Preview"), NULL/*"view_new_preview"*/),

    new ZoomVerb(SP_VERB_VIEW_MODE_NORMAL, "ViewModeNormal", N_("_Normal"),
                 N_("Switch to normal display mode"), NULL),
    new ZoomVerb(SP_VERB_VIEW_MODE_NO_FILTERS, "ViewModeNoFilters", N_("No _Filters"),
                 N_("Switch to normal display without filters"), NULL),
    new ZoomVerb(SP_VERB_VIEW_MODE_OUTLINE, "ViewModeOutline", N_("_Outline"),
                 N_("Switch to outline (wireframe) display mode"), NULL),
//    new ZoomVerb(SP_VERB_VIEW_MODE_PRINT_COLORS_PREVIEW, "ViewModePrintColorsPreview", N_("_Print Colors Preview"),
//                 N_("Switch to print colors preview mode"), NULL),
    new ZoomVerb(SP_VERB_VIEW_MODE_TOGGLE, "ViewModeToggle", N_("_Toggle"),
                 N_("Toggle between normal and outline display modes"), NULL),

    new ZoomVerb(SP_VERB_VIEW_CMS_TOGGLE, "ViewCmsToggle", N_("Color-managed view"),
                 N_("Toggle color-managed display for this document window"), INKSCAPE_ICON_COLOR_MANAGEMENT),

    new ZoomVerb(SP_VERB_VIEW_ICON_PREVIEW, "ViewIconPreview", N_("Ico_n Preview..."),
                 N_("Open a window to preview objects at different icon resolutions"), INKSCAPE_ICON_DIALOG_ICON_PREVIEW),
    new ZoomVerb(SP_VERB_ZOOM_PAGE, "ZoomPage", N_("_Page"),
                 N_("Zoom to fit page in window"), INKSCAPE_ICON_ZOOM_FIT_PAGE),
    new ZoomVerb(SP_VERB_ZOOM_PAGE_WIDTH, "ZoomPageWidth", N_("Page _Width"),
                 N_("Zoom to fit page width in window"), INKSCAPE_ICON_ZOOM_FIT_WIDTH),
    new ZoomVerb(SP_VERB_ZOOM_DRAWING, "ZoomDrawing", N_("_Drawing"),
                 N_("Zoom to fit drawing in window"), INKSCAPE_ICON_ZOOM_FIT_DRAWING),
    new ZoomVerb(SP_VERB_ZOOM_SELECTION, "ZoomSelection", N_("_Selection"),
                 N_("Zoom to fit selection in window"), INKSCAPE_ICON_ZOOM_FIT_SELECTION),

    /* Dialogs */
    new DialogVerb(SP_VERB_DIALOG_DISPLAY, "DialogPreferences", N_("In_kscape Preferences..."),
                   N_("Edit global Inkscape preferences"), GTK_STOCK_PREFERENCES ),
    new DialogVerb(SP_VERB_DIALOG_NAMEDVIEW, "DialogDocumentProperties", N_("_Document Properties..."),
                   N_("Edit properties of this document (to be saved with the document)"), GTK_STOCK_PROPERTIES ),
    new DialogVerb(SP_VERB_DIALOG_METADATA, "DialogMetadata", N_("Document _Metadata..."),
                   N_("Edit document metadata (to be saved with the document)"), INKSCAPE_ICON_DOCUMENT_METADATA ),
    new DialogVerb(SP_VERB_DIALOG_FILL_STROKE, "DialogFillStroke", N_("_Fill and Stroke..."),
                   N_("Edit objects' colors, gradients, stroke width, arrowheads, dash patterns..."), INKSCAPE_ICON_DIALOG_FILL_AND_STROKE),
    new DialogVerb(SP_VERB_DIALOG_GLYPHS, "DialogGlyphs", N_("Glyphs..."),
                   N_("Select characters from a glyphs palette"), GTK_STOCK_SELECT_FONT),
    // TRANSLATORS: "Swatches" means: color samples
    new DialogVerb(SP_VERB_DIALOG_SWATCHES, "DialogSwatches", N_("S_watches..."),
                   N_("Select colors from a swatches palette"), GTK_STOCK_SELECT_COLOR),
    new DialogVerb(SP_VERB_DIALOG_TRANSFORM, "DialogTransform", N_("Transfor_m..."),
                   N_("Precisely control objects' transformations"), INKSCAPE_ICON_DIALOG_TRANSFORM),
    new DialogVerb(SP_VERB_DIALOG_ALIGN_DISTRIBUTE, "DialogAlignDistribute", N_("_Align and Distribute..."),
                   N_("Align and distribute objects"), INKSCAPE_ICON_DIALOG_ALIGN_AND_DISTRIBUTE),
    new DialogVerb(SP_VERB_DIALOG_SPRAY_OPTION, "DialogSprayOption", N_("_Spray options..."),
                   N_("Some options for the spray"), INKSCAPE_ICON_DIALOG_SPRAY_OPTIONS),
    new DialogVerb(SP_VERB_DIALOG_UNDO_HISTORY, "DialogUndoHistory", N_("Undo _History..."),
                   N_("Undo History"), INKSCAPE_ICON_EDIT_UNDO_HISTORY),
    new DialogVerb(SP_VERB_DIALOG_TEXT, "DialogText", N_("_Text and Font..."),
                   N_("View and select font family, font size and other text properties"), INKSCAPE_ICON_DIALOG_TEXT_AND_FONT),
    new DialogVerb(SP_VERB_DIALOG_XML_EDITOR, "DialogXMLEditor", N_("_XML Editor..."),
                   N_("View and edit the XML tree of the document"), INKSCAPE_ICON_DIALOG_XML_EDITOR),
    new DialogVerb(SP_VERB_DIALOG_FIND, "DialogFind", N_("_Find..."),
                   N_("Find objects in document"), GTK_STOCK_FIND ),
    new DialogVerb(SP_VERB_DIALOG_FINDREPLACE, "DialogFindReplace", N_("Find and _Replace Text..."),
                   N_("Find and replace text in document"), GTK_STOCK_FIND_AND_REPLACE ),
    new DialogVerb(SP_VERB_DIALOG_SPELLCHECK, "DialogSpellcheck", N_("Check Spellin_g..."),
                   N_("Check spelling of text in document"), GTK_STOCK_SPELL_CHECK ),
    new DialogVerb(SP_VERB_DIALOG_DEBUG, "DialogDebug", N_("_Messages..."),
                   N_("View debug messages"), INKSCAPE_ICON_DIALOG_MESSAGES),
    new DialogVerb(SP_VERB_DIALOG_SCRIPT, "DialogScript", N_("S_cripts..."),
                   N_("Run scripts"), INKSCAPE_ICON_DIALOG_SCRIPTS),
    new DialogVerb(SP_VERB_DIALOG_TOGGLE, "DialogsToggle", N_("Show/Hide D_ialogs"),
                   N_("Show or hide all open dialogs"), INKSCAPE_ICON_SHOW_DIALOGS),
    new DialogVerb(SP_VERB_DIALOG_CLONETILER, "DialogClonetiler", N_("Create Tiled Clones..."),
                   N_("Create multiple clones of selected object, arranging them into a pattern or scattering"), INKSCAPE_ICON_DIALOG_TILE_CLONES),
    new DialogVerb(SP_VERB_DIALOG_ITEM, "DialogObjectProperties", N_("_Object Properties..."),
                   N_("Edit the ID, locked and visible status, and other object properties"), INKSCAPE_ICON_DIALOG_OBJECT_PROPERTIES),
/*#ifdef WITH_INKBOARD
    new DialogVerb(SP_VERB_XMPP_CLIENT, "DialogXmppClient",
                   N_("_Instant Messaging..."), N_("Jabber Instant Messaging Client"), NULL),
#endif*/
    new DialogVerb(SP_VERB_DIALOG_INPUT, "DialogInput", N_("_Input Devices..."),
                   N_("Configure extended input devices, such as a graphics tablet"), INKSCAPE_ICON_DIALOG_INPUT_DEVICES),
    new DialogVerb(SP_VERB_DIALOG_EXTENSIONEDITOR, "org.inkscape.dialogs.extensioneditor", N_("_Extensions..."),
                   N_("Query information about extensions"), NULL),
    new DialogVerb(SP_VERB_DIALOG_LAYERS, "DialogLayers", N_("Layer_s..."),
                   N_("View Layers"), INKSCAPE_ICON_DIALOG_LAYERS),
    new DialogVerb(SP_VERB_DIALOG_LIVE_PATH_EFFECT, "DialogLivePathEffect", N_("Path Effect Editor..."),
                   N_("Manage, edit, and apply path effects"), NULL),
    new DialogVerb(SP_VERB_DIALOG_FILTER_EFFECTS, "DialogFilterEffects", N_("Filter Editor..."),
                   N_("Manage, edit, and apply SVG filters"), NULL),
    new DialogVerb(SP_VERB_DIALOG_SVG_FONTS, "DialogSVGFonts", N_("SVG Font Editor..."),
                   N_("Edit SVG fonts"), NULL),
    new DialogVerb(SP_VERB_DIALOG_PRINT_COLORS_PREVIEW, "DialogPrintColorsPreview", N_("Print Colors..."),
                   N_("Select which color separations to render in Print Colors Preview rendermode"), NULL),

    /* Help */
    new HelpVerb(SP_VERB_HELP_ABOUT_EXTENSIONS, "HelpAboutExtensions", N_("About E_xtensions"),
                 N_("Information on Inkscape extensions"), NULL),
    new HelpVerb(SP_VERB_HELP_MEMORY, "HelpAboutMemory", N_("About _Memory"),
                 N_("Memory usage information"), INKSCAPE_ICON_DIALOG_MEMORY),
    new HelpVerb(SP_VERB_HELP_ABOUT, "HelpAbout", N_("_About Inkscape"),
                 N_("Inkscape version, authors, license"), INKSCAPE_ICON_INKSCAPE),
    //new HelpVerb(SP_VERB_SHOW_LICENSE, "ShowLicense", N_("_License"),
    //           N_("Distribution terms"), /*"show_license"*/"inkscape_options"),

    /* Tutorials */
    new TutorialVerb(SP_VERB_TUTORIAL_BASIC, "TutorialsBasic", N_("Inkscape: _Basic"),
                     N_("Getting started with Inkscape"), NULL/*"tutorial_basic"*/),
    new TutorialVerb(SP_VERB_TUTORIAL_SHAPES, "TutorialsShapes", N_("Inkscape: _Shapes"),
                     N_("Using shape tools to create and edit shapes"), NULL),
    new TutorialVerb(SP_VERB_TUTORIAL_ADVANCED, "TutorialsAdvanced", N_("Inkscape: _Advanced"),
                     N_("Advanced Inkscape topics"), NULL/*"tutorial_advanced"*/),
    // TRANSLATORS: "to trace" means "to convert a bitmap to vector graphics" (to vectorize)
    new TutorialVerb(SP_VERB_TUTORIAL_TRACING, "TutorialsTracing", N_("Inkscape: T_racing"),
                     N_("Using bitmap tracing"), NULL/*"tutorial_tracing"*/),
    new TutorialVerb(SP_VERB_TUTORIAL_CALLIGRAPHY, "TutorialsCalligraphy", N_("Inkscape: _Calligraphy"),
                     N_("Using the Calligraphy pen tool"), NULL),
    new TutorialVerb(SP_VERB_TUTORIAL_INTERPOLATE, "TutorialsInterpolate", N_("Inkscape: _Interpolate"),
                     N_("Using the interpolate extension"), NULL/*"tutorial_interpolate"*/),
    new TutorialVerb(SP_VERB_TUTORIAL_DESIGN, "TutorialsDesign", N_("_Elements of Design"),
                     N_("Principles of design in the tutorial form"), NULL/*"tutorial_design"*/),
    new TutorialVerb(SP_VERB_TUTORIAL_TIPS, "TutorialsTips", N_("_Tips and Tricks"),
                     N_("Miscellaneous tips and tricks"), NULL/*"tutorial_tips"*/),

    /* Effect -- renamed Extension */
    new EffectLastVerb(SP_VERB_EFFECT_LAST, "EffectLast", N_("Previous Extension"),
                       N_("Repeat the last extension with the same settings"), NULL),
    new EffectLastVerb(SP_VERB_EFFECT_LAST_PREF, "EffectLastPref", N_("Previous Extension Settings..."),
                       N_("Repeat the last extension with new settings"), NULL),

    /* Fit Page */
    new FitCanvasVerb(SP_VERB_FIT_CANVAS_TO_SELECTION, "FitCanvasToSelection", N_("Fit Page to Selection"),
                       N_("Fit the page to the current selection"), NULL),
    new FitCanvasVerb(SP_VERB_FIT_CANVAS_TO_DRAWING, "FitCanvasToDrawing", N_("Fit Page to Drawing"),
                       N_("Fit the page to the drawing"), NULL),
    new FitCanvasVerb(SP_VERB_FIT_CANVAS_TO_SELECTION_OR_DRAWING, "FitCanvasToSelectionOrDrawing", N_("Fit Page to Selection or Drawing"),
                       N_("Fit the page to the current selection or the drawing if there is no selection"), NULL),
    /* LockAndHide */
    new LockAndHideVerb(SP_VERB_UNLOCK_ALL, "UnlockAll", N_("Unlock All"),
                       N_("Unlock all objects in the current layer"), NULL),
    new LockAndHideVerb(SP_VERB_UNLOCK_ALL_IN_ALL_LAYERS, "UnlockAllInAllLayers", N_("Unlock All in All Layers"),
                       N_("Unlock all objects in all layers"), NULL),
    new LockAndHideVerb(SP_VERB_UNHIDE_ALL, "UnhideAll", N_("Unhide All"),
                       N_("Unhide all objects in the current layer"), NULL),
    new LockAndHideVerb(SP_VERB_UNHIDE_ALL_IN_ALL_LAYERS, "UnhideAllInAllLayers", N_("Unhide All in All Layers"),
                       N_("Unhide all objects in all layers"), NULL),
    /*Color Management*/
    new EditVerb(SP_VERB_EDIT_LINK_COLOR_PROFILE, "LinkColorProfile", N_("Link Color Profile"),
                 N_("Link an ICC color profile"), NULL),
    new EditVerb(SP_VERB_EDIT_REMOVE_COLOR_PROFILE, "RemoveColorProfile", N_("Remove Color Profile"),
                 N_("Remove a linked ICC color profile"), NULL),
    /* Footer */
    new Verb(SP_VERB_LAST, " '\"invalid id", NULL, NULL, NULL)
};


void
Verb::list (void) {
    // Go through the dynamic verb table
    for (VerbTable::iterator iter = _verbs.begin(); iter != _verbs.end(); iter++) {
        Verb * verb = iter->second;
        if (verb->get_code() == SP_VERB_INVALID ||
                verb->get_code() == SP_VERB_NONE ||
                verb->get_code() == SP_VERB_LAST) {
            continue;
        }

        printf("%s: %s\n", verb->get_id(), verb->get_tip()? verb->get_tip() : verb->get_name());
    }

    return;
};

}  /* namespace Inkscape */

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
