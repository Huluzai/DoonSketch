#ifndef __NR_FILTER_DIFFUSELIGHTING_H__
#define __NR_FILTER_DIFFUSELIGHTING_H__

/*
 * feDiffuseLighting renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 * 
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdk.h>
#include "display/nr-light-types.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"
#include "filters/distantlight.h"
#include "filters/pointlight.h"
#include "filters/spotlight.h"
#include "color.h"

namespace Inkscape {
namespace Filters {
    
class FilterDiffuseLighting : public FilterPrimitive {
public:
    union {
        SPFeDistantLight *distant;
        SPFePointLight *point;
        SPFeSpotLight *spot;
    } light;
    LightType light_type;
    gdouble diffuseConstant;
    gdouble surfaceScale;
    guint32 lighting_color;
    
    FilterDiffuseLighting();
    static FilterPrimitive *create();
    virtual ~FilterDiffuseLighting();
    virtual int render(FilterSlot &slot, FilterUnits const &units);
    virtual void area_enlarge(NRRectL &area, Geom::Matrix const &trans);
    virtual FilterTraits get_input_traits();

private:
};

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_DIFFUSELIGHTING_H__ */
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
