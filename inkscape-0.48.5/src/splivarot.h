#ifndef __SP_LIVAROT_H__
#define __SP_LIVAROT_H__

/*
 * boolops and outlines
 *
 * public domain
 */

#include "livarot/Path.h"
#include <2geom/forward.h>
#include <2geom/path.h>
class SPCurve;
struct SPItem;

// boolean operations
// work on the current selection
// selection has 2 contain exactly 2 items
void sp_selected_path_union (SPDesktop *desktop);
void sp_selected_path_union_skip_undo (SPDesktop *desktop);
void sp_selected_path_intersect (SPDesktop *desktop);
void sp_selected_path_diff (SPDesktop *desktop);
void sp_selected_path_diff_skip_undo (SPDesktop *desktop);
void sp_selected_path_symdiff (SPDesktop *desktop);
void sp_selected_path_cut (SPDesktop *desktop);
void sp_selected_path_slice (SPDesktop *desktop);

// offset/inset of a curve
// takes the fill-rule in consideration
// offset amount is the stroke-width of the curve
void sp_selected_path_offset (SPDesktop *desktop);
void sp_selected_path_offset_screen (SPDesktop *desktop, double pixels);
void sp_selected_path_inset (SPDesktop *desktop);
void sp_selected_path_inset_screen (SPDesktop *desktop, double pixels);
void sp_selected_path_create_offset (SPDesktop *desktop);
void sp_selected_path_create_inset (SPDesktop *desktop);
void sp_selected_path_create_updating_offset (SPDesktop *desktop);
void sp_selected_path_create_updating_inset (SPDesktop *desktop);

void sp_selected_path_create_offset_object_zero (SPDesktop *desktop);
void sp_selected_path_create_updating_offset_object_zero (SPDesktop *desktop);

// outline of a curve
// uses the stroke-width
void sp_selected_path_outline (SPDesktop *desktop);
Geom::PathVector* item_outline(SPItem const *item);

// simplifies a path (removes small segments and the like)
void sp_selected_path_simplify (SPDesktop *desktop);

Path *Path_for_item(SPItem *item, bool doTransformation, bool transformFull = true);
Geom::PathVector* pathvector_for_curve(SPItem *item, SPCurve *curve, bool doTransformation, bool transformFull, Geom::Matrix extraPreAffine, Geom::Matrix extraPostAffine);
SPCurve *curve_for_item(SPItem *item);
boost::optional<Path::cut_position> get_nearest_position_on_Path(Path *path, Geom::Point p, unsigned seg = 0);
Geom::Point get_point_on_Path(Path *path, int piece, double t);

#endif

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
