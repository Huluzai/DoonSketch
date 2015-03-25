#define __PERSP3D_C__

/*
 * Class modelling a 3D perspective as an SPObject
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "persp3d.h"
#include "perspective-line.h"
#include "attributes.h"
#include "document-private.h"
#include "vanishing-point.h"
#include "box3d-context.h"
#include "box3d.h"
#include "xml/document.h"
#include "xml/node-event-vector.h"
#include "desktop-handles.h"
#include <glibmm/i18n.h>

static void persp3d_class_init(Persp3DClass *klass);
static void persp3d_init(Persp3D *persp);

static void persp3d_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void persp3d_release(SPObject *object);
static void persp3d_set(SPObject *object, unsigned key, gchar const *value);
static void persp3d_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *persp3d_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static void persp3d_on_repr_attr_changed (Inkscape::XML::Node * repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive, void * data);

static void persp3d_update_with_point (Persp3DImpl *persp_impl, Proj::Axis const axis, Proj::Pt2 const &new_image);
static gchar * persp3d_pt_to_str (Persp3DImpl *persp_impl, Proj::Axis const axis);

static SPObjectClass *persp3d_parent_class;

static int global_counter = 0;

/* Constructor/destructor for the internal class */

Persp3DImpl::Persp3DImpl() {
    tmat = Proj::TransfMat3x4 ();
    document = NULL;

    my_counter = global_counter++;
}

/**
 * Registers Persp3d class and returns its type.
 */
GType
persp3d_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(Persp3DClass),
            NULL, NULL,
            (GClassInitFunc) persp3d_class_init,
            NULL, NULL,
            sizeof(Persp3D),
            16,
            (GInstanceInitFunc) persp3d_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "Persp3D", &info, (GTypeFlags)0);
    }
    return type;
}

static Inkscape::XML::NodeEventVector const persp3d_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    persp3d_on_repr_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};

/**
 * Callback to initialize Persp3D vtable.
 */
static void persp3d_class_init(Persp3DClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    persp3d_parent_class = (SPObjectClass *) g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build = persp3d_build;
    sp_object_class->release = persp3d_release;
    sp_object_class->set = persp3d_set;
    sp_object_class->update = persp3d_update;
    sp_object_class->write = persp3d_write;
}

/**
 * Callback to initialize Persp3D object.
 */
static void
persp3d_init(Persp3D *persp)
{
    persp->perspective_impl = new Persp3DImpl();
}

/**
 * Virtual build: set persp3d attributes from its associated XML node.
 */
static void persp3d_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) persp3d_parent_class)->build)
        (* ((SPObjectClass *) persp3d_parent_class)->build)(object, document, repr);

    /* calls sp_object_set for the respective attributes */
    // The transformation matrix is updated according to the values we read for the VPs
    sp_object_read_attr(object, "inkscape:vp_x");
    sp_object_read_attr(object, "inkscape:vp_y");
    sp_object_read_attr(object, "inkscape:vp_z");
    sp_object_read_attr(object, "inkscape:persp3d-origin");

    if (repr) {
        repr->addListener (&persp3d_repr_events, object);
    }
}

/**
 * Virtual release of Persp3D members before destruction.
 */
static void persp3d_release(SPObject *object) {
    Persp3D *persp = SP_PERSP3D(object);
    delete persp->perspective_impl;
    SP_OBJECT_REPR(object)->removeListenerByData(object);
}


/**
 * Virtual set: set attribute to value.
 */
