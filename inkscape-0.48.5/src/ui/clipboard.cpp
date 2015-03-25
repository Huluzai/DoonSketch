/** @file
 * @brief System-wide clipboard management - implementation
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk@o2.pl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Incorporates some code from selection-chemistry.cpp, see that file for more credits.
 *
 * Copyright (C) 2008 authors
 * Copyright (C) 2010 Jon A. Cruz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 */

#include "ui/clipboard.h"

// TODO: reduce header bloat if possible

#include <list>
#include <algorithm>
#include <gtkmm/clipboard.h>
#include <glibmm/ustring.h>
#include <glibmm/i18n.h>
#include <glib/gstdio.h> // for g_file_set_contents etc., used in _onGet and paste
#include "gc-core.h"
#include "xml/repr.h"
#include "inkscape.h"
#include "io/stringstream.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-style.h" // for sp_desktop_set_style, used in _pasteStyle
#include "document.h"
#include "document-private.h"
#include "selection.h"
#include "message-stack.h"
#include "context-fns.h"
#include "dropper-context.h" // used in copy()
#include "style.h"
#include "extension/db.h" // extension database
#include "extension/input.h"
#include "extension/output.h"
#include "selection-chemistry.h"
#include "libnr/nr-rect.h"
#include "libnr/nr-convert2geom.h"
#include <2geom/rect.h>
#include <2geom/transforms.h>
#include "box3d.h"
#include "gradient-drag.h"
#include "sp-item.h"
#include "sp-item-transform.h" // for sp_item_scale_rel, used in _pasteSize
#include "sp-path.h"
#include "sp-pattern.h"
#include "sp-shape.h"
#include "sp-gradient.h"
#include "sp-gradient-reference.h"
#include "sp-gradient-fns.h"
#include "sp-linear-gradient-fns.h"
#include "sp-radial-gradient-fns.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "sp-textpath.h"
#include "sp-rect.h"
#include "live_effects/lpeobject.h"
#include "live_effects/lpeobject-reference.h"
#include "live_effects/parameter/path.h"
#include "svg/svg.h" // for sp_svg_transform_write, used in _copySelection
#include "svg/css-ostringstream.h" // used in _parseColor
#include "file.h" // for file_import, used in _pasteImage
#include "text-context.h"
#include "text-editing.h"
#include "tools-switch.h"
#include "path-chemistry.h"
#include "id-clash.h"
#include "unit-constants.h"
#include "helper/png-write.h"
#include "svg/svg-color.h"
#include "sp-namedview.h"
#include "snap.h"

/// @brief Made up mimetype to represent Gdk::Pixbuf clipboard contents
#define CLIPBOARD_GDK_PIXBUF_TARGET "image/x-gdk-pixbuf"

#define CLIPBOARD_TEXT_TARGET "text/plain"

#ifdef WIN32
#include <windows.h>
// Clipboard Formats: http://msdn.microsoft.com/en-us/library/ms649013(VS.85).aspx
// On Windows, most graphical applications can handle CF_DIB/CF_BITMAP and/or CF_ENHMETAFILE
// GTK automatically presents an "image/bmp" target as CF_DIB/CF_BITMAP
// Presenting "image/x-emf" as CF_ENHMETAFILE must be done by Inkscape ?
#define CLIPBOARD_WIN32_EMF_TARGET "CF_ENHMETAFILE"
#define CLIPBOARD_WIN32_EMF_MIME   "image/x-emf"
#endif

namespace Inkscape {
namespace UI {


/**
 * @brief Default implementation of the clipboard manager
 */
class ClipboardManagerImpl : public ClipboardManager {
public:
    virtual void copy(SPDesktop *desktop);
    virtual void copyPathParameter(Inkscape::LivePathEffect::PathParam *);
    virtual bool paste(SPDesktop *desktop, bool in_place);
    virtual bool pasteStyle(SPDesktop *desktop);
    virtual bool pasteSize(SPDesktop *desktop, bool separately, bool apply_x, bool apply_y);
    virtual bool pastePathEffect(SPDesktop *desktop);
    virtual Glib::ustring getPathParameter(SPDesktop* desktop);
    virtual Glib::ustring getShapeOrTextObjectId(SPDesktop *desktop);
    virtual const gchar *getFirstObjectID();

    ClipboardManagerImpl();
    ~ClipboardManagerImpl();

private:
    void _copySelection(Inkscape::Selection *);
    void _copyUsedDefs(SPItem *);
    void _copyGradient(SPGradient *);
    void _copyPattern(SPPattern *);
    void _copyTextPath(SPTextPath *);
    Inkscape::XML::Node *_copyNode(Inkscape::XML::Node *, Inkscape::XML::Document *, Inkscape::XML::Node *);

    void _pasteDocument(SPDesktop *desktop, SPDocument *clipdoc, bool in_place);
    void _pasteDefs(SPDesktop *desktop, SPDocument *clipdoc);
    bool _pasteImage(SPDocument *doc);
    bool _pasteText(SPDesktop *desktop);
    SPCSSAttr *_parseColor(const Glib::ustring &);
    void _applyPathEffect(SPItem *, gchar const *);
    SPDocument *_retrieveClipboard(Glib::ustring = "");

    // clipboard callbacks
    void _onGet(Gtk::SelectionData &, guint);
    void _onClear();

    // various helpers
    void _createInternalClipboard();
    void _discardInternalClipboard();
    Inkscape::XML::Node *_createClipNode();
    Geom::Scale _getScale(SPDesktop *desktop, Geom::Point const &min, Geom::Point const &max, Geom::Rect const &obj_rect, bool apply_x, bool apply_y);
    Glib::ustring _getBestTarget();
    void _setClipboardTargets();
    void _setClipboardColor(guint32);
    void _userWarn(SPDesktop *, char const *);

    void _inkscape_wait_for_targets(std::list<Glib::ustring> &);

    // private properites
    SPDocument *_clipboardSPDoc; ///< Document that stores the clipboard until someone requests it
    Inkscape::XML::Node *_defs; ///< Reference to the clipboard document's defs node
    Inkscape::XML::Node *_root; ///< Reference to the clipboard's root node
    Inkscape::XML::Node *_clipnode; ///< The node that holds extra information
    Inkscape::XML::Document *_doc; ///< Reference to the clipboard's Inkscape::XML::Document

    // we need a way to copy plain text AND remember its style;
    // the standard _clipnode is only available in an SVG tree, hence this special storage
    SPCSSAttr *_text_style; ///< Style copied along with plain text fragment

