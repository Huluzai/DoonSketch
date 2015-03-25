#define __SP_DOCUMENT_C__

/** \file
 * SPDocument manipulation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2004-2005 MenTaLguY
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/** \class SPDocument
 * SPDocument serves as the container of both model trees (agnostic XML
 * and typed object tree), and implements all of the document-level
 * functionality used by the program. Many document level operations, like
 * load, save, print, export and so on, use SPDocument as their basic datatype.
 *
 * SPDocument implements undo and redo stacks and an id-based object
 * dictionary.  Thanks to unique id attributes, the latter can be used to
 * map from the XML tree back to the object tree.
 *
 * SPDocument performs the basic operations needed for asynchronous
 * update notification (SPObject ::modified virtual method), and implements
 * the 'modified' signal, as well.
 */


#define noSP_DOCUMENT_DEBUG_IDLE
#define noSP_DOCUMENT_DEBUG_UNDO

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <gtk/gtk.h>
#include <string>
#include <cstring>

#include "application/application.h"
#include "application/editor.h"
#include "desktop.h"
#include "dir-util.h"
#include "display/nr-arena-item.h"
#include "document-private.h"
#include "helper/units.h"
#include "inkscape-private.h"
#include "inkscape-version.h"
#include "libavoid/router.h"
#include "persp3d.h"
#include "preferences.h"
#include "profile-manager.h"
#include "rdf.h"
#include "sp-item-group.h"
#include "sp-namedview.h"
#include "sp-object-repr.h"
#include "transf_mat_3x4.h"
#include "unit-constants.h"
#include "xml/repr.h"
#include "xml/rebase-hrefs.h"

// Higher number means lower priority.
#define SP_DOCUMENT_UPDATE_PRIORITY (G_PRIORITY_HIGH_IDLE - 2)

// Should have a lower priority than SP_DOCUMENT_UPDATE_PRIORITY,
// since we want it to happen when there are no more updates.
#define SP_DOCUMENT_REROUTING_PRIORITY (G_PRIORITY_HIGH_IDLE - 1)


static gint sp_document_idle_handler(gpointer data);
static gint sp_document_rerouting_handler(gpointer data);

gboolean sp_document_resource_list_free(gpointer key, gpointer value, gpointer data);

static gint doc_count = 0;

static unsigned long next_serial = 0;

SPDocument::SPDocument() :
    keepalive(FALSE),
    virgin(TRUE),
    modified_since_save(FALSE),
    rdoc(0),
    rroot(0),
    root(0),
    style_cascade(cr_cascade_new(NULL, NULL, NULL)),
    uri(0),
    base(0),
    name(0),
    priv(0), // reset in ctor
    actionkey(),
    modified_id(0),
    rerouting_handler_id(0),
    profileManager(0), // deferred until after other initialization
    router(new Avoid::Router(Avoid::PolyLineRouting|Avoid::OrthogonalRouting)),
    _collection_queue(0),
    oldSignalsConnected(false),
    current_persp3d(0)
{
    // Penalise libavoid for choosing paths with needless extra segments.
    // This results in much better looking orthogonal connector paths.
    router->setRoutingPenalty(Avoid::segmentPenalty);

    SPDocumentPrivate *p = new SPDocumentPrivate();

    p->serial = next_serial++;

    p->iddef = g_hash_table_new(g_direct_hash, g_direct_equal);
    p->reprdef = g_hash_table_new(g_direct_hash, g_direct_equal);

    p->resources = g_hash_table_new(g_str_hash, g_str_equal);

    p->sensitive = FALSE;
    p->partial = NULL;
    p->history_size = 0;
    p->undo = NULL;
    p->redo = NULL;
    p->seeking = false;

    priv = p;

    // Once things are set, hook in the manager
    profileManager = new Inkscape::ProfileManager(this);

    // XXX only for testing!
    priv->undoStackObservers.add(p->console_output_undo_observer);
}

SPDocument::~SPDocument() {
    collectOrphans();

    // kill/unhook this first
    if ( profileManager ) {
        delete profileManager;
        profileManager = 0;
    }

    if (router) {
        delete router;
        router = NULL;
    }

    if (priv) {
        if (priv->partial) {
            sp_repr_free_log(priv->partial);
            priv->partial = NULL;
        }

        sp_document_clear_redo(this);
        sp_document_clear_undo(this);

        if (root) {
            root->releaseReferences();
            sp_object_unref(root);
            root = NULL;
        }

        if (priv->iddef) g_hash_table_destroy(priv->iddef);
        if (priv->reprdef) g_hash_table_destroy(priv->reprdef);

        if (rdoc) Inkscape::GC::release(rdoc);

        /* Free resources */
        g_hash_table_foreach_remove(priv->resources, sp_document_resource_list_free, this);
        g_hash_table_destroy(priv->resources);

        delete priv;
        priv = NULL;
    }

    cr_cascade_unref(style_cascade);
    style_cascade = NULL;

    if (name) {
        g_free(name);
        name = NULL;
    }
    if (base) {
        g_free(base);
        base = NULL;
    }
    if (uri) {
        g_free(uri);
        uri = NULL;
    }

    if (modified_id) {
        g_source_remove(modified_id);
        modified_id = 0;
    }

    if (rerouting_handler_id) {
        g_source_remove(rerouting_handler_id);
        rerouting_handler_id = 0;
    }

    if (oldSignalsConnected) {
        g_signal_handlers_disconnect_by_func(G_OBJECT(INKSCAPE),
                                             reinterpret_cast<gpointer>(sp_document_reset_key),
                                             static_cast<gpointer>(this));
    } else {
        _selection_changed_connection.disconnect();
        _desktop_activated_connection.disconnect();
    }

    if (keepalive) {
        inkscape_unref();
        keepalive = FALSE;
    }
    //delete this->_whiteboard_session_manager;
}