// FIXME: Currently we only read the finite positions of vanishing points;
//        should we move VPs into their own repr (as it's done for SPStop, e.g.)?
static void
persp3d_set(SPObject *object, unsigned key, gchar const *value)
{
    Persp3DImpl *persp_impl = SP_PERSP3D(object)->perspective_impl;

    switch (key) {
        case SP_ATTR_INKSCAPE_PERSP3D_VP_X: {
            if (value) {
                Proj::Pt2 new_image (value);
                persp3d_update_with_point (persp_impl, Proj::X, new_image);
            }
            break;
        }
        case SP_ATTR_INKSCAPE_PERSP3D_VP_Y: {
            if (value) {
                Proj::Pt2 new_image (value);
                persp3d_update_with_point (persp_impl, Proj::Y, new_image);
                break;
            }
        }
        case SP_ATTR_INKSCAPE_PERSP3D_VP_Z: {
            if (value) {
                Proj::Pt2 new_image (value);
                persp3d_update_with_point (persp_impl, Proj::Z, new_image);
                break;
            }
        }
        case SP_ATTR_INKSCAPE_PERSP3D_ORIGIN: {
            if (value) {
                Proj::Pt2 new_image (value);
                persp3d_update_with_point (persp_impl, Proj::W, new_image);
                break;
            }
        }
        default: {
            if (((SPObjectClass *) persp3d_parent_class)->set)
                (* ((SPObjectClass *) persp3d_parent_class)->set)(object, key, value);
            break;
        }
    }

    // FIXME: Is this the right place for resetting the draggers?
    SPEventContext *ec = inkscape_active_event_context();
    if (SP_IS_BOX3D_CONTEXT(ec)) {
        Box3DContext *bc = SP_BOX3D_CONTEXT(ec);
        bc->_vpdrag->updateDraggers();
        bc->_vpdrag->updateLines();
        bc->_vpdrag->updateBoxHandles();
        bc->_vpdrag->updateBoxReprs();
    }
}

static void
persp3d_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* TODO: Should we update anything here? */

    }

    if (((SPObjectClass *) persp3d_parent_class)->update)
        ((SPObjectClass *) persp3d_parent_class)->update(object, ctx, flags);
}

Persp3D *
persp3d_create_xml_element (SPDocument *document, Persp3DImpl *dup) {// if dup is given, copy the attributes over
    SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS(document);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);
    Inkscape::XML::Node *repr;

    /* if no perspective is given, create a default one */
    repr = xml_doc->createElement("inkscape:perspective");
    repr->setAttribute("sodipodi:type", "inkscape:persp3d");

    Proj::Pt2 proj_vp_x = Proj::Pt2 (0.0, sp_document_height(document)/2, 1.0);
    Proj::Pt2 proj_vp_y = Proj::Pt2 (0.0, 1000.0, 0.0);
    Proj::Pt2 proj_vp_z = Proj::Pt2 (sp_document_width(document), sp_document_height(document)/2, 1.0);
    Proj::Pt2 proj_origin = Proj::Pt2 (sp_document_width(document)/2, sp_document_height(document)/3, 1.0);

    if (dup) {
        proj_vp_x = dup->tmat.column (Proj::X);
        proj_vp_y = dup->tmat.column (Proj::Y);
        proj_vp_z = dup->tmat.column (Proj::Z);
        proj_origin = dup->tmat.column (Proj::W);
    }

    gchar *str = NULL;
    str = proj_vp_x.coord_string();
    repr->setAttribute("inkscape:vp_x", str);
    g_free (str);
    str = proj_vp_y.coord_string();
    repr->setAttribute("inkscape:vp_y", str);
    g_free (str);
    str = proj_vp_z.coord_string();
    repr->setAttribute("inkscape:vp_z", str);
    g_free (str);
    str = proj_origin.coord_string();
    repr->setAttribute("inkscape:persp3d-origin", str);
    g_free (str);

    /* Append the new persp3d to defs */
    SP_OBJECT_REPR(defs)->addChild(repr, NULL);
    Inkscape::GC::release(repr);

    return (Persp3D *) sp_object_get_child_by_repr (SP_OBJECT(defs), repr);
}

Persp3D *
persp3d_document_first_persp (SPDocument *document) {
    SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS(document);
    Inkscape::XML::Node *repr;
    for (SPObject *child = sp_object_first_child(defs); child != NULL; child = SP_OBJECT_NEXT(child) ) {
        repr = SP_OBJECT_REPR(child);
        if (SP_IS_PERSP3D(child)) {
            return SP_PERSP3D(child);
        }
    }
    return NULL;
}

/**
 * Virtual write: write object attributes to repr.
 */
