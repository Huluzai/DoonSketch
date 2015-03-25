/** @file
 * @brief LPE perspective path effect implementation.
 */
/* Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) 2007-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "persp3d.h"
//#include "transf_mat_3x4.h"
#include "document.h"

#include "live_effects/lpe-perspective_path.h"
#include "sp-item-group.h"
#include "knot-holder-entity.h"

#include "inkscape.h"

#include <2geom/path.h>

namespace Inkscape {
namespace LivePathEffect {

namespace PP {

class KnotHolderEntityOffset : public LPEKnotHolderEntity
{
public:
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get();
};

} // namespace PP

LPEPerspectivePath::LPEPerspectivePath(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // initialise your parameters here:
    scalex(_("Scale x"), _("Scale factor in x direction"), "scalex", &wr, this, 1.0),
    scaley(_("Scale y"), _("Scale factor in y direction"), "scaley", &wr, this, 1.0),
    offsetx(_("Offset x"), _("Offset in x direction"), "offsetx", &wr, this, 0.0),
    offsety(_("Offset y"), _("Offset in y direction"), "offsety", &wr, this, 0.0),
    uses_plane_xy(_("Uses XY plane?"), _("If true, put the path on the left side of an imaginary box, otherwise on the right side"), "uses_plane_xy", &wr, this, true)
{
    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&scalex) );
    registerParameter( dynamic_cast<Parameter *>(&scaley) );
    registerParameter( dynamic_cast<Parameter *>(&offsetx) );
    registerParameter( dynamic_cast<Parameter *>(&offsety) );
    registerParameter( dynamic_cast<Parameter *>(&uses_plane_xy) );

    registerKnotHolderHandle(new PP::KnotHolderEntityOffset(), _("Adjust the origin"));

    concatenate_before_pwd2 = true; // don't split the path into its subpaths

    Persp3D *persp = persp3d_document_first_persp(inkscape_active_document());

    Proj::TransfMat3x4 pmat = persp->perspective_impl->tmat;

    pmat.copy_tmat(tmat);
}

LPEPerspectivePath::~LPEPerspectivePath()
{

}

void
LPEPerspectivePath::doBeforeEffect (SPLPEItem *lpeitem)
{
    original_bbox(lpeitem, true);
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPerspectivePath::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > path_a_pw = pwd2_in;

    // FIXME: the minus sign is there because the SVG coordinate system goes down;
    //        remove this once we have unified coordinate systems
    path_a_pw = path_a_pw + Geom::Point(offsetx, -offsety);

    D2<Piecewise<SBasis> > B = make_cuts_independent(path_a_pw);
    Piecewise<SBasis> preimage[4];

    //Geom::Point orig = Geom::Point(bounds_X.min(), bounds_Y.middle());
    //orig = Geom::Point(orig[X], sp_document_height(inkscape_active_document()) - orig[Y]);

    //double offset = uses_plane_xy ? boundingbox_X.extent() : 0.0;

    orig = Point(uses_plane_xy ? boundingbox_X.max() : boundingbox_X.min(), boundingbox_Y.middle());

    /**
    g_print ("Orig: (%8.2f, %8.2f)\n", orig[X], orig[Y]);

    g_print ("B[1] - orig[1]: %8.2f\n", (B[1] - orig[1])[0].valueAt(0));
    g_print ("B[0] - orig[0]: %8.2f\n", (B[0] - orig[0])[0].valueAt(0));
    **/

    if (uses_plane_xy) {
        preimage[0] =  (-B[0] + orig[0]) * scalex / 200.0;
        preimage[1] =  ( B[1] - orig[1]) * scaley / 400.0;
        preimage[2] =  B[0] - B[0]; // hack!
    } else {
        preimage[0] =  B[0] - B[0]; // hack!
        preimage[1] =  (B[1] - orig[1]) * scaley / 400.0;
        preimage[2] =  (B[0] - orig[0]) * scalex / 200.0;
    }

    /* set perspective origin to first point of path */
    tmat[0][3] = orig[0];
    tmat[1][3] = orig[1];

    /**
    g_print ("preimage[1]: %8.2f\n", preimage[1][0].valueAt(0));
    g_print ("preimage[2]: %8.2f\n", preimage[2][0].valueAt(0));
    **/

    Piecewise<SBasis> res[3];
    for (int j = 0; j < 3; ++j) {
        res[j] =
              preimage[0] * tmat[j][0]
            + preimage[1] * tmat[j][1]
            + preimage[2] * tmat[j][2]
            +               tmat[j][3];
    }
    D2<Piecewise<SBasis> > result(divide(res[0],res[2], 3),
                                  divide(res[1],res[2], 3));

    Piecewise<D2<SBasis> > output = sectionize(result);

    return output;
}

namespace PP {

// TODO: make this more generic
static LPEPerspectivePath *
get_effect(SPItem *item)
{
    Effect *effect = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
    if (effect->effectType() != PERSPECTIVE_PATH) {
        g_print ("Warning: Effect is not of type LPEPerspectivePath!\n");
        return NULL;
    }
    return static_cast<LPEPerspectivePath *>(effect);
}

void
KnotHolderEntityOffset::knot_set(Geom::Point const &p, Geom::Point const &origin, guint /*state*/)
{
    using namespace Geom;
 
    LPEPerspectivePath* lpe = get_effect(item);

    Geom::Point const s = snap_knot_position(p);

    lpe->offsetx.param_set_value((s - origin)[Geom::X]);
    lpe->offsety.param_set_value(-(s - origin)[Geom::Y]); // additional minus sign is due to coordinate system flipping

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

Geom::Point
KnotHolderEntityOffset::knot_get()
{
    LPEPerspectivePath* lpe = get_effect(item);
    return lpe->orig + Geom::Point(lpe->offsetx, -lpe->offsety);
}

} // namespace PP

} //namespace LivePathEffect
} /* namespace Inkscape */

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