Persp3D *
SPDocument::getCurrentPersp3D() {
    // Check if current_persp3d is still valid
    std::vector<Persp3D*> plist;
    getPerspectivesInDefs(plist);
    for (unsigned int i = 0; i < plist.size(); ++i) {
        if (current_persp3d == plist[i])
            return current_persp3d;
    }

    // If not, return the first perspective in defs (which may be NULL of none exists)
    current_persp3d = persp3d_document_first_persp (this);

    return current_persp3d;
}

Persp3DImpl *
SPDocument::getCurrentPersp3DImpl() {
    return current_persp3d_impl;
}

void
SPDocument::setCurrentPersp3D(Persp3D * const persp) {
    current_persp3d = persp;
    //current_persp3d_impl = persp->perspective_impl;
}

void
SPDocument::getPerspectivesInDefs(std::vector<Persp3D*> &list) {
    SPDefs *defs = SP_ROOT(this->root)->defs;
    for (SPObject *i = sp_object_first_child(SP_OBJECT(defs)); i != NULL; i = SP_OBJECT_NEXT(i) ) {
        if (SP_IS_PERSP3D(i))
            list.push_back(SP_PERSP3D(i));
    }
}

/**
void SPDocument::initialize_current_persp3d()
{
    this->current_persp3d = persp3d_document_first_persp(this);
    if (!this->current_persp3d) {
        this->current_persp3d = persp3d_create_xml_element(this);
    }
}
**/

unsigned long SPDocument::serial() const {
    return priv->serial;
}

void SPDocument::queueForOrphanCollection(SPObject *object) {
    g_return_if_fail(object != NULL);
    g_return_if_fail(SP_OBJECT_DOCUMENT(object) == this);

    sp_object_ref(object, NULL);
    _collection_queue = g_slist_prepend(_collection_queue, object);
}

void SPDocument::collectOrphans() {
    while (_collection_queue) {
        GSList *objects=_collection_queue;
        _collection_queue = NULL;
        for ( GSList *iter=objects ; iter ; iter = iter->next ) {
            SPObject *object=reinterpret_cast<SPObject *>(iter->data);
            object->collectOrphan();
            sp_object_unref(object, NULL);
        }
        g_slist_free(objects);
    }
}

void SPDocument::reset_key (void */*dummy*/)
{
    actionkey.clear();
}

SPDocument *
sp_document_create(Inkscape::XML::Document *rdoc,
                   gchar const *uri,
                   gchar const *base,
                   gchar const *name,
                   unsigned int keepalive)
{
    SPDocument *document;
    Inkscape::XML::Node *rroot;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    rroot = rdoc->root();

    document = new SPDocument();

    document->keepalive = keepalive;

    document->rdoc = rdoc;
    document->rroot = rroot;

#ifndef WIN32
    document->uri = prepend_current_dir_if_relative(uri);
#else
    // FIXME: it may be that prepend_current_dir_if_relative works OK on windows too, test!
    document->uri = uri? g_strdup(uri) : NULL;
#endif

    // base is simply the part of the path before filename; e.g. when running "inkscape ../file.svg" the base is "../"
    // which is why we use g_get_current_dir() in calculating the abs path above
    //This is NULL for a new document
    if (base)
        document->base = g_strdup(base);
    else
        document->base = NULL;
    document->name = g_strdup(name);

    document->root = sp_object_repr_build_tree(document, rroot);

    /* fixme: Not sure about this, but lets assume ::build updates */
    rroot->setAttribute("inkscape:version", Inkscape::version_string);
    /* fixme: Again, I moved these here to allow version determining in ::build (Lauris) */

    /* Quick hack 2 - get default image size into document */
    if (!rroot->attribute("width")) rroot->setAttribute("width", "100%");
    if (!rroot->attribute("height")) rroot->setAttribute("height", "100%");
    /* End of quick hack 2 */

    /* Quick hack 3 - Set uri attributes */
    if (uri) {
        rroot->setAttribute("sodipodi:docname", uri);
    }
    /* End of quick hack 3 */

    /* Eliminate obsolete sodipodi:docbase, for privacy reasons */
    rroot->setAttribute("sodipodi:docbase", NULL);

    /* Eliminate any claim to adhere to a profile, as we don't try to */
    rroot->setAttribute("baseProfile", NULL);

    // creating namedview
    if (!sp_item_group_get_child_by_name((SPGroup *) document->root, NULL, "sodipodi:namedview")) {
        // if there's none in the document already,
        Inkscape::XML::Node *rnew = NULL;

        rnew = rdoc->createElement("sodipodi:namedview");
        //rnew->setAttribute("id", "base");

        // Add namedview data from the preferences
        // we can't use getAllEntries because this could produce non-SVG doubles
        Glib::ustring pagecolor = prefs->getString("/template/base/pagecolor");
        if (!pagecolor.empty()) {
            rnew->setAttribute("pagecolor", pagecolor.data());
        }
        Glib::ustring bordercolor = prefs->getString("/template/base/bordercolor");
        if (!bordercolor.empty()) {
            rnew->setAttribute("bordercolor", bordercolor.data());
        }
        sp_repr_set_svg_double(rnew, "borderopacity",
            prefs->getDouble("/template/base/borderopacity", 1.0));
        sp_repr_set_svg_double(rnew, "objecttolerance",
            prefs->getDouble("/template/base/objecttolerance", 10.0));
        sp_repr_set_svg_double(rnew, "gridtolerance",
            prefs->getDouble("/template/base/gridtolerance", 10.0));
        sp_repr_set_svg_double(rnew, "guidetolerance",
            prefs->getDouble("/template/base/guidetolerance", 10.0));
        sp_repr_set_svg_double(rnew, "inkscape:pageopacity",
            prefs->getDouble("/template/base/inkscape:pageopacity", 0.0));
        sp_repr_set_int(rnew, "inkscape:pageshadow",
            prefs->getInt("/template/base/inkscape:pageshadow", 2));
        sp_repr_set_int(rnew, "inkscape:window-width",
            prefs->getInt("/template/base/inkscape:window-width", 640));
        sp_repr_set_int(rnew, "inkscape:window-height",
            prefs->getInt("/template/base/inkscape:window-height", 480));

        // insert into the document
        rroot->addChild(rnew, NULL);
        // clean up
        Inkscape::GC::release(rnew);
    }

    /* Defs */
    if (!SP_ROOT(document->root)->defs) {
        Inkscape::XML::Node *r;
        r = rdoc->createElement("svg:defs");
        rroot->addChild(r, NULL);
        Inkscape::GC::release(r);
        g_assert(SP_ROOT(document->root)->defs);
    }

    /* Default RDF */
    rdf_set_defaults( document );

    if (keepalive) {
        inkscape_ref();
    }

    // Check if the document already has a perspective (e.g., when opening an existing
    // document). If not, create a new one and set it as the current perspective.
    document->setCurrentPersp3D(persp3d_document_first_persp(document));
    if (!document->getCurrentPersp3D()) {
        //document->setCurrentPersp3D(persp3d_create_xml_element (document));
        Persp3DImpl *persp_impl = new Persp3DImpl();
        document->setCurrentPersp3DImpl(persp_impl);
    }

    sp_document_set_undo_sensitive(document, true);

    // reset undo key when selection changes, so that same-key actions on different objects are not coalesced
    if (!Inkscape::NSApplication::Application::getNewGui()) {
        g_signal_connect(G_OBJECT(INKSCAPE), "change_selection",
                         G_CALLBACK(sp_document_reset_key), document);
        g_signal_connect(G_OBJECT(INKSCAPE), "activate_desktop",
                         G_CALLBACK(sp_document_reset_key), document);
        document->oldSignalsConnected = true;
    } else {
        document->_selection_changed_connection = Inkscape::NSApplication::Editor::connectSelectionChanged (sigc::mem_fun (*document, &SPDocument::reset_key));
        document->_desktop_activated_connection = Inkscape::NSApplication::Editor::connectDesktopActivated (sigc::mem_fun (*document, &SPDocument::reset_key));
        document->oldSignalsConnected = false;
    }

    return document;
}