static Inkscape::XML::Node *
persp3d_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    Persp3DImpl *persp_impl = SP_PERSP3D(object)->perspective_impl;

    if ((flags & SP_OBJECT_WRITE_BUILD & SP_OBJECT_WRITE_EXT) && !repr) {
        // this is where we end up when saving as plain SVG (also in other circumstances?);
        // hence we don't set the sodipodi:type attribute
        repr = xml_doc->createElement("inkscape:perspective");
    }

    if (flags & SP_OBJECT_WRITE_EXT) {
        gchar *str = NULL; // FIXME: Should this be freed each time we set an attribute or only in the end or at all?
        str = persp3d_pt_to_str (persp_impl, Proj::X);
        repr->setAttribute("inkscape:vp_x", str);

        str = persp3d_pt_to_str (persp_impl, Proj::Y);
        repr->setAttribute("inkscape:vp_y", str);

        str = persp3d_pt_to_str (persp_impl, Proj::Z);
        repr->setAttribute("inkscape:vp_z", str);

        str = persp3d_pt_to_str (persp_impl, Proj::W);
        repr->setAttribute("inkscape:persp3d-origin", str);
    }

    if (((SPObjectClass *) persp3d_parent_class)->write)
        (* ((SPObjectClass *) persp3d_parent_class)->write)(object, xml_doc, repr, flags);

    return repr;
}

/* convenience wrapper around persp3d_get_finite_dir() and persp3d_get_infinite_dir() */
Geom::Point persp3d_get_PL_dir_from_pt (Persp3D *persp, Geom::Point const &pt, Proj::Axis axis) {
    if (persp3d_VP_is_finite(persp->perspective_impl, axis)) {
        return persp3d_get_finite_dir(persp, pt, axis);
    } else {
        return persp3d_get_infinite_dir(persp, axis);
    }
}

Geom::Point
persp3d_get_finite_dir (Persp3D *persp, Geom::Point const &pt, Proj::Axis axis) {
    Box3D::PerspectiveLine pl(pt, axis, persp);
    return pl.direction();
}

Geom::Point
persp3d_get_infinite_dir (Persp3D *persp, Proj::Axis axis) {
    Proj::Pt2 vp(persp3d_get_VP(persp, axis));
    if (vp[2] != 0.0) {
        g_print ("VP should be infinite but is (%f : %f : %f)\n", vp[0], vp[1], vp[2]);
        g_return_val_if_fail(vp[2] != 0.0, Geom::Point(0.0, 0.0));
    }
    return Geom::Point(vp[0], vp[1]);
}

double
persp3d_get_infinite_angle (Persp3D *persp, Proj::Axis axis) {
    return persp->perspective_impl->tmat.get_infinite_angle(axis);
}

bool
persp3d_VP_is_finite (Persp3DImpl *persp_impl, Proj::Axis axis) {
    return persp_impl->tmat.has_finite_image(axis);
}

void
persp3d_toggle_VP (Persp3D *persp, Proj::Axis axis, bool set_undo) {
    persp->perspective_impl->tmat.toggle_finite(axis);
    // FIXME: Remove this repr update and rely on vp_drag_sel_modified() to do this for us
    //        On the other hand, vp_drag_sel_modified() would update all boxes;
    //        here we can confine ourselves to the boxes of this particular perspective.
    persp3d_update_box_reprs (persp);
    SP_OBJECT(persp)->updateRepr(SP_OBJECT_WRITE_EXT);
    if (set_undo) {
        sp_document_done(sp_desktop_document(inkscape_active_desktop()), SP_VERB_CONTEXT_3DBOX,
                         _("Toggle vanishing point"));
    }
}

/* toggle VPs for the same axis in all perspectives of a given list */
void
persp3d_toggle_VPs (std::list<Persp3D *> p, Proj::Axis axis) {
    for (std::list<Persp3D *>::iterator i = p.begin(); i != p.end(); ++i) {
        persp3d_toggle_VP((*i), axis, false);
    }
    sp_document_done(sp_desktop_document(inkscape_active_desktop()), SP_VERB_CONTEXT_3DBOX,
                     _("Toggle multiple vanishing points"));
}

void
persp3d_set_VP_state (Persp3D *persp, Proj::Axis axis, Proj::VPState state) {
    if (persp3d_VP_is_finite(persp->perspective_impl, axis) != (state == Proj::VP_FINITE)) {
        persp3d_toggle_VP(persp, axis);
    }
}

