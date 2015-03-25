#define __SP_BOX3D_CONTEXT_C__

/*
 * 3D box drawing context
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2007      Maximilian Albert <Anhalter42@gmx.de>
 * Copyright (C) 2006      Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2000-2005 authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "display/sp-canvas.h"
#include "document.h"
#include "sp-namedview.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "desktop-handles.h"
#include "snap.h"
#include "display/curve.h"
#include "desktop.h"
#include "message-context.h"
#include "pixmaps/cursor-3dbox.xpm"
#include "box3d.h"
#include "box3d-context.h"
#include "sp-metrics.h"
#include <glibmm/i18n.h>
#include "object-edit.h"
#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "preferences.h"
#include "context-fns.h"
#include "desktop-style.h"
#include "transf_mat_3x4.h"
#include "perspective-line.h"
#include "persp3d.h"
#include "box3d-side.h"
#include "document-private.h"
#include "line-geometry.h"
#include "shape-editor.h"
#include "display/canvas-base.h"

static void sp_box3d_context_class_init(Box3DContextClass *klass);
static void sp_box3d_context_init(Box3DContext *box3d_context);
static void sp_box3d_context_dispose(GObject *object);

static void sp_box3d_context_setup(SPEventContext *ec);
static void sp_box3d_context_finish(SPEventContext *ec);

static gint sp_box3d_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_box3d_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static void sp_box3d_drag(Box3DContext &bc, guint state);
static void sp_box3d_finish(Box3DContext *bc);

static SPEventContextClass *parent_class;

GtkType sp_box3d_context_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(Box3DContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_box3d_context_class_init,
            NULL, NULL,
            sizeof(Box3DContext),
            4,
            (GInstanceInitFunc) sp_box3d_context_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "Box3DContext", &info, (GTypeFlags) 0);
    }
    return type;
}

static void sp_box3d_context_class_init(Box3DContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass *) g_type_class_peek_parent(klass);

    object_class->dispose = sp_box3d_context_dispose;

    event_context_class->setup = sp_box3d_context_setup;
    event_context_class->finish = sp_box3d_context_finish;
    event_context_class->root_handler  = sp_box3d_context_root_handler;
    event_context_class->item_handler  = sp_box3d_context_item_handler;
}

static void sp_box3d_context_init(Box3DContext *box3d_context)
{
    SPEventContext *event_context = SP_EVENT_CONTEXT(box3d_context);

    event_context->cursor_shape = SP_3DBOX_CURSOR;
    event_context->xp = 0;
    event_context->yp = 0;
    event_context->tolerance = 0;
    event_context->within_tolerance = false;
    event_context->item_to_select = NULL;

    box3d_context->item = NULL;

    box3d_context->ctrl_dragged = false;
    box3d_context->extruded = false;

    box3d_context->_vpdrag = NULL;

    new (&box3d_context->sel_changed_connection) sigc::connection();
}

static void sp_box3d_context_finish(SPEventContext *ec)
{
    Box3DContext *bc = SP_BOX3D_CONTEXT(ec);
    SPDesktop *desktop = ec->desktop;

    sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate), GDK_CURRENT_TIME);
    sp_box3d_finish(bc);
    bc->sel_changed_connection.disconnect();
//    sp_repr_remove_listener_by_data(cc->active_shape_repr, cc);

    if (((SPEventContextClass *) parent_class)->finish) {
        ((SPEventContextClass *) parent_class)->finish(ec);
    }
}


static void sp_box3d_context_dispose(GObject *object)
{
    Box3DContext *bc = SP_BOX3D_CONTEXT(object);
    SPEventContext *ec = SP_EVENT_CONTEXT(object);

    ec->enableGrDrag(false);

    delete (bc->_vpdrag);
    bc->_vpdrag = NULL;

    bc->sel_changed_connection.disconnect();
    bc->sel_changed_connection.~connection();

    delete ec->shape_editor;
    ec->shape_editor = NULL;

    /* fixme: This is necessary because we do not grab */
    if (bc->item) {
        sp_box3d_finish(bc);
    }

    if (bc->_message_context) {
        delete bc->_message_context;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new knotholder
*/
static void sp_box3d_context_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    Box3DContext *bc = SP_BOX3D_CONTEXT(data);
    SPEventContext *ec = SP_EVENT_CONTEXT(bc);

    ec->shape_editor->unset_item(SH_KNOTHOLDER);
    SPItem *item = selection->singleItem();
    ec->shape_editor->set_item(item, SH_KNOTHOLDER);

    if (selection->perspList().size() == 1) {
        // selecting a single box changes the current perspective
        ec->desktop->doc()->setCurrentPersp3D(selection->perspList().front());
    }
}

