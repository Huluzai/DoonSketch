#ifndef SEEN_SP_GRADIENT_FNS_H
#define SEEN_SP_GRADIENT_FNS_H

/** \file
 * Macros and fn declarations related to gradients.
 */

#include <glib.h>
#include <glib-object.h>
#include <2geom/forward.h>
#include "sp-gradient-spread.h"
#include "sp-gradient-units.h"

class SPGradient;

SPGradientSpread sp_gradient_get_spread (SPGradient *gradient);

/* Gradient repr methods */
void sp_gradient_repr_write_vector(SPGradient *gr);
void sp_gradient_repr_clear_vector(SPGradient *gr);

void sp_gradient_render_vector_block_rgba(SPGradient *gr, guchar *px, gint w, gint h, gint rs, gint pos, gint span, bool horizontal);
void sp_gradient_render_vector_block_rgb(SPGradient *gr, guchar *px, gint w, gint h, gint rs, gint pos, gint span, bool horizontal);

/** Transforms to/from gradient position space in given environment */
Geom::Matrix sp_gradient_get_g2d_matrix(SPGradient const *gr, Geom::Matrix const &ctm,
                                      Geom::Rect const &bbox);
Geom::Matrix sp_gradient_get_gs2d_matrix(SPGradient const *gr, Geom::Matrix const &ctm,
                                       Geom::Rect const &bbox);
void sp_gradient_set_gs2d_matrix(SPGradient *gr, Geom::Matrix const &ctm, Geom::Rect const &bbox,
                                 Geom::Matrix const &gs2d);


#endif /* !SEEN_SP_GRADIENT_FNS_H */

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