void
persp3d_rotate_VP (Persp3D *persp, Proj::Axis axis, double angle, bool alt_pressed) { // angle is in degrees
    // FIXME: Most of this functionality should be moved to trans_mat_3x4.(h|cpp)
    if (persp->perspective_impl->tmat.has_finite_image(axis)) {
        // don't rotate anything for finite VPs
        return;
    }
    Proj::Pt2 v_dir_proj (persp->perspective_impl->tmat.column(axis));
    Geom::Point v_dir (v_dir_proj[0], v_dir_proj[1]);
    double a = Geom::atan2 (v_dir) * 180/M_PI;
    a += alt_pressed ? 0.5 * ((angle > 0 ) - (angle < 0)) : angle; // the r.h.s. yields +/-0.5 or angle
    persp->perspective_impl->tmat.set_infinite_direction (axis, a);

    persp3d_update_box_reprs (persp);
    SP_OBJECT(persp)->updateRepr(SP_OBJECT_WRITE_EXT);
}

void
persp3d_update_with_point (Persp3DImpl *persp_impl, Proj::Axis const axis, Proj::Pt2 const &new_image) {
    persp_impl->tmat.set_image_pt (axis, new_image);
}

void
persp3d_apply_affine_transformation (Persp3D *persp, Geom::Matrix const &xform) {
    persp->perspective_impl->tmat *= xform;
    persp3d_update_box_reprs(persp);
    SP_OBJECT(persp)->updateRepr(SP_OBJECT_WRITE_EXT);
}

gchar *
persp3d_pt_to_str (Persp3DImpl *persp_impl, Proj::Axis const axis)
{
    return persp_impl->tmat.pt_to_str(axis);
}

void
persp3d_add_box (Persp3D *persp, SPBox3D *box) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    if (!box) {
        return;
    }
    if (std::find (persp_impl->boxes.begin(), persp_impl->boxes.end(), box) != persp_impl->boxes.end()) {
        return;
    }
    persp_impl->boxes.push_back(box);
}

void
persp3d_remove_box (Persp3D *persp, SPBox3D *box) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    std::vector<SPBox3D *>::iterator i = std::find (persp_impl->boxes.begin(), persp_impl->boxes.end(), box);
    if (i != persp_impl->boxes.end())
        persp_impl->boxes.erase(i);
}

bool
persp3d_has_box (Persp3D *persp, SPBox3D *box) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    // FIXME: For some reason, std::find() does not seem to compare pointers "correctly" (or do we need to
    //        provide a proper comparison function?), so we manually traverse the list.
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        if ((*i) == box) {
            return true;
        }
    }
    return false;
}

void
persp3d_update_box_displays (Persp3D *persp) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    if (persp_impl->boxes.empty())
        return;
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        box3d_position_set(*i);
    }
}

void
persp3d_update_box_reprs (Persp3D *persp) {
    if (!persp) {
        // Hmm, is it an error if this happens?
        return;
    }
    Persp3DImpl *persp_impl = persp->perspective_impl;

    if (persp_impl->boxes.empty())
        return;
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        SP_OBJECT(*i)->updateRepr(SP_OBJECT_WRITE_EXT);
        box3d_set_z_orders(*i);
    }
}

void
persp3d_update_z_orders (Persp3D *persp) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    if (persp_impl->boxes.empty())
        return;
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        box3d_set_z_orders(*i);
    }
}

// FIXME: For some reason we seem to require a vector instead of a list in Persp3D, but in vp_knot_moved_handler()
//        we need a list of boxes. If we can store a list in Persp3D right from the start, this function becomes
//        obsolete. We should do this.
std::list<SPBox3D *>
persp3d_list_of_boxes(Persp3D *persp) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    std::list<SPBox3D *> bx_lst;
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        bx_lst.push_back(*i);
    }
    return bx_lst;
}

bool
persp3d_perspectives_coincide(const Persp3D *lhs, const Persp3D *rhs)
{
    return lhs->perspective_impl->tmat == rhs->perspective_impl->tmat;
}