/**
 * Fetches document from URI, or creates new, if NULL; public document
 * appears in document list.
 */
SPDocument *
sp_document_new(gchar const *uri, unsigned int keepalive, bool make_new)
{
    SPDocument *doc;
    Inkscape::XML::Document *rdoc;
    gchar *base = NULL;
    gchar *name = NULL;

    if (uri) {
        Inkscape::XML::Node *rroot;
        gchar *s, *p;
        /* Try to fetch repr from file */
        rdoc = sp_repr_read_file(uri, SP_SVG_NS_URI);
        /* If file cannot be loaded, return NULL without warning */
        if (rdoc == NULL) return NULL;
        rroot = rdoc->root();
        /* If xml file is not svg, return NULL without warning */
        /* fixme: destroy document */
        if (strcmp(rroot->name(), "svg:svg") != 0) return NULL;
        s = g_strdup(uri);
        p = strrchr(s, '/');
        if (p) {
            name = g_strdup(p + 1);
            p[1] = '\0';
            base = g_strdup(s);
        } else {
            base = NULL;
            name = g_strdup(uri);
        }
        g_free(s);
    } else {
        rdoc = sp_repr_document_new("svg:svg");
    }

    if (make_new) {
        base = NULL;
        uri = NULL;
        name = g_strdup_printf(_("New document %d"), ++doc_count);
    }

    //# These should be set by now
    g_assert(name);

    doc = sp_document_create(rdoc, uri, base, name, keepalive);

    g_free(base);
    g_free(name);

    return doc;
}

SPDocument *
sp_document_new_from_mem(gchar const *buffer, gint length, unsigned int keepalive)
{
    SPDocument *doc;
    Inkscape::XML::Document *rdoc;
    Inkscape::XML::Node *rroot;
    gchar *name;

    rdoc = sp_repr_read_mem(buffer, length, SP_SVG_NS_URI);

    /* If it cannot be loaded, return NULL without warning */
    if (rdoc == NULL) return NULL;

    rroot = rdoc->root();
    /* If xml file is not svg, return NULL without warning */
    /* fixme: destroy document */
    if (strcmp(rroot->name(), "svg:svg") != 0) return NULL;

    name = g_strdup_printf(_("Memory document %d"), ++doc_count);

    doc = sp_document_create(rdoc, NULL, NULL, name, keepalive);

    return doc;
}

SPDocument *
sp_document_ref(SPDocument *doc)
{
    g_return_val_if_fail(doc != NULL, NULL);
    Inkscape::GC::anchor(doc);
    return doc;
}

SPDocument *
sp_document_unref(SPDocument *doc)
{
    g_return_val_if_fail(doc != NULL, NULL);
    Inkscape::GC::release(doc);
    return NULL;
}

gdouble sp_document_width(SPDocument *document)
{
    g_return_val_if_fail(document != NULL, 0.0);
    g_return_val_if_fail(document->priv != NULL, 0.0);
    g_return_val_if_fail(document->root != NULL, 0.0);

    SPRoot *root = SP_ROOT(document->root);

    if (root->width.unit == SVGLength::PERCENT && root->viewBox_set)
        return root->viewBox.x1 - root->viewBox.x0;
    return root->width.computed;
}

void
sp_document_set_width (SPDocument *document, gdouble width, const SPUnit *unit)
{
    SPRoot *root = SP_ROOT(document->root);

    if (root->width.unit == SVGLength::PERCENT && root->viewBox_set) { // set to viewBox=
        root->viewBox.x1 = root->viewBox.x0 + sp_units_get_pixels (width, *unit);
    } else { // set to width=
        gdouble old_computed = root->width.computed;
        root->width.computed = sp_units_get_pixels (width, *unit);
        /* SVG does not support meters as a unit, so we must translate meters to
         * cm when writing */
        if (!strcmp(unit->abbr, "m")) {
            root->width.value = 100*width;
            root->width.unit = SVGLength::CM;
        } else {
            root->width.value = width;
            root->width.unit = (SVGLength::Unit) sp_unit_get_svg_unit(unit);
        }

        if (root->viewBox_set)
            root->viewBox.x1 = root->viewBox.x0 + (root->width.computed / old_computed) * (root->viewBox.x1 - root->viewBox.x0);
    }

    SP_OBJECT (root)->updateRepr();
}

