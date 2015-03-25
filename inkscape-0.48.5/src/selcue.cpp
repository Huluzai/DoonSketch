#define __SELCUE_C__

/*
 * Helper object for showing selected items
 *
 * Authors:
 *   bulia byak <bulia@users.sf.net>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>

#include "desktop-handles.h"
#include "selection.h"
#include "display/sp-canvas-util.h"
#include "display/sodipodi-ctrl.h"
#include "display/sodipodi-ctrlrect.h"
#include "libnrtype/Layout-TNG.h"
#include "text-editing.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "preferences.h"
#include "selcue.h"

Inkscape::SelCue::SelCue(SPDesktop *desktop)
    : _desktop(desktop)
{
    _selection = sp_desktop_selection(_desktop);

    _sel_changed_connection = _selection->connectChanged(
        sigc::hide(sigc::mem_fun(*this, &Inkscape::SelCue::_newItemBboxes))
        );

    _sel_modified_connection = _selection->connectModified(
        sigc::hide(sigc::hide(sigc::mem_fun(*this, &Inkscape::SelCue::_updateItemBboxes)))
        );

    _updateItemBboxes();
}

Inkscape::SelCue::~SelCue()
{
    _sel_changed_connection.disconnect();
    _sel_modified_connection.disconnect();

    for (std::vector<SPCanvasItem*>::iterator i = _item_bboxes.begin(); i != _item_bboxes.end(); i++) {
        gtk_object_destroy(*i);
    }
    _item_bboxes.clear();

    for (std::vector<SPCanvasItem*>::iterator i = _text_baselines.begin(); i != _text_baselines.end(); i++) {
        gtk_object_destroy(*i);
    }
    _text_baselines.clear();
}

void Inkscape::SelCue::_updateItemBboxes()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gint mode = prefs->getInt("/options/selcue/value", MARK);
    if (mode == NONE) {
        return;
    }

    g_return_if_fail(_selection != NULL);

    int prefs_bbox = prefs->getBool("/tools/bounding_box");
    SPItem::BBoxType bbox_type = !prefs_bbox ? 
        SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;

    GSList const *items = _selection->itemList();
    if (_item_bboxes.size() != g_slist_length((GSList *) items)) {
        _newItemBboxes();
        return;
    }

    int bcount = 0;
    for (GSList const *l = _selection->itemList(); l != NULL; l = l->next) {
        SPItem *item = (SPItem *) l->data;
        SPCanvasItem* box = _item_bboxes[bcount ++];

        if (box) {
            Geom::OptRect const b = sp_item_bbox_desktop(item, bbox_type);

            if (b) {
                sp_canvas_item_show(box);
                if (mode == MARK) {
                    SP_CTRL(box)->moveto(Geom::Point(b->min()[Geom::X], b->max()[Geom::Y]));
                } else if (mode == BBOX) {
                    SP_CTRLRECT(box)->setRectangle(*b);
                }
            } else { // no bbox
                sp_canvas_item_hide(box);
            }
        }
    }

    _newTextBaselines();
}


void Inkscape::SelCue::_newItemBboxes()
{
    for (std::vector<SPCanvasItem*>::iterator i = _item_bboxes.begin(); i != _item_bboxes.end(); i++) {
        gtk_object_destroy(*i);
    }
    _item_bboxes.clear();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gint mode = prefs->getInt("/options/selcue/value", MARK);
    if (mode == NONE) {
        return;
    }

    g_return_if_fail(_selection != NULL);

    int prefs_bbox = prefs->getBool("/tools/bounding_box");
    SPItem::BBoxType bbox_type = !prefs_bbox ? 
        SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;
    
    for (GSList const *l = _selection->itemList(); l != NULL; l = l->next) {
        SPItem *item = (SPItem *) l->data;

        Geom::OptRect const b = sp_item_bbox_desktop(item, bbox_type);

        SPCanvasItem* box = NULL;

        if (b) {
            if (mode == MARK) {
                box = sp_canvas_item_new(sp_desktop_controls(_desktop),
                                         SP_TYPE_CTRL,
                                         "mode", SP_CTRL_MODE_XOR,
                                         "shape", SP_CTRL_SHAPE_DIAMOND,
                                         "size", 5.0,
                                         "filled", TRUE,
                                         "fill_color", 0x000000ff,
                                         "stroked", FALSE,
                                         "stroke_color", 0x000000ff,
                                         NULL);
                sp_canvas_item_show(box);
                SP_CTRL(box)->moveto(Geom::Point(b->min()[Geom::X], b->max()[Geom::Y]));

                sp_canvas_item_move_to_z(box, 0); // just low enough to not get in the way of other draggable knots

            } else if (mode == BBOX) {
                box = sp_canvas_item_new(sp_desktop_controls(_desktop),
                                         SP_TYPE_CTRLRECT,
                                         NULL);

                SP_CTRLRECT(box)->setRectangle(*b);
                SP_CTRLRECT(box)->setColor(0x000000a0, 0, 0);
                SP_CTRLRECT(box)->setDashed(true);

                sp_canvas_item_move_to_z(box, 0);
            }
        }

        if (box) {
            _item_bboxes.push_back(box);
        }
    }

    _newTextBaselines();
}

void Inkscape::SelCue::_newTextBaselines()
{
    for (std::vector<SPCanvasItem*>::iterator i = _text_baselines.begin(); i != _text_baselines.end(); i++) {
        gtk_object_destroy(*i);
    }
    _text_baselines.clear();

    for (GSList const *l = _selection->itemList(); l != NULL; l = l->next) {
        SPItem *item = (SPItem *) l->data;

        SPCanvasItem* baseline_point = NULL;
        if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item)) { // visualize baseline
            Inkscape::Text::Layout const *layout = te_get_layout(item);
            if (layout != NULL && layout->outputExists()) {
                boost::optional<Geom::Point> pt = layout->baselineAnchorPoint();
                if (pt) {
                    baseline_point = sp_canvas_item_new(sp_desktop_controls(_desktop), SP_TYPE_CTRL,
                        "mode", SP_CTRL_MODE_XOR,
                        "size", 4.0,
                        "filled", 0,
                        "stroked", 1,
                        "stroke_color", 0x000000ff,
                        NULL);

                    sp_canvas_item_show(baseline_point);
                    SP_CTRL(baseline_point)->moveto((*pt) * sp_item_i2d_affine(item));
                    sp_canvas_item_move_to_z(baseline_point, 0);
                }
            }
        }

        if (baseline_point) {
               _text_baselines.push_back(baseline_point);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
