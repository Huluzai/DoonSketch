#ifndef SEEN_CONN_AVOID_REF
#define SEEN_CONN_AVOID_REF

/** \file
 * A class for handling shape interaction with libavoid.
 */
/*
 * Authors:
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * Copyright (C) 2005 Michael Wybrow
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <stddef.h>
#include <sigc++/connection.h>

struct SPDesktop;
struct SPItem;
struct ConnectionPoint;
typedef std::map<int, ConnectionPoint> IdConnectionPointMap;
namespace Avoid { class ShapeRef; }

class SPAvoidRef {
public:
    SPAvoidRef(SPItem *spitem);
    virtual ~SPAvoidRef();

    // libavoid's internal representation of the item.
    Avoid::ShapeRef *shapeRef;

    // Used for holding connection points for item
    IdConnectionPointMap connection_points;

    void setAvoid(char const *value);
    void setConnectionPoints(gchar const *value);
    void addConnectionPoint(ConnectionPoint &cp);
    void updateConnectionPoint(ConnectionPoint &cp);
    void deleteConnectionPoint(ConnectionPoint &cp);
    void handleSettingChange(void);

    // Returns a list of SPItems of all connectors/shapes attached to
    // this object.  Pass one of the following for 'type':
    //     Avoid::runningTo
    //     Avoid::runningFrom
    //     Avoid::runningToAndFrom
    GSList *getAttachedShapes(const unsigned int type);
    GSList *getAttachedConnectors(const unsigned int type);
    Geom::Point getConnectionPointPos(const int type, const int id);

    bool isValidConnPointId( const int type, const int id );

private:
    SPItem *item;

    // true if avoiding, false if not.
    bool setting;
    bool new_setting;

    // A sigc connection for transformed signal.
    sigc::connection _transformed_connection;
    void setConnectionPointsAttrUndoable(const gchar* value, const gchar* action);
};

extern GSList *get_avoided_items(GSList *list, SPObject *from,
        SPDesktop *desktop, bool initialised = true);
extern void avoid_item_move(Geom::Matrix const *mp, SPItem *moved_item);
extern void init_avoided_shape_geometry(SPDesktop *desktop);

static const double defaultConnSpacing = 3.0;
static const double defaultConnCurvature = 3.0;

#endif /* !SEEN_CONN_AVOID_REF */

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
