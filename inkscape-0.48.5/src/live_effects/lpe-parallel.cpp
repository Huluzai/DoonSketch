#define INKSCAPE_LPE_PARALLEL_CPP
/** \file
 * LPE <parallel> implementation
 */
/*
 * Authors:
 *   Maximilian Albert
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-parallel.h"
#include "sp-shape.h"
#include "display/curve.h"

#include <2geom/path.h>
#include <2geom/transforms.h>

namespace Inkscape {
namespace LivePathEffect {

namespace Pl {

class KnotHolderEntityLeftEnd : public LPEKnotHolderEntity
{
public:
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get();
};

class KnotHolderEntityRightEnd : public LPEKnotHolderEntity
{
public:
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get();
};

} // namespace Pl

LPEParallel::LPEParallel(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // initialise your parameters here:
    offset_pt(_("Offset"), _("Adjust the offset"), "offset_pt", &wr, this),
    length_left(_("Length left"), _("Specifies the left end of the parallel"), "length-left", &wr, this, 150),
    length_right(_("Length right"), _("Specifies the right end of the parallel"), "length-right", &wr, this, 150)
{
    show_orig_path = true;

    registerParameter(dynamic_cast<Parameter *>(&offset_pt));
    registerParameter( dynamic_cast<Parameter *>(&length_left) );
    registerParameter( dynamic_cast<Parameter *>(&length_right) );

    registerKnotHolderHandle(new Pl::KnotHolderEntityLeftEnd(), _("Adjust the \"left\" end of the parallel"));
    registerKnotHolderHandle(new Pl::KnotHolderEntityRightEnd(), _("Adjust the \"right\" end of the parallel"));
}

LPEParallel::~LPEParallel()
{

}

void
LPEParallel::doOnApply (SPLPEItem *lpeitem)
{
    SPCurve *curve = SP_SHAPE(lpeitem)->curve;

    A = *(curve->first_point());
    B = *(curve->last_point());
    dir = unit_vector(B - A);

    offset_pt.param_set_and_write_new_value((A + B)/2 + dir.ccw() * 100);
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEParallel::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > output;

    A = pwd2_in.firstValue();
    B = pwd2_in.lastValue();
    dir = unit_vector(B - A);

    C = offset_pt - dir * length_left;
    D = offset_pt + dir * length_right;

    output = Piecewise<D2<SBasis> >(D2<SBasis>(Linear(C[X], D[X]), Linear(C[Y], D[Y])));
    
    return output + dir;
}

namespace Pl {

// TODO: make this more generic
static LPEParallel *
get_effect(SPItem *item)
{
    Effect *effect = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
    if (effect->effectType() != PARALLEL) {
        g_print ("Warning: Effect is not of type LPEParallel!\n");
        return NULL;
    }
    return static_cast<LPEParallel *>(effect);
}

void
KnotHolderEntityLeftEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/)
{
    using namespace Geom;

    LPEParallel *lpe = get_effect(item);
    
    Geom::Point const s = snap_knot_position(p);

    double lambda = L2(s - lpe->offset_pt) * sgn(dot(s - lpe->offset_pt, lpe->dir));
    lpe->length_left.param_set_value(-lambda);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

void
KnotHolderEntityRightEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/)
{
    using namespace Geom;

    LPEParallel *lpe = get_effect(item);
    
    Geom::Point const s = snap_knot_position(p);

    double lambda = L2(s - lpe->offset_pt) * sgn(dot(s - lpe->offset_pt, lpe->dir));
    lpe->length_right.param_set_value(lambda);

    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

Geom::Point
KnotHolderEntityLeftEnd::knot_get()
{
    LPEParallel *lpe = get_effect(item);
    return lpe->C;
}

Geom::Point
KnotHolderEntityRightEnd::knot_get()
{
    LPEParallel *lpe = get_effect(item);
    return lpe->D;
}

} // namespace Pl

/* ######################## */

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