void sp_document_set_height (SPDocument * document, gdouble height, const SPUnit *unit)
{
    SPRoot *root = SP_ROOT(document->root);

    if (root->height.unit == SVGLength::PERCENT && root->viewBox_set) { // set to viewBox=
        root->viewBox.y1 = root->viewBox.y0 + sp_units_get_pixels (height, *unit);
    } else { // set to height=
        gdouble old_computed = root->height.computed;
        root->height.computed = sp_units_get_pixels (height, *unit);
        /* SVG does not support meters as a unit, so we must translate meters to
         * cm when writing */
        if (!strcmp(unit->abbr, "m")) {
            root->height.value = 100*height;
            root->height.unit = SVGLength::CM;
        } else {
            root->height.value = height;
            root->height.unit = (SVGLength::Unit) sp_unit_get_svg_unit(unit);
        }

        if (root->viewBox_set)
            root->viewBox.y1 = root->viewBox.y0 + (root->height.computed / old_computed) * (root->viewBox.y1 - root->viewBox.y0);
    }

    SP_OBJECT (root)->updateRepr();
}

gdouble sp_document_height(SPDocument *document)
{
    g_return_val_if_fail(document != NULL, 0.0);
    g_return_val_if_fail(document->priv != NULL, 0.0);
    g_return_val_if_fail(document->root != NULL, 0.0);

    SPRoot *root = SP_ROOT(document->root);

    if (root->height.unit == SVGLength::PERCENT && root->viewBox_set)
        return root->viewBox.y1 - root->viewBox.y0;
    return root->height.computed;
}

Geom::Point sp_document_dimensions(SPDocument *doc)
{
    return Geom::Point(sp_document_width(doc), sp_document_height(doc));
}

/**
 * Gets page fitting margin information from the namedview node in the XML.
 * \param nv_repr reference to this document's namedview
 * \param key the same key used by the RegisteredScalarUnit in
 *        ui/widget/page-sizer.cpp
 * \param margin_units units for the margin
 * \param return_units units to return the result in
 * \param width width in px (for percentage margins)
 * \param height height in px (for percentage margins)
 * \param use_width true if the this key is left or right margins, false
 *        otherwise.  Used for percentage margins.
 * \return the margin size in px, else 0.0 if anything is invalid.
 */
static double getMarginLength(Inkscape::XML::Node * const nv_repr,
                             gchar const * const key,
                             SPUnit const * const margin_units,
                             SPUnit const * const return_units,
                             double const width,
                             double const height,
                             bool const use_width)
{
    double value;
    if (!sp_repr_get_double (nv_repr, key, &value)) {
        return 0.0;
    }
    if (margin_units == &sp_unit_get_by_id (SP_UNIT_PERCENT)) {
        return (use_width)? width * value : height * value; 
    }
    if (!sp_convert_distance (&value, margin_units, return_units)) {
        return 0.0;
    }
    return value;
}

/**
 * Given a Geom::Rect that may, for example, correspond to the bbox of an object,
 * this function fits the canvas to that rect by resizing the canvas
 * and translating the document root into position.
 * \param rect fit document size to this
 * \param with_margins add margins to rect, by taking margins from this
 *        document's namedview (<sodipodi:namedview> "fit-margin-..."
 *        attributes, and "units")
 */
void SPDocument::fitToRect(Geom::Rect const &rect, bool with_margins)
{
    double const w = rect.width();
    double const h = rect.height();

    double const old_height = sp_document_height(this);
    SPUnit const &px(sp_unit_get_by_id(SP_UNIT_PX));
    
    /* in px */
    double margin_top = 0.0;
    double margin_left = 0.0;
    double margin_right = 0.0;
    double margin_bottom = 0.0;
    
    SPNamedView *nv = sp_document_namedview(this, 0);
    
    if (with_margins && nv) {
        Inkscape::XML::Node *nv_repr = SP_OBJECT_REPR (nv);
        if (nv_repr != NULL) {
            gchar const * const units_abbr = nv_repr->attribute("units");
            SPUnit const *margin_units = NULL;
            if (units_abbr != NULL) {
                margin_units = sp_unit_get_by_abbreviation(units_abbr);
            }
            if (margin_units == NULL) {
                margin_units = &px;
            }
            margin_top = getMarginLength(nv_repr, "fit-margin-top",
                                         margin_units, &px, w, h, false);
            margin_left = getMarginLength(nv_repr, "fit-margin-left",
                                          margin_units, &px, w, h, true);
            margin_right = getMarginLength(nv_repr, "fit-margin-right",
                                           margin_units, &px, w, h, true);
            margin_bottom = getMarginLength(nv_repr, "fit-margin-bottom",
                                            margin_units, &px, w, h, false);
        }
    }
    
    Geom::Rect const rect_with_margins(
            rect.min() - Geom::Point(margin_left, margin_bottom),
            rect.max() + Geom::Point(margin_right, margin_top));
    
    
    sp_document_set_width(this, rect_with_margins.width(), &px);
    sp_document_set_height(this, rect_with_margins.height(), &px);

    Geom::Translate const tr(
            Geom::Point(0, old_height - rect_with_margins.height())
            - to_2geom(rect_with_margins.min()));
    SP_GROUP(root)->translateChildItems(tr);

    if(nv) {
        Geom::Translate tr2(-rect_with_margins.min());
        nv->translateGuides(tr2);
        nv->translateGrids(tr2);

        // update the viewport so the drawing appears to stay where it was
        nv->scrollAllDesktops(-tr2[0], tr2[1], false);
    }
}

