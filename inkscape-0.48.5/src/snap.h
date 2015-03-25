/**
 * \file snap.h
 * \brief Per-desktop object that handles snapping queries
 *//*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Carl Hetherington <inkscape@carlh.net>
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 2006-2007 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2000-2002 Lauris Kaplinski
 * Copyright (C) 2000-2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_SNAP_H
#define SEEN_SNAP_H

#include <vector>
#include "guide-snapper.h"
#include "object-snapper.h"
#include "snap-preferences.h"

/* Guides */
enum SPGuideDragType { // used both here and in desktop-events.cpp
    SP_DRAG_TRANSLATE,
    SP_DRAG_ROTATE,
    SP_DRAG_MOVE_ORIGIN,
    SP_DRAG_NONE
};

class SPNamedView;

/// Class to coordinate snapping operations
/**
 * The SnapManager class handles most (if not all) of the interfacing of the snapping mechanisms
 * with the other parts of the code base. It stores the references to the various types of snappers
 * for grid, guides and objects, and it stores most of the snapping preferences. Besides that
 * it provides methods to setup the snapping environment (e.g. keeps a list of the items to ignore
 * when looking for snap target candidates, and toggling of the snap indicator), and it provides
 * many different methods for snapping queries (free snapping vs. constrained snapping,
 * returning the result by reference or through a return statement, etc.)
 * 
 * Each SPNamedView has one of these.  It offers methods to snap points to whatever
 * snappers are defined (e.g. grid, guides etc.).  It also allows callers to snap
 * points which have undergone some transformation (e.g. translation, scaling etc.)
 *
 * \par How snapping is implemented in Inkscape
 * \par
 * The snapping system consists of two key elements. The first one is the snap manager
 * (this class), which keeps some data about objects in the document and answers queries
 * of the type "given this point and type of transformation, what is the best place
 * to snap to?".
 * 
 * The second is in event-context.cpp and implements the snapping timeout. Whenever a motion
 * events happens over the canvas, it stores it for later use and initiates a timeout.
 * This timeout is discarded whenever a new motion event occurs. When the timeout expires,
 * a global flag in SnapManager, accessed via getSnapPostponedGlobally(), is set to true
 * and the stored event is replayed, but this time with snapping enabled. This way you can
 * write snapping code directly in your control point's dragged handler as if there was
 * no timeout.
 */

class SnapManager
{
public:
    enum Transformation {
        TRANSLATION,
        SCALE,
        STRETCH,
        SKEW
    };

    SnapManager(SPNamedView const *v);

    typedef std::list<const Inkscape::Snapper*> SnapperList;

    /**
     * Return true if any snapping might occur, whether its to grids, guides or objects.
     *
     * Each snapper instance handles its own snapping target, e.g. grids, guides or
     * objects. This method iterates through all these snapper instances and returns
     * true if any of the snappers might possible snap, considering only the relevant
     * snapping preferences.
     *
     * @return true if one of the snappers will try to snap to something.
     */
    bool someSnapperMightSnap(bool immediately = true) const;

    /**
     * @return true if one of the grids might be snapped to.
     */
    bool gridSnapperMightSnap() const;

    void setup(SPDesktop const *desktop,
            bool snapindicator = true,
            SPItem const *item_to_ignore = NULL,
            std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes = NULL,
            SPGuide *guide_to_ignore = NULL);

    void setup(SPDesktop const *desktop,
               bool snapindicator,
               std::vector<SPItem const *> &items_to_ignore,
               std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes = NULL,
               SPGuide *guide_to_ignore = NULL);
    void setupIgnoreSelection(SPDesktop const *desktop,
                              bool snapindicator = true,
                              std::vector<Inkscape::SnapCandidatePoint> *unselected_nodes = NULL,
                              SPGuide *guide_to_ignore = NULL);

    // freeSnapReturnByRef() is preferred over freeSnap(), because it only returns a
    // point if snapping has occurred (by overwriting p); otherwise p is untouched
    void freeSnapReturnByRef(Geom::Point &p,
                                Inkscape::SnapSourceType const source_type,
                                Geom::OptRect const &bbox_to_snap = Geom::OptRect()) const;

    Inkscape::SnappedPoint freeSnap(Inkscape::SnapCandidatePoint const &p,
                                    Geom::OptRect const &bbox_to_snap = Geom::OptRect() ) const;

    void preSnap(Inkscape::SnapCandidatePoint const &p);

    Geom::Point multipleOfGridPitch(Geom::Point const &t, Geom::Point const &origin);

    // constrainedSnapReturnByRef() is preferred over constrainedSnap(), because it only returns a
    // point, by overwriting p, if snapping has occurred; otherwise p is untouched
    void constrainedSnapReturnByRef(Geom::Point &p,
                                    Inkscape::SnapSourceType const source_type,
                                    Inkscape::Snapper::ConstraintLine const &constraint,
                                    Geom::OptRect const &bbox_to_snap = Geom::OptRect()) const;

