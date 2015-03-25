#define INKSCAPE_LPE_PERP_BISECTOR_CPP
/** \file
 * LPE <perp_bisector> implementation.
 */
/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Maximilin Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpe-perp_bisector.h"
#include "display/curve.h"
#include "sp-path.h"
#include "line-geometry.h"
#include "sp-lpe-item.h"
#include <2geom/path.h>

namespace Inkscape {
namespace LivePathEffect {
namespace PB {

class KnotHolderEntityEnd : public LPEKnotHolderEntity {
public:
    void bisector_end_set(Geom::Point const &p, bool left = true);
};

class KnotHolderEntityLeftEnd : public KnotHolderEntityEnd {
public:
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get();
};

class KnotHolderEntityRightEnd : public KnotHolderEntityEnd {
public:
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get();
};

// TODO: Make this more generic
static LPEPerpBisector *
get_effect(SPItem *item)
{
    Effect *effect = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
    if (effect->effectType() != PERP_BISECTOR) {
        g_print ("Warning: Effect is not of type LPEPerpBisector!\n");
        return NULL;
    }
    return static_cast<LPEPerpBisector *>(effect);
}

Geom::Point
KnotHolderEntityLeftEnd::knot_get() {
    Inkscape::LivePathEffect::LPEPerpBisector *lpe = get_effect(item);
    return Geom::Point(lpe->C);
}

Geom::Point
KnotHolderEntityRightEnd::knot_get() {
    Inkscape::LivePathEffect::LPEPerpBisector *lpe = get_effect(item);
    return Geom::Point(lpe->D);
}

void
KnotHolderEntityEnd::bisector_end_set(Geom::Point const &p, bool left) {
    Inkscape::LivePathEffect::LPEPerpBisector *lpe =
        dynamic_cast<Inkscape::LivePathEffect::LPEPerpBisector *> (sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item)));
    if (!lpe) return;

    Geom::Point const s = snap_knot_position(p);

    double lambda = Geom::nearest_point(s, lpe->M, lpe->perp_dir);
    if (left) {
        lpe->C = lpe->M + lpe->perp_dir * lambda;
        lpe->length_left.param_set_value(lambda);
    } else {
        lpe->D = lpe->M + lpe->perp_dir * lambda;
        lpe->length_right.param_set_value(-lambda);
    }

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), true, true);
}

void
KnotHolderEntityLeftEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/) {
    bisector_end_set(p);
}

void
KnotHolderEntityRightEnd::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/) {
    bisector_end_set(p, false);
}

} //namescape PB

LPEPerpBisector::LPEPerpBisector(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    length_left(_("Length left"), _("Specifies the left end of the bisector"), "length-left", &wr, this, 200),
    length_right(_("Length right"), _("Specifies the right end of the bisector"), "length-right", &wr, this, 200),
    A(0,0), B(0,0), M(0,0), C(0,0), D(0,0), perp_dir(0,0)
{
    show_orig_path = true;

    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&length_left) );
    registerParameter( dynamic_cast<Parameter *>(&length_right) );

    registerKnotHolderHandle(new PB::KnotHolderEntityLeftEnd(), _("Adjust the bisector's \"left\" end"));
    registerKnotHolderHandle(new PB::KnotHolderEntityRightEnd(), _("Adjust the bisector's \"right\" end"));
}

LPEPerpBisector::~LPEPerpBisector()
{
}

void
LPEPerpBisector::doOnApply (SPLPEItem */*lpeitem*/)
{
    /* make the path a straight line */
    /**
    SPCurve* curve = sp_path_get_curve_for_edit (SP_PATH(lpeitem)); // TODO: Should we use sp_shape_get_curve()?

    Geom::Point A(curve->first_point());
    Geom::Point B(curve->last_point());

    SPCurve *c = new SPCurve();
    c->moveto(A);
    c->lineto(B);
    // TODO: Why doesn't sp_path_set_original_curve(SP_PATH(lpeitem), c, TRUE, true) work?
    SP_PATH(lpeitem)->original_curve = c->ref();
    c->unref();
    **/
}


Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPerpBisector::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > output;

    A = pwd2_in.firstValue();
    B = pwd2_in.lastValue();
    M = (A + B)/2;

    perp_dir = unit_vector((B - A).ccw());

    C = M + perp_dir * length_left;
    D = M - perp_dir * length_right;

    output = Piecewise<D2<SBasis> >(D2<SBasis>(Linear(C[X], D[X]), Linear(C[Y], D[Y])));

    return output;
}

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