/* Create a default perspective in document defs if none is present (which can happen, among other
 * circumstances, after 'vacuum defs' or when a pre-0.46 file is opened).
 */
static void sp_box3d_context_ensure_persp_in_defs(SPDocument *document) {
    SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS(document);

    bool has_persp = false;
    for (SPObject *child = sp_object_first_child(defs); child != NULL; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_PERSP3D(child)) {
            has_persp = true;
            break;
        }
    }

    if (!has_persp) {
        document->setCurrentPersp3D(persp3d_create_xml_element (document));
    }
}

static void sp_box3d_context_setup(SPEventContext *ec)
{
    Box3DContext *bc = SP_BOX3D_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    ec->shape_editor = new ShapeEditor(ec->desktop);

    SPItem *item = sp_desktop_selection(ec->desktop)->singleItem();
    if (item) {
        ec->shape_editor->set_item(item, SH_KNOTHOLDER);
    }

    bc->sel_changed_connection.disconnect();
    bc->sel_changed_connection = sp_desktop_selection(ec->desktop)->connectChanged(
        sigc::bind(sigc::ptr_fun(&sp_box3d_context_selection_changed), (gpointer)bc)
    );

    bc->_vpdrag = new Box3D::VPDrag(sp_desktop_document (ec->desktop));
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (prefs->getBool("/tools/shapes/selcue")) {
        ec->enableSelectionCue();
    }

    if (prefs->getBool("/tools/shapes/gradientdrag")) {
        ec->enableGrDrag();
    }

    bc->_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());
}

static gint sp_box3d_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    SPDesktop *desktop = event_context->desktop;

    gint ret = FALSE;

    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1 && !event_context->space_panning) {
            Inkscape::setup_for_drag_start(desktop, event_context, event);
            ret = TRUE;
        }
        break;
        // motion and release are always on root (why?)
    default:
        break;
    }

    if (((SPEventContextClass *) parent_class)->item_handler) {
        ret = ((SPEventContextClass *) parent_class)->item_handler(event_context, item, event);
    }

    return ret;
}