void
persp3d_absorb(Persp3D *persp1, Persp3D *persp2) {
    /* double check if we are called in sane situations */
    g_return_if_fail (persp3d_perspectives_coincide(persp1, persp2) && persp1 != persp2);

    std::vector<SPBox3D *>::iterator boxes;

    // Note: We first need to copy the boxes of persp2 into a separate list;
    //       otherwise the loop below gets confused when perspectives are reattached.
    std::list<SPBox3D *> boxes_of_persp2 = persp3d_list_of_boxes(persp2);

    for (std::list<SPBox3D *>::iterator i = boxes_of_persp2.begin(); i != boxes_of_persp2.end(); ++i) {
        box3d_switch_perspectives((*i), persp2, persp1, true);
        SP_OBJECT(*i)->updateRepr(SP_OBJECT_WRITE_EXT); // so that undo/redo can do its job properly
    }
}

static void
persp3d_on_repr_attr_changed ( Inkscape::XML::Node * /*repr*/,
                               const gchar */*key*/,
                               const gchar */*oldval*/,
                               const gchar */*newval*/,
                               bool /*is_interactive*/,
                               void * data )
{
    if (!data)
        return;

    Persp3D *persp = (Persp3D*) data;
    persp3d_update_box_displays (persp);
}

/* checks whether all boxes linked to this perspective are currently selected */
bool
persp3d_has_all_boxes_in_selection (Persp3D *persp, Inkscape::Selection *selection) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    std::list<SPBox3D *> selboxes = selection->box3DList();

    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        if (std::find(selboxes.begin(), selboxes.end(), *i) == selboxes.end()) {
            // we have an unselected box in the perspective
            return false;
        }
    }
    return true;
}

/* some debugging stuff follows */

void
persp3d_print_debugging_info (Persp3D *persp) {
    Persp3DImpl *persp_impl = persp->perspective_impl;
    g_print ("=== Info for Persp3D %d ===\n", persp_impl->my_counter);
    gchar * cstr;
    for (int i = 0; i < 4; ++i) {
        cstr = persp3d_get_VP(persp, Proj::axes[i]).coord_string();
        g_print ("  VP %s:   %s\n", Proj::string_from_axis(Proj::axes[i]), cstr);
        g_free(cstr);
    }
    cstr = persp3d_get_VP(persp, Proj::W).coord_string();
    g_print ("  Origin: %s\n", cstr);
    g_free(cstr);

    g_print ("  Boxes: ");
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        g_print ("%d (%d)  ", (*i)->my_counter, box3d_get_perspective(*i)->perspective_impl->my_counter);
    }
    g_print ("\n");
    g_print ("========================\n");
}

void
persp3d_print_debugging_info_all(SPDocument *document) {
    SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS(document);
    Inkscape::XML::Node *repr;
    for (SPObject *child = sp_object_first_child(defs); child != NULL; child = SP_OBJECT_NEXT(child) ) {
        repr = SP_OBJECT_REPR(child);
        if (SP_IS_PERSP3D(child)) {
            persp3d_print_debugging_info(SP_PERSP3D(child));
        }
    }
    persp3d_print_all_selected();
}

void
persp3d_print_all_selected() {
    g_print ("\n======================================\n");
    g_print ("Selected perspectives and their boxes:\n");

    std::list<Persp3D *> sel_persps = sp_desktop_selection(inkscape_active_desktop())->perspList();

    for (std::list<Persp3D *>::iterator j = sel_persps.begin(); j != sel_persps.end(); ++j) {
        Persp3D *persp = SP_PERSP3D(*j);
        Persp3DImpl *persp_impl = persp->perspective_impl;
        g_print ("  %s (%d):  ", SP_OBJECT_REPR(persp)->attribute("id"), persp->perspective_impl->my_counter);
        for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin();
             i != persp_impl->boxes.end(); ++i) {
            g_print ("%d ", (*i)->my_counter);
        }
        g_print ("\n");
    }
    g_print ("======================================\n\n");
 }

void print_current_persp3d(gchar *func_name, Persp3D *persp) {
    g_print ("%s: current_persp3d is now %s\n",
             func_name,
             persp ? SP_OBJECT_REPR(persp)->attribute("id") : "NULL");
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