static void
do_change_uri(SPDocument *const document, gchar const *const filename, bool const rebase)
{
    g_return_if_fail(document != NULL);

    gchar *new_base;
    gchar *new_name;
    gchar *new_uri;
    if (filename) {

#ifndef WIN32
        new_uri = prepend_current_dir_if_relative(filename);
#else
        // FIXME: it may be that prepend_current_dir_if_relative works OK on windows too, test!
        new_uri = g_strdup(filename);
#endif

        new_base = g_path_get_dirname(new_uri);
        new_name = g_path_get_basename(new_uri);
    } else {
        new_uri = g_strdup_printf(_("Unnamed document %d"), ++doc_count);
        new_base = NULL;
        new_name = g_strdup(document->uri);
    }

    // Update saveable repr attributes.
    Inkscape::XML::Node *repr = sp_document_repr_root(document);

    // Changing uri in the document repr must not be not undoable.
    bool const saved = sp_document_get_undo_sensitive(document);
    sp_document_set_undo_sensitive(document, false);

    if (rebase) {
        Inkscape::XML::rebase_hrefs(document, new_base, true);
    }

    repr->setAttribute("sodipodi:docname", document->name);
    sp_document_set_undo_sensitive(document, saved);


    g_free(document->name);
    g_free(document->base);
    g_free(document->uri);
    document->name = new_name;
    document->base = new_base;
    document->uri = new_uri;

    document->priv->uri_set_signal.emit(document->uri);
}

/**
 * Sets base, name and uri members of \a document.  Doesn't update
 * any relative hrefs in the document: thus, this is primarily for
 * newly-created documents.
 *
 * \see sp_document_change_uri_and_hrefs
 */
void sp_document_set_uri(SPDocument *document, gchar const *filename)
{
    g_return_if_fail(document != NULL);

    do_change_uri(document, filename, false);
}

/**
 * Changes the base, name and uri members of \a document, and updates any
 * relative hrefs in the document to be relative to the new base.
 *
 * \see sp_document_set_uri
 */
void sp_document_change_uri_and_hrefs(SPDocument *document, gchar const *filename)
{
    g_return_if_fail(document != NULL);

    do_change_uri(document, filename, true);
}

void
sp_document_resized_signal_emit(SPDocument *doc, gdouble width, gdouble height)
{
    g_return_if_fail(doc != NULL);

    doc->priv->resized_signal.emit(width, height);
}

sigc::connection SPDocument::connectModified(SPDocument::ModifiedSignal::slot_type slot)
{
    return priv->modified_signal.connect(slot);
}

sigc::connection SPDocument::connectURISet(SPDocument::URISetSignal::slot_type slot)
{
    return priv->uri_set_signal.connect(slot);
}

sigc::connection SPDocument::connectResized(SPDocument::ResizedSignal::slot_type slot)
{
    return priv->resized_signal.connect(slot);
}

sigc::connection
SPDocument::connectReconstructionStart(SPDocument::ReconstructionStart::slot_type slot)
{
    return priv->_reconstruction_start_signal.connect(slot);
}

void
SPDocument::emitReconstructionStart(void)
{
    // printf("Starting Reconstruction\n");
    priv->_reconstruction_start_signal.emit();
    return;
}

sigc::connection
SPDocument::connectReconstructionFinish(SPDocument::ReconstructionFinish::slot_type  slot)
{
    return priv->_reconstruction_finish_signal.connect(slot);
}

void
SPDocument::emitReconstructionFinish(void)
{
    // printf("Finishing Reconstruction\n");
    priv->_reconstruction_finish_signal.emit();

/**    
    // Reference to the old persp3d object is invalid after reconstruction.
    initialize_current_persp3d();
    
    return;
**/
}

sigc::connection SPDocument::connectCommit(SPDocument::CommitSignal::slot_type slot)
{
    return priv->commit_signal.connect(slot);
}



void SPDocument::_emitModified() {
    static guint const flags = SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG;
    root->emitModified(0);
    priv->modified_signal.emit(flags);
}

void SPDocument::bindObjectToId(gchar const *id, SPObject *object) {
    GQuark idq = g_quark_from_string(id);

    if (object) {
        g_assert(g_hash_table_lookup(priv->iddef, GINT_TO_POINTER(idq)) == NULL);
        g_hash_table_insert(priv->iddef, GINT_TO_POINTER(idq), object);
    } else {
        g_assert(g_hash_table_lookup(priv->iddef, GINT_TO_POINTER(idq)) != NULL);
        g_hash_table_remove(priv->iddef, GINT_TO_POINTER(idq));
    }

    SPDocumentPrivate::IDChangedSignalMap::iterator pos;

    pos = priv->id_changed_signals.find(idq);
    if ( pos != priv->id_changed_signals.end() ) {
        if (!(*pos).second.empty()) {
            (*pos).second.emit(object);
        } else { // discard unused signal
            priv->id_changed_signals.erase(pos);
        }
    }
}

void
SPDocument::addUndoObserver(Inkscape::UndoStackObserver& observer)
{
    this->priv->undoStackObservers.add(observer);
}

void
SPDocument::removeUndoObserver(Inkscape::UndoStackObserver& observer)
{
    this->priv->undoStackObservers.remove(observer);
}

SPObject *SPDocument::getObjectById(gchar const *id) {
    g_return_val_if_fail(id != NULL, NULL);

    GQuark idq = g_quark_from_string(id);
    return (SPObject*)g_hash_table_lookup(priv->iddef, GINT_TO_POINTER(idq));
}

sigc::connection SPDocument::connectIdChanged(gchar const *id,
                                              SPDocument::IDChangedSignal::slot_type slot)
{
    return priv->id_changed_signals[g_quark_from_string(id)].connect(slot);
}

void SPDocument::bindObjectToRepr(Inkscape::XML::Node *repr, SPObject *object) {
    if (object) {
        g_assert(g_hash_table_lookup(priv->reprdef, repr) == NULL);
        g_hash_table_insert(priv->reprdef, repr, object);
    } else {
        g_assert(g_hash_table_lookup(priv->reprdef, repr) != NULL);
        g_hash_table_remove(priv->reprdef, repr);
    }
}