static gint sp_box3d_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    static bool dragging;

    SPDesktop *desktop = event_context->desktop;
    SPDocument *document = sp_desktop_document (desktop);
    Inkscape::Selection *selection = sp_desktop_selection (desktop);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int const snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

    Box3DContext *bc = SP_BOX3D_CONTEXT(event_context);
    Persp3D *cur_persp = document->getCurrentPersp3D();

    event_context->tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    gint ret = FALSE;
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        if ( event->button.button == 1  && !event_context->space_panning) {
            Geom::Point const button_w(event->button.x,
                                       event->button.y);

            // save drag origin
            event_context->xp = (gint) button_w[Geom::X];
            event_context->yp = (gint) button_w[Geom::Y];
            event_context->within_tolerance = true;

            // remember clicked item, *not* disregarding groups (since a 3D box is a group), honoring Alt
            event_context->item_to_select = sp_event_context_find_item (desktop, button_w, event->button.state & GDK_MOD1_MASK, event->button.state & GDK_CONTROL_MASK);

            dragging = true;

            /*  */
            Geom::Point button_dt(desktop->w2d(button_w));
            bc->drag_origin = from_2geom(button_dt);
            bc->drag_ptB = from_2geom(button_dt);
            bc->drag_ptC = from_2geom(button_dt);

            // This can happen after saving when the last remaining perspective was purged and must be recreated.
            if (!cur_persp) {
                sp_box3d_context_ensure_persp_in_defs(document);
                cur_persp = document->getCurrentPersp3D();
            }

            /* Projective preimages of clicked point under current perspective */
            bc->drag_origin_proj = cur_persp->perspective_impl->tmat.preimage (from_2geom(button_dt), 0, Proj::Z);
            bc->drag_ptB_proj = bc->drag_origin_proj;
            bc->drag_ptC_proj = bc->drag_origin_proj;
            bc->drag_ptC_proj.normalize();
            bc->drag_ptC_proj[Proj::Z] = 0.25;

            /* Snap center */
            SnapManager &m = desktop->namedview->snap_manager;
            m.setup(desktop, true, bc->item);
            m.freeSnapReturnByRef(button_dt, Inkscape::SNAPSOURCE_NODE_HANDLE);
            bc->center = from_2geom(button_dt);

            sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                ( GDK_KEY_PRESS_MASK |
                                  GDK_BUTTON_RELEASE_MASK       |
                                  GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK       |
                                  GDK_BUTTON_PRESS_MASK ),
                                NULL, event->button.time);
            ret = TRUE;
        }
        break;
    case GDK_MOTION_NOTIFY:
        if ( dragging
             && ( event->motion.state & GDK_BUTTON1_MASK )  && !event_context->space_panning)
        {
            if ( event_context->within_tolerance
                 && ( abs( (gint) event->motion.x - event_context->xp ) < event_context->tolerance )
                 && ( abs( (gint) event->motion.y - event_context->yp ) < event_context->tolerance ) ) {
                break; // do not drag if we're within tolerance from origin
            }
            // Once the user has moved farther than tolerance from the original location
            // (indicating they intend to draw, not click), then always process the
            // motion notify coordinates as given (no snapping back to origin)
            event_context->within_tolerance = false;

            Geom::Point const motion_w(event->motion.x,
                                       event->motion.y);
            Geom::Point motion_dt(desktop->w2d(motion_w));

            SnapManager &m = desktop->namedview->snap_manager;
            m.setup(desktop, true, bc->item);
            m.freeSnapReturnByRef(motion_dt, Inkscape::SNAPSOURCE_NODE_HANDLE);

            bc->ctrl_dragged  = event->motion.state & GDK_CONTROL_MASK;

            if (event->motion.state & GDK_SHIFT_MASK && !bc->extruded && bc->item) {
                // once shift is pressed, set bc->extruded
                bc->extruded = true;
            }

            if (!bc->extruded) {
                bc->drag_ptB = from_2geom(motion_dt);
                bc->drag_ptC = from_2geom(motion_dt);

                bc->drag_ptB_proj = cur_persp->perspective_impl->tmat.preimage (from_2geom(motion_dt), 0, Proj::Z);
                bc->drag_ptC_proj = bc->drag_ptB_proj;
                bc->drag_ptC_proj.normalize();
                bc->drag_ptC_proj[Proj::Z] = 0.25;
            } else {
                // Without Ctrl, motion of the extruded corner is constrained to the
                // perspective line from drag_ptB to vanishing point Y.
                if (!bc->ctrl_dragged) {
                    /* snapping */
                    Box3D::PerspectiveLine pline (bc->drag_ptB, Proj::Z, document->getCurrentPersp3D());
                    bc->drag_ptC = pline.closest_to (from_2geom(motion_dt));

                    bc->drag_ptB_proj.normalize();
                    bc->drag_ptC_proj = cur_persp->perspective_impl->tmat.preimage (bc->drag_ptC, bc->drag_ptB_proj[Proj::X], Proj::X);
                } else {
                    bc->drag_ptC = from_2geom(motion_dt);

                    bc->drag_ptB_proj.normalize();
                    bc->drag_ptC_proj = cur_persp->perspective_impl->tmat.preimage (from_2geom(motion_dt), bc->drag_ptB_proj[Proj::X], Proj::X);
                }
                m.freeSnapReturnByRef(bc->drag_ptC, Inkscape::SNAPSOURCE_NODE_HANDLE);
            }

            sp_box3d_drag(*bc, event->motion.state);

            ret = TRUE;
        } else if (!sp_event_context_knot_mouseover(bc)) {
            SnapManager &m = desktop->namedview->snap_manager;
            m.setup(desktop);

            Geom::Point const motion_w(event->motion.x, event->motion.y);
            Geom::Point motion_dt(desktop->w2d(motion_w));
            m.preSnap(Inkscape::SnapCandidatePoint(motion_dt, Inkscape::SNAPSOURCE_NODE_HANDLE));
        }
        break;
    case GDK_BUTTON_RELEASE:
        event_context->xp = event_context->yp = 0;
        if ( event->button.button == 1  && !event_context->space_panning) {
            dragging = false;
            sp_event_context_discard_delayed_snap_event(event_context);

            if (!event_context->within_tolerance) {
                // we've been dragging, finish the box
                sp_box3d_finish(bc);
            } else if (event_context->item_to_select) {
                // no dragging, select clicked item if any
                if (event->button.state & GDK_SHIFT_MASK) {
                    selection->toggle(event_context->item_to_select);
                } else {
                    selection->set(event_context->item_to_select);
                }
            } else {
                // click in an empty space
                selection->clear();
            }

            event_context->item_to_select = NULL;
            ret = TRUE;
            sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                  event->button.time);
        }
        break;
    case GDK_KEY_PRESS:
        switch (get_group0_keyval (&event->key)) {
        case GDK_Up:
        case GDK_Down:
        case GDK_KP_Up:
        case GDK_KP_Down:
            // prevent the zoom field from activation
            if (!MOD__CTRL_ONLY)
                ret = TRUE;
            break;

        case GDK_bracketright:
            persp3d_rotate_VP (document->getCurrentPersp3D(), Proj::X, -180/snaps, MOD__ALT);
            sp_document_done(document, SP_VERB_CONTEXT_3DBOX,
                             _("Change perspective (angle of PLs)"));
            ret = true;
            break;

        case GDK_bracketleft:
            persp3d_rotate_VP (document->getCurrentPersp3D(), Proj::X, 180/snaps, MOD__ALT);
            sp_document_done(document, SP_VERB_CONTEXT_3DBOX,
                             _("Change perspective (angle of PLs)"));
            ret = true;
            break;

        case GDK_parenright:
            persp3d_rotate_VP (document->getCurrentPersp3D(), Proj::Y, -180/snaps, MOD__ALT);
            sp_document_done(document, SP_VERB_CONTEXT_3DBOX,
                             _("Change perspective (angle of PLs)"));
            ret = true;
            break;

        case GDK_parenleft:
            persp3d_rotate_VP (document->getCurrentPersp3D(), Proj::Y, 180/snaps, MOD__ALT);
            sp_document_done(document, SP_VERB_CONTEXT_3DBOX,
                             _("Change perspective (angle of PLs)"));
            ret = true;
            break;

        case GDK_braceright:
            persp3d_rotate_VP (document->getCurrentPersp3D(), Proj::Z, -180/snaps, MOD__ALT);
            sp_document_done(document, SP_VERB_CONTEXT_3DBOX,
                             _("Change perspective (angle of PLs)"));
            ret = true;
            break;

        case GDK_braceleft:
            persp3d_rotate_VP (document->getCurrentPersp3D(), Proj::Z, 180/snaps, MOD__ALT);
            sp_document_done(document, SP_VERB_CONTEXT_3DBOX,
                             _("Change perspective (angle of PLs)"));
            ret = true;
            break;

        /* TODO: what is this???
        case GDK_O:
            if (MOD__CTRL && MOD__SHIFT) {
                Box3D::create_canvas_point(persp3d_get_VP(document()->getCurrentPersp3D(), Proj::W).affine(),
                                           6, 0xff00ff00);
            }
            ret = true;
            break;
        */

        case GDK_g:
        case GDK_G:
            if (MOD__SHIFT_ONLY) {
                sp_selection_to_guides(desktop);
                ret = true;
            }
            break;

        case GDK_p:
        case GDK_P:
            if (MOD__SHIFT_ONLY) {
                if (document->getCurrentPersp3D()) {
                    persp3d_print_debugging_info (document->getCurrentPersp3D());
                }
                ret = true;
            }
            break;

        case GDK_x:
        case GDK_X:
            if (MOD__ALT_ONLY) {
                desktop->setToolboxFocusTo ("altx-box3d");
                ret = TRUE;
            }
            if (MOD__SHIFT_ONLY) {
                persp3d_toggle_VPs(selection->perspList(), Proj::X);
                bc->_vpdrag->updateLines(); // FIXME: Shouldn't this be done automatically?
                ret = true;
            }
            break;

        case GDK_y:
        case GDK_Y:
            if (MOD__SHIFT_ONLY) {
                persp3d_toggle_VPs(selection->perspList(), Proj::Y);
                bc->_vpdrag->updateLines(); // FIXME: Shouldn't this be done automatically?
                ret = true;
            }
            break;

        case GDK_z:
        case GDK_Z:
            if (MOD__SHIFT_ONLY) {
                persp3d_toggle_VPs(selection->perspList(), Proj::Z);
                bc->_vpdrag->updateLines(); // FIXME: Shouldn't this be done automatically?
                ret = true;
            }
            break;

        case GDK_Escape:
            sp_desktop_selection(desktop)->clear();
            //TODO: make dragging escapable by Esc
            break;

        case GDK_space:
            if (dragging) {
                sp_canvas_item_ungrab(SP_CANVAS_ITEM(desktop->acetate),
                                      event->button.time);
                dragging = false;
                sp_event_context_discard_delayed_snap_event(event_context);
                if (!event_context->within_tolerance) {
                    // we've been dragging, finish the box
                    sp_box3d_finish(bc);
                }
                // do not return true, so that space would work switching to selector
            }
            break;

        default:
            break;
        }
        break;
    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->root_handler) {
            ret = ((SPEventContextClass *) parent_class)->root_handler(event_context, event);
        }
    }

    return ret;
}

