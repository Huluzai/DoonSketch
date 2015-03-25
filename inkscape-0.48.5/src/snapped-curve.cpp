/**
 *    \file src/snapped-curve.cpp
 *    \brief SnappedCurve class.
 *
 *    Authors:
 *      Diederik van Lierop <mail@diedenrezi.nl>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include "snapped-curve.h"
#include <2geom/crossing.h>
#include <2geom/path-intersection.h>

Inkscape::SnappedCurve::SnappedCurve(Geom::Point const &snapped_point, int num_path, int num_segm, Geom::Coord const &snapped_distance, Geom::Coord const &snapped_tolerance, bool const &always_snap, bool const &fully_constrained, Geom::Curve const *curve, SnapSourceType source, long source_num, SnapTargetType target, Geom::OptRect target_bbox)
{
    _num_path = num_path;
    _num_segm = num_segm;
    _distance = snapped_distance;
    _tolerance = std::max(snapped_tolerance, 1.0);
    _always_snap = always_snap;
    _curve = curve;
    _second_distance = NR_HUGE;
    _second_tolerance = 1;
    _second_always_snap = false;
    _point = snapped_point;
    _at_intersection = false;
    _fully_constrained = fully_constrained;
    _source = source;
    _source_num = source_num;
    _target = target;
    _target_bbox = target_bbox;
}

Inkscape::SnappedCurve::SnappedCurve()
{
    _num_path = 0;
    _num_segm = 0;
    _distance = NR_HUGE;
    _tolerance = 1;
    _always_snap = false;
    _curve = NULL;
    _second_distance = NR_HUGE;
    _second_tolerance = 1;
    _second_always_snap = false;
    _point = Geom::Point(0,0);
    _at_intersection = false;
    _fully_constrained = false;
    _source = SNAPSOURCE_UNDEFINED;
    _source_num = 0;
    _target = SNAPTARGET_UNDEFINED;
    _target_bbox = Geom::OptRect();
}

Inkscape::SnappedCurve::~SnappedCurve()
{
}

Inkscape::SnappedPoint Inkscape::SnappedCurve::intersect(SnappedCurve const &curve, Geom::Point const &p, Geom::Matrix dt2doc) const
{
    // Calculate the intersections of two curves, which are both within snapping range, and
    // return only the closest intersection
    // The point of intersection should be considered for snapping, but might be outside the snapping range
    // PS: We need p (the location of the mouse pointer) for find out which intersection is the
    // closest, as there might be multiple intersections of two curves
    Geom::Crossings cs = crossings(*(this->_curve), *(curve._curve));

    if (cs.size() > 0) {
        // There might be multiple intersections: find the closest
        Geom::Coord best_dist = NR_HUGE;
        Geom::Point best_p = Geom::Point(NR_HUGE, NR_HUGE);
        for (Geom::Crossings::const_iterator i = cs.begin(); i != cs.end(); i++) {
            Geom::Point p_ix = this->_curve->pointAt((*i).ta);
            Geom::Coord dist = Geom::distance(p_ix, p);

            // Test if we have two segments (curves) from the same path..
            if (this->_num_path == curve._num_path) {
                // Never try to intersect a segment with itself
                if (this->_num_segm == curve._num_segm) continue;
                // Two subsequent segments (curves) in a path will have a common node; this node is not considered to be an intersection
                if (this->_num_segm == curve._num_segm + 1 && (*i).ta == 0 && (*i).tb == 1) continue;
                if (this->_num_segm + 1 == curve._num_segm && (*i).ta == 1 && (*i).tb == 0) continue;
            }

            if (dist < best_dist) {
                best_dist = dist;
                best_p = p_ix;
            }
        }

        // Now we've found the closest intersection, return it as a SnappedPoint
        bool const use_this_as_primary = _distance < curve.getSnapDistance();
        Inkscape::SnappedCurve const *primaryC = use_this_as_primary ? this : &curve;
        Inkscape::SnappedCurve const *secondaryC = use_this_as_primary ? &curve : this;

        // The intersection should in fact be returned in desktop coordinates
        best_p = best_p * dt2doc;

        Geom::Coord primaryDist = use_this_as_primary ? Geom::L2(best_p - this->getPoint()) : Geom::L2(best_p - curve.getPoint());
        Geom::Coord secondaryDist = use_this_as_primary ? Geom::L2(best_p - curve.getPoint()) : Geom::L2(best_p - this->getPoint());
        // TODO: Investigate whether it is possible to use document coordinates everywhere
        // in the snapper code. Only the mouse position should be in desktop coordinates, I guess.
        // All paths are already in document coords and we are certainly not going to change THAT.
        return SnappedPoint(best_p, Inkscape::SNAPSOURCE_UNDEFINED, primaryC->getSourceNum(), Inkscape::SNAPTARGET_PATH_INTERSECTION, primaryDist, primaryC->getTolerance(), primaryC->getAlwaysSnap(), true, false, true,
                secondaryDist, secondaryC->getTolerance(), secondaryC->getAlwaysSnap());
    }

    // No intersection
    return SnappedPoint(Geom::Point(NR_HUGE, NR_HUGE), SNAPSOURCE_UNDEFINED, 0, SNAPTARGET_UNDEFINED, NR_HUGE, 0, false, false, false, false, NR_HUGE, 0, false);
}

// search for the closest snapped line
bool getClosestCurve(std::list<Inkscape::SnappedCurve> const &list, Inkscape::SnappedCurve &result)
{
    bool success = false;

    for (std::list<Inkscape::SnappedCurve>::const_iterator i = list.begin(); i != list.end(); i++) {
        if ((i == list.begin()) || (*i).getSnapDistance() < result.getSnapDistance()) {
            result = *i;
            success = true;
        }
    }

    return success;
}

// search for the closest intersection of two snapped curves, which are both member of the same collection
bool getClosestIntersectionCS(std::list<Inkscape::SnappedCurve> const &list, Geom::Point const &p, Inkscape::SnappedPoint &result, Geom::Matrix dt2doc)
{
    bool success = false;

    for (std::list<Inkscape::SnappedCurve>::const_iterator i = list.begin(); i != list.end(); i++) {
        if ((*i).getTarget() != Inkscape::SNAPTARGET_BBOX_EDGE) { // We don't support snapping to intersections of bboxes,
            // as this would require two bboxes two be flashed in the snap indicator
            std::list<Inkscape::SnappedCurve>::const_iterator j = i;
            j++;
            for (; j != list.end(); j++) {
                if ((*j).getTarget() != Inkscape::SNAPTARGET_BBOX_EDGE) { // We don't support snapping to intersections of bboxes
                    Inkscape::SnappedPoint sp = (*i).intersect(*j, p, dt2doc);
                    if (sp.getAtIntersection()) {
                        // if it's the first point
                        bool const c1 = !success;
                        // or, if it's closer
                        bool const c2 = sp.getSnapDistance() < result.getSnapDistance();
                        // or, if it's just as close then look at the other distance
                        // (only relevant for snapped points which are at an intersection)
                        bool const c3 = (sp.getSnapDistance() == result.getSnapDistance()) && (sp.getSecondSnapDistance() < result.getSecondSnapDistance());
                        // then prefer this point over the previous one
                        if (c1 || c2 || c3) {
                            result = sp;
                            success = true;
                        }
                    }
                }
            }
        }
    }

    return success;
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