SPObject *SPDocument::getObjectByRepr(Inkscape::XML::Node *repr) {
    g_return_val_if_fail(repr != NULL, NULL);
    return (SPObject*)g_hash_table_lookup(priv->reprdef, repr);
}

Glib::ustring SPDocument::getLanguage() {
    gchar const *document_language = rdf_get_work_entity(this, rdf_find_entity("language"));
    if (document_language) {
        while (isspace(*document_language))
            document_language++;
    }
    if ( !document_language || 0 == *document_language) {
        // retrieve system language
        document_language = getenv("LC_ALL");
        if ( NULL == document_language || *document_language == 0 ) {
            document_language = getenv ("LC_MESSAGES");
        }
        if ( NULL == document_language || *document_language == 0 ) {
            document_language = getenv ("LANG");
        }

        if ( NULL != document_language ) {
            const char *pos = strchr(document_language, '_');
            if ( NULL != pos ) {
                return Glib::ustring(document_language, pos - document_language);
            }
        }
    }

    if ( NULL == document_language )
        return Glib::ustring();
    return document_language;
}

/* Object modification root handler */

void
sp_document_request_modified(SPDocument *doc)
{
    if (!doc->modified_id) {
        doc->modified_id = g_idle_add_full(SP_DOCUMENT_UPDATE_PRIORITY, 
                sp_document_idle_handler, doc, NULL);
    }
    if (!doc->rerouting_handler_id) {
        doc->rerouting_handler_id = g_idle_add_full(SP_DOCUMENT_REROUTING_PRIORITY, 
                sp_document_rerouting_handler, doc, NULL);
    }
}

void
sp_document_setup_viewport (SPDocument *doc, SPItemCtx *ctx)
{
    ctx->ctx.flags = 0;
    ctx->i2doc = Geom::identity();
    /* Set up viewport in case svg has it defined as percentages */
    if (SP_ROOT(doc->root)->viewBox_set) { // if set, take from viewBox
        ctx->vp.x0 = SP_ROOT(doc->root)->viewBox.x0;
        ctx->vp.y0 = SP_ROOT(doc->root)->viewBox.y0;
        ctx->vp.x1 = SP_ROOT(doc->root)->viewBox.x1;
        ctx->vp.y1 = SP_ROOT(doc->root)->viewBox.y1;
    } else { // as a last resort, set size to A4
        ctx->vp.x0 = 0.0;
        ctx->vp.y0 = 0.0;
        ctx->vp.x1 = 210 * PX_PER_MM;
        ctx->vp.y1 = 297 * PX_PER_MM;
    }
    ctx->i2vp = Geom::identity();
}

/**
 * Tries to update the document state based on the modified and
 * "update required" flags, and return true if the document has
 * been brought fully up to date.
 */
bool
SPDocument::_updateDocument()
{
    /* Process updates */
    if (this->root->uflags || this->root->mflags) {
        if (this->root->uflags) {
            SPItemCtx ctx;
            sp_document_setup_viewport (this, &ctx);

            bool saved = sp_document_get_undo_sensitive(this);
            sp_document_set_undo_sensitive(this, false);

            this->root->updateDisplay((SPCtx *)&ctx, 0);

            sp_document_set_undo_sensitive(this, saved);
        }
        this->_emitModified();
    }

    return !(this->root->uflags || this->root->mflags);
}


/**
 * Repeatedly works on getting the document updated, since sometimes
 * it takes more than one pass to get the document updated.  But it
 * usually should not take more than a few loops, and certainly never
 * more than 32 iterations.  So we bail out if we hit 32 iterations,
 * since this typically indicates we're stuck in an update loop.
 */
gint
sp_document_ensure_up_to_date(SPDocument *doc)
{
    // Bring the document up-to-date, specifically via the following:
    //   1a) Process all document updates.
    //   1b) When completed, process connector routing changes.
    //   2a) Process any updates resulting from connector reroutings.
    int counter = 32;
    for (unsigned int pass = 1; pass <= 2; ++pass) {
        // Process document updates.
        while (!doc->_updateDocument()) {
            if (counter == 0) {
                g_warning("More than 32 iteration while updating document '%s'", doc->uri);
                break;
            }
            counter--;
        }
        if (counter == 0)
        {
            break;
        }

        // After updates on the first pass we get libavoid to process all the 
        // changed objects and provide new routings.  This may cause some objects
            // to be modified, hence the second update pass.
        if (pass == 1) {
            doc->router->processTransaction();
        }
    }
    
    if (doc->modified_id) {
        /* Remove handler */
        g_source_remove(doc->modified_id);
        doc->modified_id = 0;
    }
    if (doc->rerouting_handler_id) {
        /* Remove handler */
        g_source_remove(doc->rerouting_handler_id);
        doc->rerouting_handler_id = 0;
    }
    return counter>0;
}

/**
 * An idle handler to update the document.  Returns true if
 * the document needs further updates.
 */
static gint
sp_document_idle_handler(gpointer data)
{
    SPDocument *doc = static_cast<SPDocument *>(data);
    if (doc->_updateDocument()) {
        doc->modified_id = 0;
        return false;
    } else {
        return true;
    }
}

/**
 * An idle handler to reroute connectors in the document.  
 */
static gint
sp_document_rerouting_handler(gpointer data)
{
    // Process any queued movement actions and determine new routings for 
    // object-avoiding connectors.  Callbacks will be used to update and 
    // redraw affected connectors.
    SPDocument *doc = static_cast<SPDocument *>(data);
    doc->router->processTransaction();
    
    // We don't need to handle rerouting again until there are further 
    // diagram updates.
    doc->rerouting_handler_id = 0;
    return false;
}

static bool is_within(Geom::Rect const &area, Geom::Rect const &box)
{
    return area.contains(box);
}

static bool overlaps(Geom::Rect const &area, Geom::Rect const &box)
{
    return area.intersects(box);
}