static void sp_box3d_drag(Box3DContext &bc, guint /*state*/)
{
    SPDesktop *desktop = SP_EVENT_CONTEXT(&bc)->desktop;

    if (!bc.item) {

        if (Inkscape::have_viable_layer(desktop, bc._message_context) == false) {
            return;
        }

        /* Create object */
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_EVENT_CONTEXT_DOCUMENT(&bc));
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:g");
        repr->setAttribute("sodipodi:type", "inkscape:box3d");

        /* Set style */
        sp_desktop_apply_style_tool (desktop, repr, "/tools/shapes/3dbox", false);

        bc.item = (SPItem *) desktop->currentLayer()->appendChildRepr(repr);
        Inkscape::GC::release(repr);
        Inkscape::XML::Node *repr_side;
        // TODO: Incorporate this in box3d-side.cpp!
        for (int i = 0; i < 6; ++i) {
            repr_side = xml_doc->createElement("svg:path");
            repr_side->setAttribute("sodipodi:type", "inkscape:box3dside");
            repr->addChild(repr_side, NULL);

            Box3DSide *side = SP_BOX3D_SIDE(inkscape_active_document()->getObjectByRepr (repr_side));

            guint desc = Box3D::int_to_face(i);

            Box3D::Axis plane = (Box3D::Axis) (desc & 0x7);
            plane = (Box3D::is_plane(plane) ? plane : Box3D::orth_plane_or_axis(plane));
            side->dir1 = Box3D::extract_first_axis_direction(plane);
            side->dir2 = Box3D::extract_second_axis_direction(plane);
            side->front_or_rear = (Box3D::FrontOrRear) (desc & 0x8);

            /* Set style */
            box3d_side_apply_style(side);

            SP_OBJECT(side)->updateRepr(); // calls box3d_side_write() and updates, e.g., the axes string description
        }

        box3d_set_z_orders(SP_BOX3D(bc.item));
        bc.item->updateRepr();

        // TODO: It would be nice to show the VPs during dragging, but since there is no selection
        //       at this point (only after finishing the box), we must do this "manually"
        /* bc._vpdrag->updateDraggers(); */

        desktop->canvas->force_full_redraw_after_interruptions(5);
    }

    g_assert(bc.item);

    SPBox3D *box = SP_BOX3D(bc.item);

    box->orig_corner0 = bc.drag_origin_proj;
    box->orig_corner7 = bc.drag_ptC_proj;

    box3d_check_for_swapped_coords(box);

    /* we need to call this from here (instead of from box3d_position_set(), for example)
       because z-order setting must not interfere with display updates during undo/redo */
    box3d_set_z_orders (box);

    box3d_position_set(box);

    // status text
    bc._message_context->setF(Inkscape::NORMAL_MESSAGE, "%s", _("<b>3D Box</b>; with <b>Shift</b> to extrude along the Z axis"));
}