    Glib::RefPtr<Gtk::Clipboard> _clipboard; ///< Handle to the system wide clipboard - for convenience
    std::list<Glib::ustring> _preferred_targets; ///< List of supported clipboard targets
};


ClipboardManagerImpl::ClipboardManagerImpl()
    : _clipboardSPDoc(NULL),
      _defs(NULL),
      _root(NULL),
      _clipnode(NULL),
      _doc(NULL),
      _text_style(NULL),
      _clipboard( Gtk::Clipboard::get() )
{
    // push supported clipboard targets, in order of preference
    _preferred_targets.push_back("image/x-inkscape-svg");
    _preferred_targets.push_back("image/svg+xml");
    _preferred_targets.push_back("image/svg+xml-compressed");
#ifdef WIN32
    _preferred_targets.push_back(CLIPBOARD_WIN32_EMF_MIME);
#endif
    _preferred_targets.push_back("application/pdf");
    _preferred_targets.push_back("image/x-adobe-illustrator");
}


ClipboardManagerImpl::~ClipboardManagerImpl() {}


/**
 * @brief Copy selection contents to the clipboard
 */
void ClipboardManagerImpl::copy(SPDesktop *desktop)
{
    if ( desktop == NULL ) {
        return;
    }
    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // Special case for when the gradient dragger is active - copies gradient color
    if (desktop->event_context->get_drag()) {
        GrDrag *drag = desktop->event_context->get_drag();
        if (drag->hasSelection()) {
            guint32 col = drag->getColor();

            // set the color as clipboard content (text in RRGGBBAA format)
            _setClipboardColor(col);

            // create a style with this color on fill and opacity in master opacity, so it can be
            // pasted on other stops or objects
            if (_text_style) {
                sp_repr_css_attr_unref(_text_style);
                _text_style = NULL;
            }
            _text_style = sp_repr_css_attr_new();
            // print and set properties
            gchar color_str[16];
            g_snprintf(color_str, 16, "#%06x", col >> 8);
            sp_repr_css_set_property(_text_style, "fill", color_str);
            float opacity = SP_RGBA32_A_F(col);
            if (opacity > 1.0) {
                opacity = 1.0; // safeguard
            }
            Inkscape::CSSOStringStream opcss;
            opcss << opacity;
            sp_repr_css_set_property(_text_style, "opacity", opcss.str().data());

            _discardInternalClipboard();
            return;
        }
    }

    // Special case for when the color picker ("dropper") is active - copies color under cursor
    if (tools_isactive(desktop, TOOLS_DROPPER)) {
        _setClipboardColor(sp_dropper_context_get_color(desktop->event_context));
        _discardInternalClipboard();
        return;
    }

    // Special case for when the text tool is active - if some text is selected, copy plain text,
    // not the object that holds it; also copy the style at cursor into
    if (tools_isactive(desktop, TOOLS_TEXT)) {
        _discardInternalClipboard();
        Glib::ustring selected_text = sp_text_get_selected_text(desktop->event_context);
        _clipboard->set_text(selected_text);
        if (_text_style) {
            sp_repr_css_attr_unref(_text_style);
            _text_style = NULL;
        }
        _text_style = sp_text_get_style_at_cursor(desktop->event_context);
        return;
    }

    if (selection->isEmpty()) {  // check whether something is selected
        _userWarn(desktop, _("Nothing was copied."));
        return;
    }
    _discardInternalClipboard();

    _createInternalClipboard();   // construct a new clipboard document
    _copySelection(selection);   // copy all items in the selection to the internal clipboard
    fit_canvas_to_drawing(_clipboardSPDoc);

    _setClipboardTargets();
}


/**
 * @brief Copy a Live Path Effect path parameter to the clipboard
 * @param pp The path parameter to store in the clipboard
 */
void ClipboardManagerImpl::copyPathParameter(Inkscape::LivePathEffect::PathParam *pp)
{
    if ( pp == NULL ) {
        return;
    }
    gchar *svgd = sp_svg_write_path( pp->get_pathvector() );
    if ( svgd == NULL || *svgd == '\0' ) {
        return;
    }

    _discardInternalClipboard();
    _createInternalClipboard();

    Inkscape::XML::Node *pathnode = _doc->createElement("svg:path");
    pathnode->setAttribute("d", svgd);
    g_free(svgd);
    _root->appendChild(pathnode);
    Inkscape::GC::release(pathnode);

    fit_canvas_to_drawing(_clipboardSPDoc);
    _setClipboardTargets();
}

/**
 * @brief Paste from the system clipboard into the active desktop
 * @param in_place Whether to put the contents where they were when copied
 */
bool ClipboardManagerImpl::paste(SPDesktop *desktop, bool in_place)
{
    // do any checking whether we really are able to paste before requesting the contents
    if ( desktop == NULL ) {
        return false;
    }
    if ( Inkscape::have_viable_layer(desktop, desktop->messageStack()) == false ) {
        return false;
    }

    Glib::ustring target = _getBestTarget();

    // Special cases of clipboard content handling go here
    // Note that target priority is determined in _getBestTarget.
    // TODO: Handle x-special/gnome-copied-files and text/uri-list to support pasting files

    // if there is an image on the clipboard, paste it
    if ( target == CLIPBOARD_GDK_PIXBUF_TARGET ) {
        return _pasteImage(desktop->doc());
    }
    // if there's only text, paste it into a selected text object or create a new one
    if ( target == CLIPBOARD_TEXT_TARGET ) {
        return _pasteText(desktop);
    }

    // otherwise, use the import extensions
    SPDocument *tempdoc = _retrieveClipboard(target);
    if ( tempdoc == NULL ) {
        _userWarn(desktop, _("Nothing on the clipboard."));
        return false;
    }

    _pasteDocument(desktop, tempdoc, in_place);
    sp_document_unref(tempdoc);

    return true;
}

/**
 * @brief Returns the id of the first visible copied object
 */
const gchar *ClipboardManagerImpl::getFirstObjectID()
{
    SPDocument *tempdoc = _retrieveClipboard("image/x-inkscape-svg");
    if ( tempdoc == NULL ) {
        return NULL;
    }

    Inkscape::XML::Node
        *root = sp_document_repr_root(tempdoc);

    if (!root) {
        return NULL;
    }

    Inkscape::XML::Node *ch = sp_repr_children(root);
    while (ch != NULL &&
           strcmp(ch->name(), "svg:g") &&
           strcmp(ch->name(), "svg:path") &&
           strcmp(ch->name(), "svg:use") &&
           strcmp(ch->name(), "svg:text") &&
           strcmp(ch->name(), "svg:image") &&
           strcmp(ch->name(), "svg:rect")
        ) {
        ch = ch->next();
    }

    if (ch) {
        return ch->attribute("id");
    }

    return NULL;
}


/**
 * @brief Implements the Paste Style action
 */
bool ClipboardManagerImpl::pasteStyle(SPDesktop *desktop)
{
    if (desktop == NULL) {
        return false;
    }

    // check whether something is selected
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    if (selection->isEmpty()) {
        _userWarn(desktop, _("Select <b>object(s)</b> to paste style to."));
        return false;
    }

    SPDocument *tempdoc = _retrieveClipboard("image/x-inkscape-svg");
    if ( tempdoc == NULL ) {
        // no document, but we can try _text_style
        if (_text_style) {
            sp_desktop_set_style(desktop, _text_style);
            return true;
        } else {
            _userWarn(desktop, _("No style on the clipboard."));
            return false;
        }
    }

    Inkscape::XML::Node
        *root = sp_document_repr_root(tempdoc),
        *clipnode = sp_repr_lookup_name(root, "inkscape:clipboard", 1);

    bool pasted = false;

    if (clipnode) {
        _pasteDefs(desktop, tempdoc);
        SPCSSAttr *style = sp_repr_css_attr(clipnode, "style");
        sp_desktop_set_style(desktop, style);
        pasted = true;
    }
    else {
        _userWarn(desktop, _("No style on the clipboard."));
    }

    sp_document_unref(tempdoc);
    return pasted;
}


/**
 * @brief Resize the selection or each object in the selection to match the clipboard's size
 * @param separately Whether to scale each object in the selection separately
 * @param apply_x Whether to scale the width of objects / selection
 * @param apply_y Whether to scale the height of objects / selection
 */
bool ClipboardManagerImpl::pasteSize(SPDesktop *desktop, bool separately, bool apply_x, bool apply_y)
{
    if (!apply_x && !apply_y) {
        return false; // pointless parameters
    }

    if ( desktop == NULL ) {
        return false;
    }
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    if (selection->isEmpty()) {
        _userWarn(desktop, _("Select <b>object(s)</b> to paste size to."));
        return false;
    }

    // FIXME: actually, this should accept arbitrary documents
    SPDocument *tempdoc = _retrieveClipboard("image/x-inkscape-svg");
    if ( tempdoc == NULL ) {
        _userWarn(desktop, _("No size on the clipboard."));
        return false;
    }

    // retrieve size ifomration from the clipboard
    Inkscape::XML::Node *root = sp_document_repr_root(tempdoc);
    Inkscape::XML::Node *clipnode = sp_repr_lookup_name(root, "inkscape:clipboard", 1);
    bool pasted = false;
    if (clipnode) {
        Geom::Point min, max;
        sp_repr_get_point(clipnode, "min", &min);
        sp_repr_get_point(clipnode, "max", &max);

        // resize each object in the selection
        if (separately) {
            for (GSList *i = const_cast<GSList*>(selection->itemList()) ; i ; i = i->next) {
                SPItem *item = SP_ITEM(i->data);
                Geom::OptRect obj_size = sp_item_bbox_desktop(item);
                if ( !obj_size ) {
                    continue;
                }
                sp_item_scale_rel(item, _getScale(desktop, min, max, *obj_size, apply_x, apply_y));
            }
        }
        // resize the selection as a whole
        else {
            Geom::OptRect sel_size = selection->bounds();
            if ( sel_size ) {
                sp_selection_scale_relative(selection, sel_size->midpoint(),
                                            _getScale(desktop, min, max, *sel_size, apply_x, apply_y));
            }
        }
        pasted = true;
    }
    sp_document_unref(tempdoc);
    return pasted;
}


/**
 * @brief Applies a path effect from the clipboard to the selected path
 */
bool ClipboardManagerImpl::pastePathEffect(SPDesktop *desktop)
{
    /** @todo FIXME: pastePathEffect crashes when moving the path with the applied effect,
        segfaulting in fork_private_if_necessary(). */

    if ( desktop == NULL ) {
        return false;
    }

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    if (selection && selection->isEmpty()) {
        _userWarn(desktop, _("Select <b>object(s)</b> to paste live path effect to."));
        return false;
    }

    SPDocument *tempdoc = _retrieveClipboard("image/x-inkscape-svg");
    if ( tempdoc ) {
        Inkscape::XML::Node *root = sp_document_repr_root(tempdoc);
        Inkscape::XML::Node *clipnode = sp_repr_lookup_name(root, "inkscape:clipboard", 1);
        if ( clipnode ) {
            gchar const *effectstack = clipnode->attribute("inkscape:path-effect");
            if ( effectstack ) {
                _pasteDefs(desktop, tempdoc);
                // make sure all selected items are converted to paths first (i.e. rectangles)
                sp_selected_to_lpeitems(desktop);
                for (GSList *itemptr = const_cast<GSList *>(selection->itemList()) ; itemptr ; itemptr = itemptr->next) {
                    SPItem *item = reinterpret_cast<SPItem*>(itemptr->data);
                    _applyPathEffect(item, effectstack);
                }

                return true;
            }
        }
    }

    // no_effect:
    _userWarn(desktop, _("No effect on the clipboard."));
    return false;
}


/**
 * @brief Get LPE path data from the clipboard
 * @return The retrieved path data (contents of the d attribute), or "" if no path was found
 */
Glib::ustring ClipboardManagerImpl::getPathParameter(SPDesktop* desktop)
{
    SPDocument *tempdoc = _retrieveClipboard(); // any target will do here
    if ( tempdoc == NULL ) {
        _userWarn(desktop, _("Nothing on the clipboard."));
        return "";
    }
    Inkscape::XML::Node
        *root = sp_document_repr_root(tempdoc),
        *path = sp_repr_lookup_name(root, "svg:path", -1); // unlimited search depth
    if ( path == NULL ) {
        _userWarn(desktop, _("Clipboard does not contain a path."));
        sp_document_unref(tempdoc);
        return "";
    }
    gchar const *svgd = path->attribute("d");
    return svgd;
}


/**
 * @brief Get object id of a shape or text item from the clipboard
 * @return The retrieved id string (contents of the id attribute), or "" if no shape or text item was found
 */
Glib::ustring ClipboardManagerImpl::getShapeOrTextObjectId(SPDesktop *desktop)
{
    SPDocument *tempdoc = _retrieveClipboard(); // any target will do here
    if ( tempdoc == NULL ) {
        _userWarn(desktop, _("Nothing on the clipboard."));
        return "";
    }
    Inkscape::XML::Node *root = sp_document_repr_root(tempdoc);

    Inkscape::XML::Node *repr = sp_repr_lookup_name(root, "svg:path", -1); // unlimited search depth
    if ( repr == NULL ) {
        repr = sp_repr_lookup_name(root, "svg:text", -1);
    }

    if ( repr == NULL ) {
        _userWarn(desktop, _("Clipboard does not contain a path."));
        sp_document_unref(tempdoc);
        return "";
    }
    gchar const *svgd = repr->attribute("id");
    return svgd;
}


/**
 * @brief Iterate over a list of items and copy them to the clipboard.
 */
void ClipboardManagerImpl::_copySelection(Inkscape::Selection *selection)
{
    GSList const *items = selection->itemList();
    // copy the defs used by all items
    for (GSList *i = const_cast<GSList *>(items) ; i != NULL ; i = i->next) {
        _copyUsedDefs(SP_ITEM (i->data));
    }

    // copy the representation of the items
    GSList *sorted_items = g_slist_copy(const_cast<GSList *>(items));
    sorted_items = g_slist_sort(sorted_items, (GCompareFunc) sp_object_compare_position);

    for (GSList *i = sorted_items ; i ; i = i->next) {
        if (!SP_IS_ITEM(i->data)) {
            continue;
        }
        Inkscape::XML::Node *obj = SP_OBJECT_REPR(i->data);
        Inkscape::XML::Node *obj_copy = _copyNode(obj, _doc, _root);

        // copy complete inherited style
        SPCSSAttr *css = sp_repr_css_attr_inherited(obj, "style");
        sp_repr_css_set(obj_copy, css, "style");
        sp_repr_css_attr_unref(css);

        // write the complete accumulated transform passed to us
        // (we're dealing with unattached representations, so we write to their attributes
        // instead of using sp_item_set_transform)
        gchar *transform_str = sp_svg_transform_write(sp_item_i2doc_affine(SP_ITEM(i->data)));
        obj_copy->setAttribute("transform", transform_str);
        g_free(transform_str);
    }

    // copy style for Paste Style action
    if (sorted_items) {
        if (SP_IS_ITEM(sorted_items->data)) {
            SPCSSAttr *style = take_style_from_item((SPItem *) sorted_items->data);
            sp_repr_css_set(_clipnode, style, "style");
            sp_repr_css_attr_unref(style);
        }

        // copy path effect from the first path
        if (SP_IS_OBJECT(sorted_items->data)) {
            gchar const *effect = SP_OBJECT_REPR(sorted_items->data)->attribute("inkscape:path-effect");
            if (effect) {
                _clipnode->setAttribute("inkscape:path-effect", effect);
            }
        }
    }

    Geom::OptRect size = selection->bounds();
    if (size) {
        sp_repr_set_point(_clipnode, "min", size->min());
        sp_repr_set_point(_clipnode, "max", size->max());
    }

    g_slist_free(sorted_items);
}


/**
 * @brief Recursively copy all the definitions used by a given item to the clipboard defs
 */
void ClipboardManagerImpl::_copyUsedDefs(SPItem *item)
{
    // copy fill and stroke styles (patterns and gradients)
    SPStyle *style = item->style;

    if (style && (style->fill.isPaintserver())) {
        SPPaintServer *server = item->style->getFillPaintServer();
        if ( SP_IS_LINEARGRADIENT(server) || SP_IS_RADIALGRADIENT(server) ) {
            _copyGradient(SP_GRADIENT(server));
        }
        if ( SP_IS_PATTERN(server) ) {
            _copyPattern(SP_PATTERN(server));
        }
    }
    if (style && (style->stroke.isPaintserver())) {
        SPPaintServer *server = item->style->getStrokePaintServer();
        if ( SP_IS_LINEARGRADIENT(server) || SP_IS_RADIALGRADIENT(server) ) {
            _copyGradient(SP_GRADIENT(server));
        }
        if ( SP_IS_PATTERN(server) ) {
            _copyPattern(SP_PATTERN(server));
        }
    }

    // For shapes, copy all of the shape's markers
    if (SP_IS_SHAPE(item)) {
        SPShape *shape = SP_SHAPE (item);
        for (int i = 0 ; i < SP_MARKER_LOC_QTY ; i++) {
            if (shape->marker[i]) {
                _copyNode(SP_OBJECT_REPR(SP_OBJECT(shape->marker[i])), _doc, _defs);
            }
        }
    }
    // For lpe items, copy lpe stack if applicable
    if (SP_IS_LPE_ITEM(item)) {
        SPLPEItem *lpeitem = SP_LPE_ITEM (item);
        if (sp_lpe_item_has_path_effect(lpeitem)) {
            for (PathEffectList::iterator it = lpeitem->path_effect_list->begin(); it != lpeitem->path_effect_list->end(); ++it)
            {
                LivePathEffectObject *lpeobj = (*it)->lpeobject;
                if (lpeobj) {
                    _copyNode(SP_OBJECT_REPR(SP_OBJECT(lpeobj)), _doc, _defs);
                }
            }
        }
    }
    // For 3D boxes, copy perspectives
    if (SP_IS_BOX3D(item)) {
        _copyNode(SP_OBJECT_REPR(SP_OBJECT(box3d_get_perspective(SP_BOX3D(item)))), _doc, _defs);
    }
    // Copy text paths
    if (SP_IS_TEXT_TEXTPATH(item)) {
        _copyTextPath(SP_TEXTPATH(sp_object_first_child(SP_OBJECT(item))));
    }
    // Copy clipping objects
    if (item->clip_ref->getObject()) {
        _copyNode(SP_OBJECT_REPR(item->clip_ref->getObject()), _doc, _defs);
    }
    // Copy mask objects
    if (item->mask_ref->getObject()) {
        SPObject *mask = item->mask_ref->getObject();
        _copyNode(SP_OBJECT_REPR(mask), _doc, _defs);
        // recurse into the mask for its gradients etc.
        for (SPObject *o = SP_OBJECT(mask)->children ; o != NULL ; o = o->next) {
            if (SP_IS_ITEM(o)) {
                _copyUsedDefs(SP_ITEM(o));
            }
        }
    }
    // Copy filters
    if (style->getFilter()) {
        SPObject *filter = style->getFilter();
        if (SP_IS_FILTER(filter)) {
            _copyNode(SP_OBJECT_REPR(filter), _doc, _defs);
        }
    }

    // recurse
    for (SPObject *o = SP_OBJECT(item)->children ; o != NULL ; o = o->next) {
        if (SP_IS_ITEM(o)) {
            _copyUsedDefs(SP_ITEM(o));
        }
    }
}


/**
 * @brief Copy a single gradient to the clipboard's defs element
 */
void ClipboardManagerImpl::_copyGradient(SPGradient *gradient)
{
    while (gradient) {
        // climb up the refs, copying each one in the chain
        _copyNode(SP_OBJECT_REPR(gradient), _doc, _defs);
        gradient = gradient->ref->getObject();
    }
}


/**
 * @brief Copy a single pattern to the clipboard document's defs element
 */
void ClipboardManagerImpl::_copyPattern(SPPattern *pattern)
{
    // climb up the references, copying each one in the chain
    while (pattern) {
        _copyNode(SP_OBJECT_REPR(pattern), _doc, _defs);

        // items in the pattern may also use gradients and other patterns, so recurse
        for (SPObject *child = sp_object_first_child(SP_OBJECT(pattern)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            if (!SP_IS_ITEM (child)) {
                continue;
            }
            _copyUsedDefs(SP_ITEM(child));
        }
        pattern = pattern->ref->getObject();
    }
}


/**
 * @brief Copy a text path to the clipboard's defs element
 */
void ClipboardManagerImpl::_copyTextPath(SPTextPath *tp)
{
    SPItem *path = sp_textpath_get_path_item(tp);
    if (!path) {
        return;
    }
    Inkscape::XML::Node *path_node = SP_OBJECT_REPR(path);

    // Do not copy the text path to defs if it's already copied
    if (sp_repr_lookup_child(_root, "id", path_node->attribute("id"))) {
        return;
    }
    _copyNode(path_node, _doc, _defs);
}


/**
 * @brief Copy a single XML node from one document to another
 * @param node The node to be copied
 * @param target_doc The document to which the node is to be copied
 * @param parent The node in the target document which will become the parent of the copied node
 * @return Pointer to the copied node
 */
Inkscape::XML::Node *ClipboardManagerImpl::_copyNode(Inkscape::XML::Node *node, Inkscape::XML::Document *target_doc, Inkscape::XML::Node *parent)
{
    Inkscape::XML::Node *dup = node->duplicate(target_doc);
    parent->appendChild(dup);
    Inkscape::GC::release(dup);
    return dup;
}


/**
 * @brief Paste the contents of a document into the active desktop
 * @param clipdoc The document to paste
 * @param in_place Whether to paste the selection where it was when copied
 * @pre @c clipdoc is not empty and items can be added to the current layer
 */
void ClipboardManagerImpl::_pasteDocument(SPDesktop *desktop, SPDocument *clipdoc, bool in_place)
{
    SPDocument *target_document = sp_desktop_document(desktop);
    Inkscape::XML::Node
        *root = sp_document_repr_root(clipdoc),
        *target_parent = SP_OBJECT_REPR(desktop->currentLayer());
    Inkscape::XML::Document *target_xmldoc = sp_document_repr_doc(target_document);

    // copy definitions
    _pasteDefs(desktop, clipdoc);

    // copy objects
    GSList *pasted_objects = NULL;
    for (Inkscape::XML::Node *obj = root->firstChild() ; obj ; obj = obj->next()) {
        // Don't copy metadata, defs, named views and internal clipboard contents to the document
        if (!strcmp(obj->name(), "svg:defs")) {
            continue;
        }
        if (!strcmp(obj->name(), "svg:metadata")) {
            continue;
        }
        if (!strcmp(obj->name(), "sodipodi:namedview")) {
            continue;
        }
        if (!strcmp(obj->name(), "inkscape:clipboard")) {
            continue;
        }
        Inkscape::XML::Node *obj_copy = _copyNode(obj, target_xmldoc, target_parent);
        pasted_objects = g_slist_prepend(pasted_objects, (gpointer) obj_copy);
    }

    // Change the selection to the freshly pasted objects
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    selection->setReprList(pasted_objects);

    // invers apply parent transform
    Geom::Matrix doc2parent = sp_item_i2doc_affine(SP_ITEM(desktop->currentLayer())).inverse();
    sp_selection_apply_affine(selection, desktop->dt2doc() * doc2parent * desktop->doc2dt(), true, false);

    // Update (among other things) all curves in paths, for bounds() to work
    sp_document_ensure_up_to_date(target_document);

    // move selection either to original position (in_place) or to mouse pointer
    Geom::OptRect sel_bbox = selection->bounds();
    if (sel_bbox) {
        // get offset of selection to original position of copied elements
        Geom::Point pos_original;
        Inkscape::XML::Node *clipnode = sp_repr_lookup_name(root, "inkscape:clipboard", 1);
        if (clipnode) {
            Geom::Point min, max;
            sp_repr_get_point(clipnode, "min", &min);
            sp_repr_get_point(clipnode, "max", &max);
            pos_original = Geom::Point(min[Geom::X], max[Geom::Y]);
        }
        Geom::Point offset = pos_original - sel_bbox->corner(3);

        if (!in_place) {
            SnapManager &m = desktop->namedview->snap_manager;
            m.setup(desktop);
            sp_event_context_discard_delayed_snap_event(desktop->event_context);

            // get offset from mouse pointer to bbox center, snap to grid if enabled
            Geom::Point mouse_offset = desktop->point() - sel_bbox->midpoint();
            offset = m.multipleOfGridPitch(mouse_offset - offset, sel_bbox->midpoint() + offset) + offset;
        }

        sp_selection_move_relative(selection, offset);
    }

    g_slist_free(pasted_objects);
}


/**
 * @brief Paste SVG defs from the document retrieved from the clipboard into the active document
 * @param clipdoc The document to paste
 * @pre @c clipdoc != NULL and pasting into the active document is possible
 */
void ClipboardManagerImpl::_pasteDefs(SPDesktop *desktop, SPDocument *clipdoc)
{
    // boilerplate vars copied from _pasteDocument
    SPDocument *target_document = sp_desktop_document(desktop);
    Inkscape::XML::Node
        *root = sp_document_repr_root(clipdoc),
        *defs = sp_repr_lookup_name(root, "svg:defs", 1),
        *target_defs = SP_OBJECT_REPR(SP_DOCUMENT_DEFS(target_document));
    Inkscape::XML::Document *target_xmldoc = sp_document_repr_doc(target_document);

    prevent_id_clashes(clipdoc, target_document);

    for (Inkscape::XML::Node *def = defs->firstChild() ; def ; def = def->next()) {
        _copyNode(def, target_xmldoc, target_defs);
    }
}


/**
 * @brief Retrieve a bitmap image from the clipboard and paste it into the active document
 */
bool ClipboardManagerImpl::_pasteImage(SPDocument *doc)
{
    if ( doc == NULL ) {
        return false;
    }

    // retrieve image data
    Glib::RefPtr<Gdk::Pixbuf> img = _clipboard->wait_for_image();
    if (!img) {
        return false;
    }

    // TODO unify with interface.cpp's sp_ui_drag_data_received()
    // AARGH stupid
    Inkscape::Extension::DB::InputList o;
    Inkscape::Extension::db.get_input_list(o);
    Inkscape::Extension::DB::InputList::const_iterator i = o.begin();
    while (i != o.end() && strcmp( (*i)->get_mimetype(), "image/png" ) != 0) {
        ++i;
    }
    Inkscape::Extension::Extension *png = *i;
    bool save = (strcmp(png->get_param_optiongroup("link"), "embed") == 0);
    png->set_param_optiongroup("link", "embed");
    png->set_gui(false);

    gchar *filename = g_build_filename( g_get_tmp_dir(), "inkscape-clipboard-import", NULL );
    img->save(filename, "png");
    file_import(doc, filename, png);
    g_free(filename);

    png->set_param_optiongroup("link", save ? "embed" : "link");
    png->set_gui(true);

    return true;
}

/**
 * @brief Paste text into the selected text object or create a new one to hold it
 */
bool ClipboardManagerImpl::_pasteText(SPDesktop *desktop)
{
    if ( desktop == NULL ) {
        return false;
    }

    // if the text editing tool is active, paste the text into the active text object
    if (tools_isactive(desktop, TOOLS_TEXT)) {
        return sp_text_paste_inline(desktop->event_context);
    }

    // try to parse the text as a color and, if successful, apply it as the current style
    SPCSSAttr *css = _parseColor(_clipboard->wait_for_text());
    if (css) {
        sp_desktop_set_style(desktop, css);
        return true;
    }

    return false;
}


/**
 * @brief Attempt to parse the passed string as a hexadecimal RGB or RGBA color
 * @param text The Glib::ustring to parse
 * @return New CSS style representation if the parsing was successful, NULL otherwise
 */
SPCSSAttr *ClipboardManagerImpl::_parseColor(const Glib::ustring &text)
{
// TODO reuse existing code instead of replicating here.
    Glib::ustring::size_type len = text.bytes();
    char *str = const_cast<char *>(text.data());
    bool attempt_alpha = false;
    if ( !str || ( *str == '\0' ) ) {
        return NULL; // this is OK due to boolean short-circuit
    }

    // those conditionals guard against parsing e.g. the string "fab" as "fab000"
    // (incomplete color) and "45fab71" as "45fab710" (incomplete alpha)
    if ( *str == '#' ) {
        if ( len < 7 ) {
            return NULL;
        }
        if ( len >= 9 ) {
            attempt_alpha = true;
        }
    } else {
        if ( len < 6 ) {
            return NULL;
        }
        if ( len >= 8 ) {
            attempt_alpha = true;
        }
    }

    unsigned int color = 0, alpha = 0xff;

    // skip a leading #, if present
    if ( *str == '#' ) {
        ++str;
    }

    // try to parse first 6 digits
    int res = sscanf(str, "%6x", &color);
    if ( res && ( res != EOF ) ) {
        if (attempt_alpha) {// try to parse alpha if there's enough characters
            sscanf(str + 6, "%2x", &alpha);
            if ( !res || res == EOF ) {
                alpha = 0xff;
            }
        }

        SPCSSAttr *color_css = sp_repr_css_attr_new();

        // print and set properties
        gchar color_str[16];
        g_snprintf(color_str, 16, "#%06x", color);
        sp_repr_css_set_property(color_css, "fill", color_str);

        float opacity = static_cast<float>(alpha)/static_cast<float>(0xff);
        if (opacity > 1.0) {
            opacity = 1.0; // safeguard
        }
        Inkscape::CSSOStringStream opcss;
        opcss << opacity;
        sp_repr_css_set_property(color_css, "fill-opacity", opcss.str().data());
        return color_css;
    }
    return NULL;
}


/**
 * @brief Applies a pasted path effect to a given item
 */
void ClipboardManagerImpl::_applyPathEffect(SPItem *item, gchar const *effectstack)
{
    if ( item == NULL ) {
        return;
    }
    if ( SP_IS_RECT(item) ) {
        return;
    }

    if (SP_IS_LPE_ITEM(item))
    {
        SPLPEItem *lpeitem = SP_LPE_ITEM(item);
        // for each effect in the stack, check if we need to fork it before adding it to the item
        sp_lpe_item_fork_path_effects_if_necessary(lpeitem, 1);

        std::istringstream iss(effectstack);
        std::string href;
        while (std::getline(iss, href, ';'))
        {
            SPObject *obj = sp_uri_reference_resolve(_clipboardSPDoc, href.c_str());
            if (!obj) {
                return;
            }
            LivePathEffectObject *lpeobj = LIVEPATHEFFECT(obj);
            sp_lpe_item_add_path_effect(lpeitem, lpeobj);
        }
    }
}


/**
 * @brief Retrieve the clipboard contents as a document
 * @return Clipboard contents converted to SPDocument, or NULL if no suitable content was present
 */
SPDocument *ClipboardManagerImpl::_retrieveClipboard(Glib::ustring required_target)
{
    Glib::ustring best_target;
    if ( required_target == "" ) {
        best_target = _getBestTarget();
    } else {
        best_target = required_target;
    }

    if ( best_target == "" ) {
        return NULL;
    }

    // FIXME: Temporary hack until we add memory input.
    // Save the clipboard contents to some file, then read it
    gchar *filename = g_build_filename( g_get_tmp_dir(), "inkscape-clipboard-import", NULL );

    bool file_saved = false;
    Glib::ustring target = best_target;

#ifdef WIN32
    if (best_target == CLIPBOARD_WIN32_EMF_TARGET)
    {   // Try to save clipboard data as en emf file (using win32 api)
        if (OpenClipboard(NULL)) {
            HGLOBAL hglb = GetClipboardData(CF_ENHMETAFILE);
            if (hglb) {
                HENHMETAFILE hemf = CopyEnhMetaFile((HENHMETAFILE) hglb, filename);
                if (hemf) {
                    file_saved = true;
                    target = CLIPBOARD_WIN32_EMF_MIME;
                    DeleteEnhMetaFile(hemf);
                }
            }
            CloseClipboard();
        }
    }
#endif

    if (!file_saved) {
        if ( !_clipboard->wait_is_target_available(best_target) ) {
            return NULL;
        }

        // doing this synchronously makes better sense
        // TODO: use another method because this one is badly broken imo.
        // from documentation: "Returns: A SelectionData object, which will be invalid if retrieving the given target failed."
        // I don't know how to check whether an object is 'valid' or not, unusable if that's not possible...
        Gtk::SelectionData sel = _clipboard->wait_for_contents(best_target);
        target = sel.get_target();  // this can crash if the result was invalid of last function. No way to check for this :(

        // FIXME: Temporary hack until we add memory input.
        // Save the clipboard contents to some file, then read it
        g_file_set_contents(filename, (const gchar *) sel.get_data(), sel.get_length(), NULL);
    }

    // there is no specific plain SVG input extension, so if we can paste the Inkscape SVG format,
    // we use the image/svg+xml mimetype to look up the input extension
    if (target == "image/x-inkscape-svg") {
        target = "image/svg+xml";
    }

    Inkscape::Extension::DB::InputList inlist;
    Inkscape::Extension::db.get_input_list(inlist);
    Inkscape::Extension::DB::InputList::const_iterator in = inlist.begin();
    for (; in != inlist.end() && target != (*in)->get_mimetype() ; ++in) {
    };
    if ( in == inlist.end() ) {
        return NULL; // this shouldn't happen unless _getBestTarget returns something bogus
    }

    SPDocument *tempdoc = NULL;
    try {
        tempdoc = (*in)->open(filename);
    } catch (...) {
    }
    g_unlink(filename);
    g_free(filename);

    return tempdoc;
}


/**
 * @brief Callback called when some other application requests data from Inkscape
 *
 * Finds a suitable output extension to save the internal clipboard document,
 * then saves it to memory and sets the clipboard contents.
 */
void ClipboardManagerImpl::_onGet(Gtk::SelectionData &sel, guint /*info*/)
{
    g_assert( _clipboardSPDoc != NULL );

    Glib::ustring target = sel.get_target();
    if (target == "") {
        return; // this shouldn't happen
    }

    if (target == CLIPBOARD_TEXT_TARGET) {
        target = "image/x-inkscape-svg";
    }

    Inkscape::Extension::DB::OutputList outlist;
    Inkscape::Extension::db.get_output_list(outlist);
    Inkscape::Extension::DB::OutputList::const_iterator out = outlist.begin();
    for ( ; out != outlist.end() && target != (*out)->get_mimetype() ; ++out) {
    };
    if ( out == outlist.end() && target != "image/png") {
        return; // this also shouldn't happen
    }

    // FIXME: Temporary hack until we add support for memory output.
    // Save to a temporary file, read it back and then set the clipboard contents
    gchar *filename = g_build_filename( g_get_tmp_dir(), "inkscape-clipboard-export", NULL );
    gsize len; gchar *data;

    try {
        if (out == outlist.end() && target == "image/png")
        {
            gdouble dpi = PX_PER_IN;
            guint32 bgcolor = 0x00000000;

            Geom::Point origin (SP_ROOT(_clipboardSPDoc->root)->x.computed, SP_ROOT(_clipboardSPDoc->root)->y.computed);
            Geom::Rect area = Geom::Rect(origin, origin + sp_document_dimensions(_clipboardSPDoc));

            unsigned long int width = (unsigned long int) (area.width() * dpi / PX_PER_IN + 0.5);
            unsigned long int height = (unsigned long int) (area.height() * dpi / PX_PER_IN + 0.5);

            // read from namedview
            Inkscape::XML::Node *nv = sp_repr_lookup_name (_clipboardSPDoc->rroot, "sodipodi:namedview");
            if (nv && nv->attribute("pagecolor")) {
                bgcolor = sp_svg_read_color(nv->attribute("pagecolor"), 0xffffff00);
            }
            if (nv && nv->attribute("inkscape:pageopacity")) {
                bgcolor |= SP_COLOR_F_TO_U(sp_repr_get_double_attribute (nv, "inkscape:pageopacity", 1.0));
            }

            sp_export_png_file(_clipboardSPDoc, filename, area, width, height, dpi, dpi, bgcolor, NULL, NULL, true, NULL);
        }
        else
        {
            if (!(*out)->loaded()) {
                // Need to load the extension.
                (*out)->set_state(Inkscape::Extension::Extension::STATE_LOADED);
            }
            (*out)->save(_clipboardSPDoc, filename);
        }
        g_file_get_contents(filename, &data, &len, NULL);

        sel.set(8, (guint8 const *) data, len);
    } catch (...) {
    }

    g_unlink(filename); // delete the temporary file
    g_free(filename);
}


/**
 * @brief Callback when someone else takes the clipboard
 *
 * When the clipboard owner changes, this callback clears the internal clipboard document
 * to reduce memory usage.
 */
void ClipboardManagerImpl::_onClear()
{
    // why is this called before _onGet???
    //_discardInternalClipboard();
}


/**
 * @brief Creates an internal clipboard document from scratch
 */
void ClipboardManagerImpl::_createInternalClipboard()
{
    if ( _clipboardSPDoc == NULL ) {
        _clipboardSPDoc = sp_document_new(NULL, false, true);
        //g_assert( _clipboardSPDoc != NULL );
        _defs = SP_OBJECT_REPR(SP_DOCUMENT_DEFS(_clipboardSPDoc));
        _doc = sp_document_repr_doc(_clipboardSPDoc);
        _root = sp_document_repr_root(_clipboardSPDoc);

        _clipnode = _doc->createElement("inkscape:clipboard");
        _root->appendChild(_clipnode);
        Inkscape::GC::release(_clipnode);

        // once we create a SVG document, style will be stored in it, so flush _text_style
        if (_text_style) {
            sp_repr_css_attr_unref(_text_style);
            _text_style = NULL;
        }
    }
}


/**
 * @brief Deletes the internal clipboard document
 */
void ClipboardManagerImpl::_discardInternalClipboard()
{
    if ( _clipboardSPDoc != NULL ) {
        sp_document_unref(_clipboardSPDoc);
        _clipboardSPDoc = NULL;
        _defs = NULL;
        _doc = NULL;
        _root = NULL;
        _clipnode = NULL;
    }
}


/**
 * @brief Get the scale to resize an item, based on the command and desktop state
 */
Geom::Scale ClipboardManagerImpl::_getScale(SPDesktop *desktop, Geom::Point const &min, Geom::Point const &max, Geom::Rect const &obj_rect, bool apply_x, bool apply_y)
{
    double scale_x = 1.0;
    double scale_y = 1.0;

    if (apply_x) {
        scale_x = (max[Geom::X] - min[Geom::X]) / obj_rect[Geom::X].extent();
    }
    if (apply_y) {
        scale_y = (max[Geom::Y] - min[Geom::Y]) / obj_rect[Geom::Y].extent();
    }
    // If the "lock aspect ratio" button is pressed and we paste only a single coordinate,
    // resize the second one by the same ratio too
    if (desktop->isToolboxButtonActive("lock")) {
        if (apply_x && !apply_y) {
            scale_y = scale_x;
        }
        if (apply_y && !apply_x) {
            scale_x = scale_y;
        }
    }

    return Geom::Scale(scale_x, scale_y);
}


/**
 * @brief Find the most suitable clipboard target
 */
Glib::ustring ClipboardManagerImpl::_getBestTarget()
{
    // GTKmm's wait_for_targets() is broken, see the comment in _inkscape_wait_for_targets()
    std::list<Glib::ustring> targets; // = _clipboard->wait_for_targets();
    _inkscape_wait_for_targets(targets);

    // clipboard target debugging snippet
    /*
    g_debug("Begin clipboard targets");
    for ( std::list<Glib::ustring>::iterator x = targets.begin() ; x != targets.end(); ++x )
        g_debug("Clipboard target: %s", (*x).data());
    g_debug("End clipboard targets\n");
    //*/

    for (std::list<Glib::ustring>::iterator i = _preferred_targets.begin() ;
        i != _preferred_targets.end() ; ++i)
    {
        if ( std::find(targets.begin(), targets.end(), *i) != targets.end() ) {
            return *i;
        }
    }
#ifdef WIN32
    if (OpenClipboard(NULL))
    {   // If both bitmap and metafile are present, pick the one that was exported first.
        UINT format = EnumClipboardFormats(0);
        while (format) {
            if (format == CF_ENHMETAFILE || format == CF_DIB || format == CF_BITMAP) {
                break;
            }
            format = EnumClipboardFormats(format);
        }
        CloseClipboard();

        if (format == CF_ENHMETAFILE) {
            return CLIPBOARD_WIN32_EMF_TARGET;
        }
        if (format == CF_DIB || format == CF_BITMAP) {
            return CLIPBOARD_GDK_PIXBUF_TARGET;
        }
    }

    if (IsClipboardFormatAvailable(CF_ENHMETAFILE)) {
        return CLIPBOARD_WIN32_EMF_TARGET;
    }
#endif
    if (_clipboard->wait_is_image_available()) {
        return CLIPBOARD_GDK_PIXBUF_TARGET;
    }
    if (_clipboard->wait_is_text_available()) {
        return CLIPBOARD_TEXT_TARGET;
    }

    return "";
}


/**
 * @brief Set the clipboard targets to reflect the mimetypes Inkscape can output
 */
void ClipboardManagerImpl::_setClipboardTargets()
{
    Inkscape::Extension::DB::OutputList outlist;
    Inkscape::Extension::db.get_output_list(outlist);
    std::list<Gtk::TargetEntry> target_list;
    bool plaintextSet = false;
    for (Inkscape::Extension::DB::OutputList::const_iterator out = outlist.begin() ; out != outlist.end() ; ++out) {
        if ( !(*out)->deactivated() ) {
            Glib::ustring mime = (*out)->get_mimetype();
            if (mime != CLIPBOARD_TEXT_TARGET) {
                if ( !plaintextSet && (mime.find("svg") == Glib::ustring::npos) ) {
                    target_list.push_back(Gtk::TargetEntry(CLIPBOARD_TEXT_TARGET));
                    plaintextSet = true;
                }
                target_list.push_back(Gtk::TargetEntry(mime));
            }
        }
    }

    // Add PNG export explicitly since there is no extension for this...
    // On Windows, GTK will also present this as a CF_DIB/CF_BITMAP
    target_list.push_back(Gtk::TargetEntry( "image/png" ));

    _clipboard->set(target_list,
        sigc::mem_fun(*this, &ClipboardManagerImpl::_onGet),
        sigc::mem_fun(*this, &ClipboardManagerImpl::_onClear));

#ifdef WIN32
    // If the "image/x-emf" target handled by the emf extension would be
    // presented as a CF_ENHMETAFILE automatically (just like an "image/bmp"
    // is presented as a CF_BITMAP) this code would not be needed.. ???
    // Or maybe there is some other way to achieve the same?

    // Note: Metafile is the only format that is rendered and stored in clipboard
    // on Copy, all other formats are rendered only when needed by a Paste command.

    // FIXME: This should at least be rewritten to use "delayed rendering".
    //        If possible make it delayed rendering by using GTK API only.

    if (OpenClipboard(NULL)) {
        if ( _clipboardSPDoc != NULL ) {
            const Glib::ustring target = CLIPBOARD_WIN32_EMF_MIME;

            Inkscape::Extension::DB::OutputList outlist;
            Inkscape::Extension::db.get_output_list(outlist);
            Inkscape::Extension::DB::OutputList::const_iterator out = outlist.begin();
            for ( ; out != outlist.end() && target != (*out)->get_mimetype() ; ++out) {
            }
            if ( out != outlist.end() ) {
                // FIXME: Temporary hack until we add support for memory output.
                // Save to a temporary file, read it back and then set the clipboard contents
                gchar *filename = g_build_filename( g_get_tmp_dir(), "inkscape-clipboard-export.emf", NULL );

                try {
                    (*out)->save(_clipboardSPDoc, filename);
                    HENHMETAFILE hemf = GetEnhMetaFileA(filename);
                    if (hemf) {
                        SetClipboardData(CF_ENHMETAFILE, hemf);
                        DeleteEnhMetaFile(hemf);
                    }
                } catch (...) {
                }
                g_unlink(filename); // delete the temporary file
                g_free(filename);
            }
        }
        CloseClipboard();
    }
#endif
}


/**
 * @brief Set the string representation of a 32-bit RGBA color as the clipboard contents
 */
void ClipboardManagerImpl::_setClipboardColor(guint32 color)
{
    gchar colorstr[16];
    g_snprintf(colorstr, 16, "%08x", color);
    _clipboard->set_text(colorstr);
}


/**
 * @brief Put a notification on the mesage stack
 */
void ClipboardManagerImpl::_userWarn(SPDesktop *desktop, char const *msg)
{
    desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, msg);
}


// GTKMM's clipboard::wait_for_targets is buggy and might return bogus, see
//
// https://bugs.launchpad.net/inkscape/+bug/296778
// http://mail.gnome.org/archives/gtk-devel-list/2009-June/msg00062.html
//
// for details. Until this has been fixed upstream we will use our own implementation
// of this method, as copied from /gtkmm-2.16.0/gtk/gtkmm/clipboard.cc.
void ClipboardManagerImpl::_inkscape_wait_for_targets(std::list<Glib::ustring> &listTargets)
{
    //Get a newly-allocated array of atoms:
    GdkAtom* targets = 0;
    gint n_targets = 0;
    gboolean test = gtk_clipboard_wait_for_targets( gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), &targets, &n_targets );
    if (!test) {
        n_targets = 0; //otherwise it will be -1.
    }

    //Add the targets to the C++ container:
    for (int i = 0; i < n_targets; i++)
    {
        //Convert the atom to a string:
        gchar* const atom_name = gdk_atom_name(targets[i]);

        Glib::ustring target;
        if (atom_name) {
            target = Glib::ScopedPtr<char>(atom_name).get(); //This frees the gchar*.
        }

        listTargets.push_back(target);
    }
}

/* #######################################
          ClipboardManager class
   ####################################### */

ClipboardManager *ClipboardManager::_instance = NULL;

ClipboardManager::ClipboardManager() {}
ClipboardManager::~ClipboardManager() {}
ClipboardManager *ClipboardManager::get()
{
    if ( _instance == NULL ) {
        _instance = new ClipboardManagerImpl;
    }

    return _instance;
}

} // namespace Inkscape
} // namespace IO

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