    Inkscape::SnappedPoint constrainedSnap(Inkscape::SnapCandidatePoint const &p,
                                           Inkscape::Snapper::ConstraintLine const &constraint,
                                           Geom::OptRect const &bbox_to_snap = Geom::OptRect()) const;

    void guideFreeSnap(Geom::Point &p, Geom::Point const &guide_normal, SPGuideDragType drag_type) const;
    void guideConstrainedSnap(Geom::Point &p, SPGuide const &guideline) const;

    Inkscape::SnappedPoint freeSnapTranslation(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                               Geom::Point const &pointer,
                                               Geom::Point const &tr) const;

    Inkscape::SnappedPoint constrainedSnapTranslation(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                                      Geom::Point const &pointer,
                                                      Inkscape::Snapper::ConstraintLine const &constraint,
                                                      Geom::Point const &tr) const;

    Inkscape::SnappedPoint freeSnapScale(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                         Geom::Point const &pointer,
                                         Geom::Scale const &s,
                                         Geom::Point const &o) const;

    Inkscape::SnappedPoint constrainedSnapScale(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                                Geom::Point const &pointer,
                                                Geom::Scale const &s,
                                                Geom::Point const &o) const;

    Inkscape::SnappedPoint constrainedSnapStretch(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                                  Geom::Point const &pointer,
                                                  Geom::Coord const &s,
                                                  Geom::Point const &o,
                                                  Geom::Dim2 d,
                                                  bool uniform) const;

    Inkscape::SnappedPoint constrainedSnapSkew(std::vector<Inkscape::SnapCandidatePoint> const &p,
                                               Geom::Point const &pointer,
                                               Inkscape::Snapper::ConstraintLine const &constraint,
                                               Geom::Point const &s, // s[0] = skew factor, s[1] = scale factor
                                               Geom::Point const &o,
                                               Geom::Dim2 d) const;

    Inkscape::GuideSnapper guide;      ///< guide snapper
    Inkscape::ObjectSnapper object;    ///< snapper to other objects
    Inkscape::SnapPreferences snapprefs;

    SnapperList getSnappers() const;
    SnapperList getGridSnappers() const;

    SPDesktop const *getDesktop() const {return _desktop;}
    SPNamedView const *getNamedView() const {return _named_view;}
    SPDocument *getDocument() const;
    SPGuide const *getGuideToIgnore() const {return _guide_to_ignore;}

    bool getSnapIndicator() const {return _snapindicator;}

    Inkscape::SnappedPoint findBestSnap(Inkscape::SnapCandidatePoint const &p, SnappedConstraints const &sc, bool constrained, bool noCurves = false, bool allowOffScreen = false) const;

protected:
    SPNamedView const *_named_view;

private:
    std::vector<SPItem const *> _items_to_ignore; ///< Items that should not be snapped to, for example the items that are currently being dragged. Set using the setup() method
    SPGuide *_guide_to_ignore; ///< A guide that should not be snapped to, e.g. the guide that is currently being dragged
    SPDesktop const *_desktop;
    bool _snapindicator; ///< When true, an indicator will be drawn at the position that was being snapped to
    std::vector<Inkscape::SnapCandidatePoint> *_unselected_nodes; ///< Nodes of the path that is currently being edited and which have not been selected and which will therefore be stationary. Only these nodes will be considered for snapping to. Of each unselected node both the position (Geom::Point) and the type (Inkscape::SnapTargetType) will be stored
    //TODO: Make _unselected_nodes type safe; in the line above int is used for Inkscape::SnapTargetType, but if I remember
    //correctly then in other cases the int is being used for Inkscape::SnapSourceType, or for both. How to make
    //this type safe?

    Inkscape::SnappedPoint _snapTransformed(std::vector<Inkscape::SnapCandidatePoint> const &points,
                                            Geom::Point const &pointer,
                                            bool constrained,
                                            Inkscape::Snapper::ConstraintLine const &constraint,
                                            Transformation transformation_type,
                                            Geom::Point const &transformation,
                                            Geom::Point const &origin,
                                            Geom::Dim2 dim,
                                            bool uniform) const;

    Geom::Point _transformPoint(Inkscape::SnapCandidatePoint const &p,
                                            Transformation const transformation_type,
                                            Geom::Point const &transformation,
                                            Geom::Point const &origin,
                                            Geom::Dim2 const dim,
                                            bool const uniform) const;

    void _displaySnapsource(Inkscape::SnapCandidatePoint const &p) const;
};

#endif /* !SEEN_SNAP_H */

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