static GSList *find_items_in_area(GSList *s, SPGroup *group, unsigned int dkey, Geom::Rect const &area,
                                  bool (*test)(Geom::Rect const &, Geom::Rect const &), bool take_insensitive = false)
{
    g_return_val_if_fail(SP_IS_GROUP(group), s);

    for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
        if (!SP_IS_ITEM(o)) {
            continue;
        }
        if (SP_IS_GROUP(o) && SP_GROUP(o)->effectiveLayerMode(dkey) == SPGroup::LAYER ) {
            s = find_items_in_area(s, SP_GROUP(o), dkey, area, test);
        } else {
            SPItem *child = SP_ITEM(o);
            Geom::OptRect box = sp_item_bbox_desktop(child);
            if ( box && test(area, *box) && (take_insensitive || child->isVisibleAndUnlocked(dkey))) {
                s = g_slist_append(s, child);
            }
        }
    }

    return s;
}

/**
Returns true if an item is among the descendants of group (recursively).
 */
bool item_is_in_group(SPItem *item, SPGroup *group)
{
    for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
        if (!SP_IS_ITEM(o)) continue;
        if (SP_ITEM(o) == item)
            return true;
        if (SP_IS_GROUP(o))
            if (item_is_in_group(item, SP_GROUP(o)))
                return true;
    }
    return false;
}

/**
Returns the bottommost item from the list which is at the point, or NULL if none.
*/
SPItem*
sp_document_item_from_list_at_point_bottom(unsigned int dkey, SPGroup *group, GSList const *list,
                                           Geom::Point const p, bool take_insensitive)
{
    g_return_val_if_fail(group, NULL);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble delta = prefs->getDouble("/options/cursortolerance/value", 1.0);

    for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {

        if (!SP_IS_ITEM(o)) continue;

        SPItem *item = SP_ITEM(o);
        NRArenaItem *arenaitem = sp_item_get_arenaitem(item, dkey);
        if (arenaitem && nr_arena_item_invoke_pick(arenaitem, p, delta, NR_ARENA_ITEM_PICK_STICKY) != NULL
            && (take_insensitive || item->isVisibleAndUnlocked(dkey))) {
            if (g_slist_find((GSList *) list, item) != NULL)
                return item;
        }

        if (SP_IS_GROUP(o)) {
            SPItem *found = sp_document_item_from_list_at_point_bottom(dkey, SP_GROUP(o), list, p, take_insensitive);
            if (found)
                return found;
        }

    }
    return NULL;
}

/**
Returns the topmost (in z-order) item from the descendants of group (recursively) which
is at the point p, or NULL if none. Honors into_groups on whether to recurse into
non-layer groups or not. Honors take_insensitive on whether to return insensitive
items. If upto != NULL, then if item upto is encountered (at any level), stops searching
upwards in z-order and returns what it has found so far (i.e. the found item is
guaranteed to be lower than upto).
 */
SPItem*
find_item_at_point(unsigned int dkey, SPGroup *group, Geom::Point const p, gboolean into_groups, bool take_insensitive = false, SPItem *upto = NULL)
{
    SPItem *seen = NULL, *newseen = NULL;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble delta = prefs->getDouble("/options/cursortolerance/value", 1.0);

    for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
        if (!SP_IS_ITEM(o)) continue;

        if (upto && SP_ITEM(o) == upto)
            break;

        if (SP_IS_GROUP(o) && (SP_GROUP(o)->effectiveLayerMode(dkey) == SPGroup::LAYER || into_groups)) {
            // if nothing found yet, recurse into the group
            newseen = find_item_at_point(dkey, SP_GROUP(o), p, into_groups, take_insensitive, upto);
            if (newseen) {
                seen = newseen;
                newseen = NULL;
            }

            if (item_is_in_group(upto, SP_GROUP(o)))
                break;

        } else {
            SPItem *child = SP_ITEM(o);
            NRArenaItem *arenaitem = sp_item_get_arenaitem(child, dkey);

            // seen remembers the last (topmost) of items pickable at this point
            if (arenaitem && nr_arena_item_invoke_pick(arenaitem, p, delta, NR_ARENA_ITEM_PICK_STICKY) != NULL
                && (take_insensitive || child->isVisibleAndUnlocked(dkey))) {
                seen = child;
            }
        }
    }
    return seen;
}

/**
Returns the topmost non-layer group from the descendants of group which is at point
p, or NULL if none. Recurses into layers but not into groups.
 */