static void sp_box3d_finish(Box3DContext *bc)
{
    bc->_message_context->clear();
    bc->ctrl_dragged = false;
    bc->extruded = false;

    if ( bc->item != NULL ) {
        SPDesktop * desktop = SP_EVENT_CONTEXT_DESKTOP(bc);
        SPDocument *doc = sp_desktop_document(desktop);
        if (!doc || !doc->getCurrentPersp3D())
            return;

        SPBox3D *box = SP_BOX3D(bc->item);

        box->orig_corner0 = bc->drag_origin_proj;
        box->orig_corner7 = bc->drag_ptC_proj;

        box->updateRepr();

        box3d_relabel_corners(box);

        desktop->canvas->end_forced_full_redraws();

        sp_desktop_selection(desktop)->set(bc->item);
        sp_document_done(sp_desktop_document(desktop), SP_VERB_CONTEXT_3DBOX,
                         _("Create 3D box"));

        bc->item = NULL;
    }
}

void sp_box3d_context_update_lines(SPEventContext *ec) {
    /*  update perspective lines if we are in the 3D box tool (so that infinite ones are shown correctly) */
    if (SP_IS_BOX3D_CONTEXT (ec)) {
        Box3DContext *bc = SP_BOX3D_CONTEXT (ec);
        bc->_vpdrag->updateLines();
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