SPItem*
find_group_at_point(unsigned int dkey, SPGroup *group, Geom::Point const p)
{
    SPItem *seen = NULL;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble delta = prefs->getDouble("/options/cursortolerance/value", 1.0);

    for (SPObject *o = sp_object_first_child(SP_OBJECT(group)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
        if (!SP_IS_ITEM(o)) continue;
        if (SP_IS_GROUP(o) && SP_GROUP(o)->effectiveLayerMode(dkey) == SPGroup::LAYER) {
            SPItem *newseen = find_group_at_point(dkey, SP_GROUP(o), p);
            if (newseen) {
                seen = newseen;
            }
        }
        if (SP_IS_GROUP(o) && SP_GROUP(o)->effectiveLayerMode(dkey) != SPGroup::LAYER ) {
            SPItem *child = SP_ITEM(o);
            NRArenaItem *arenaitem = sp_item_get_arenaitem(child, dkey);

            // seen remembers the last (topmost) of groups pickable at this point
            if (arenaitem && nr_arena_item_invoke_pick(arenaitem, p, delta, NR_ARENA_ITEM_PICK_STICKY) != NULL) {
                seen = child;
            }
        }
    }
    return seen;
}

/*
 * Return list of items, contained in box
 *
 * Assumes box is normalized (and g_asserts it!)
 *
 */

GSList *sp_document_items_in_box(SPDocument *document, unsigned int dkey, Geom::Rect const &box)
{
    g_return_val_if_fail(document != NULL, NULL);
    g_return_val_if_fail(document->priv != NULL, NULL);

    return find_items_in_area(NULL, SP_GROUP(document->root), dkey, box, is_within);
}

/*
 * Return list of items, that the parts of the item contained in box
 *
 * Assumes box is normalized (and g_asserts it!)
 *
 */

GSList *sp_document_partial_items_in_box(SPDocument *document, unsigned int dkey, Geom::Rect const &box)
{
    g_return_val_if_fail(document != NULL, NULL);
    g_return_val_if_fail(document->priv != NULL, NULL);

    return find_items_in_area(NULL, SP_GROUP(document->root), dkey, box, overlaps);
}

GSList *
sp_document_items_at_points(SPDocument *document, unsigned const key, std::vector<Geom::Point> points)
{
    GSList *items = NULL;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // When picking along the path, we don't want small objects close together
    // (such as hatching strokes) to obscure each other by their deltas,
    // so we temporarily set delta to a small value
    gdouble saved_delta = prefs->getDouble("/options/cursortolerance/value", 1.0);
    prefs->setDouble("/options/cursortolerance/value", 0.25);

    for(unsigned int i = 0; i < points.size(); i++) {
        SPItem *item = sp_document_item_at_point(document, key, points[i],
                                                 false, NULL);
        if (item && !g_slist_find(items, item))
            items = g_slist_prepend (items, item);
    }

    // and now we restore it back
    prefs->setDouble("/options/cursortolerance/value", saved_delta);

    return items;
}

SPItem *
sp_document_item_at_point(SPDocument *document, unsigned const key, Geom::Point const p,
                          gboolean const into_groups, SPItem *upto)
{
    g_return_val_if_fail(document != NULL, NULL);
    g_return_val_if_fail(document->priv != NULL, NULL);

    return find_item_at_point(key, SP_GROUP(document->root), p, into_groups, false, upto);
}

SPItem*
sp_document_group_at_point(SPDocument *document, unsigned int key, Geom::Point const p)
{
    g_return_val_if_fail(document != NULL, NULL);
    g_return_val_if_fail(document->priv != NULL, NULL);

    return find_group_at_point(key, SP_GROUP(document->root), p);
}


/* Resource management */

gboolean
sp_document_add_resource(SPDocument *document, gchar const *key, SPObject *object)
{
    GSList *rlist;
    GQuark q = g_quark_from_string(key);

    g_return_val_if_fail(document != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(*key != '\0', FALSE);
    g_return_val_if_fail(object != NULL, FALSE);
    g_return_val_if_fail(SP_IS_OBJECT(object), FALSE);

    if (SP_OBJECT_IS_CLONED(object))
        return FALSE;

    rlist = (GSList*)g_hash_table_lookup(document->priv->resources, key);
    g_return_val_if_fail(!g_slist_find(rlist, object), FALSE);
    rlist = g_slist_prepend(rlist, object);
    g_hash_table_insert(document->priv->resources, (gpointer) key, rlist);

    document->priv->resources_changed_signals[q].emit();

    return TRUE;
}

gboolean
sp_document_remove_resource(SPDocument *document, gchar const *key, SPObject *object)
{
    GSList *rlist;
    GQuark q = g_quark_from_string(key);

    g_return_val_if_fail(document != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(*key != '\0', FALSE);
    g_return_val_if_fail(object != NULL, FALSE);
    g_return_val_if_fail(SP_IS_OBJECT(object), FALSE);

    if (SP_OBJECT_IS_CLONED(object))
        return FALSE;

    rlist = (GSList*)g_hash_table_lookup(document->priv->resources, key);
    g_return_val_if_fail(rlist != NULL, FALSE);
    g_return_val_if_fail(g_slist_find(rlist, object), FALSE);
    rlist = g_slist_remove(rlist, object);
    g_hash_table_insert(document->priv->resources, (gpointer) key, rlist);

    document->priv->resources_changed_signals[q].emit();

    return TRUE;
}

GSList const *
sp_document_get_resource_list(SPDocument *document, gchar const *key)
{
    g_return_val_if_fail(document != NULL, NULL);
    g_return_val_if_fail(key != NULL, NULL);
    g_return_val_if_fail(*key != '\0', NULL);

    return (GSList*)g_hash_table_lookup(document->priv->resources, key);
}

sigc::connection sp_document_resources_changed_connect(SPDocument *document,
                                                       gchar const *key,
                                                       SPDocument::ResourcesChangedSignal::slot_type slot)
{
    GQuark q = g_quark_from_string(key);
    return document->priv->resources_changed_signals[q].connect(slot);
}

/* Helpers */

gboolean
sp_document_resource_list_free(gpointer /*key*/, gpointer value, gpointer /*data*/)
{
    g_slist_free((GSList *) value);
    return TRUE;
}

unsigned int
count_objects_recursive(SPObject *obj, unsigned int count)
{
    count++; // obj itself

    for (SPObject *i = sp_object_first_child(obj); i != NULL; i = SP_OBJECT_NEXT(i)) {
        count = count_objects_recursive(i, count);
    }

    return count;
}

unsigned int
objects_in_document(SPDocument *document)
{
    return count_objects_recursive(SP_DOCUMENT_ROOT(document), 0);
}

void
vacuum_document_recursive(SPObject *obj)
{
    if (SP_IS_DEFS(obj)) {
        for (SPObject *def = obj->firstChild(); def; def = SP_OBJECT_NEXT(def)) {
            /* fixme: some inkscape-internal nodes in the future might not be collectable */
            def->requestOrphanCollection();
        }
    } else {
        for (SPObject *i = sp_object_first_child(obj); i != NULL; i = SP_OBJECT_NEXT(i)) {
            vacuum_document_recursive(i);
        }
    }
}

unsigned int
vacuum_document(SPDocument *document)
{
    unsigned int start = objects_in_document(document);
    unsigned int end;
    unsigned int newend = start;

    unsigned int iterations = 0;

    do {
        end = newend;

        vacuum_document_recursive(SP_DOCUMENT_ROOT(document));
        document->collectOrphans();
        iterations++;

        newend = objects_in_document(document);

    } while (iterations < 100 && newend < end);

    return start - newend;
}

bool SPDocument::isSeeking() const {
    return priv->seeking;
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
