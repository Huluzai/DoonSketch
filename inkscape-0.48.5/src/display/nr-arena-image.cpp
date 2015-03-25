#define __NR_ARENA_IMAGE_C__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libnr/nr-compose-transform.h>
#include <2geom/transforms.h>
#include <libnr/nr-blit.h>
#include "../preferences.h"
#include "nr-arena-image.h"
#include "style.h"
#include "display/nr-arena.h"
#include "display/nr-filter.h"
#include "display/nr-filter-gaussian.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"
#include "sp-gaussian-blur.h"
#include "filters/blend.h"
#include "display/nr-filter-blend.h"

int nr_arena_image_x_sample = 1;
int nr_arena_image_y_sample = 1;

/*
 * NRArenaCanvasImage
 *
 */

static void nr_arena_image_class_init (NRArenaImageClass *klass);
static void nr_arena_image_init (NRArenaImage *image);
static void nr_arena_image_finalize (NRObject *object);

static unsigned int nr_arena_image_update (NRArenaItem *item, NRRectL *area, NRGC *gc, unsigned int state, unsigned int reset);
static unsigned int nr_arena_image_render (cairo_t *ct, NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
static NRArenaItem *nr_arena_image_pick (NRArenaItem *item, Geom::Point p, double delta, unsigned int flags);

static NRArenaItemClass *parent_class;

NRType
nr_arena_image_get_type (void)
{
    static NRType type = 0;
    if (!type) {
        type = nr_object_register_type (NR_TYPE_ARENA_ITEM,
                                        "NRArenaImage",
                                        sizeof (NRArenaImageClass),
                                        sizeof (NRArenaImage),
                                        (void (*) (NRObjectClass *)) nr_arena_image_class_init,
                                        (void (*) (NRObject *)) nr_arena_image_init);
    }
    return type;
}

static void
nr_arena_image_class_init (NRArenaImageClass *klass)
{
    NRObjectClass *object_class;
    NRArenaItemClass *item_class;

    object_class = (NRObjectClass *) klass;
    item_class = (NRArenaItemClass *) klass;

    parent_class = (NRArenaItemClass *) ((NRObjectClass *) klass)->parent;

    object_class->finalize = nr_arena_image_finalize;
    object_class->cpp_ctor = NRObject::invoke_ctor<NRArenaImage>;

    item_class->update = nr_arena_image_update;
    item_class->render = nr_arena_image_render;
    item_class->pick = nr_arena_image_pick;
}

static void
nr_arena_image_init (NRArenaImage *image)
{
    image->px = NULL;

    image->pxw = image->pxh = image->pxrs = 0;
    image->x = image->y = 0.0;
    image->width = 256.0;
    image->height = 256.0;

    image->grid2px.setIdentity();

    image->style = 0;
    image->render_opacity = TRUE;
}

static void
nr_arena_image_finalize (NRObject *object)
{
    NRArenaImage *image = NR_ARENA_IMAGE (object);

    image->px = NULL;

    ((NRObjectClass *) parent_class)->finalize (object);
}

static unsigned int
nr_arena_image_update( NRArenaItem *item, NRRectL */*area*/, NRGC *gc, unsigned int /*state*/, unsigned int /*reset*/ )
{
    Geom::Matrix grid2px;

    // clear old bbox
    nr_arena_item_request_render(item);

    NRArenaImage *image = NR_ARENA_IMAGE (item);

    /* Copy affine */
    grid2px = gc->transform.inverse();
    double hscale, vscale; // todo: replace with Geom::Scale
    if (image->px) {
        hscale = image->pxw / image->width;
        vscale = image->pxh / image->height;
    } else {
        hscale = 1.0;
        vscale = 1.0;
    }

    image->grid2px[0] = grid2px[0] * hscale;
    image->grid2px[2] = grid2px[2] * hscale;
    image->grid2px[4] = grid2px[4] * hscale;
    image->grid2px[1] = grid2px[1] * vscale;
    image->grid2px[3] = grid2px[3] * vscale;
    image->grid2px[5] = grid2px[5] * vscale;

    image->grid2px[4] -= image->x * hscale;
    image->grid2px[5] -= image->y * vscale;

    /* Calculate bbox */
    if (image->px) {
        NRRect bbox;

        bbox.x0 = image->x;
        bbox.y0 = image->y;
        bbox.x1 = image->x + image->width;
        bbox.y1 = image->y + image->height;

        image->c00 = (Geom::Point(bbox.x0, bbox.y0) * gc->transform);
        image->c01 = (Geom::Point(bbox.x0, bbox.y1) * gc->transform);
        image->c10 = (Geom::Point(bbox.x1, bbox.y0) * gc->transform);
        image->c11 = (Geom::Point(bbox.x1, bbox.y1) * gc->transform);

        nr_rect_d_matrix_transform (&bbox, &bbox, gc->transform);

        item->bbox.x0 = static_cast<NR::ICoord>(floor(bbox.x0)); // Floor gives the coordinate in which the point resides
        item->bbox.y0 = static_cast<NR::ICoord>(floor(bbox.y0));
        item->bbox.x1 = static_cast<NR::ICoord>(ceil (bbox.x1)); // Ceil gives the first coordinate beyond the point
        item->bbox.y1 = static_cast<NR::ICoord>(ceil (bbox.y1));
    } else {
        item->bbox.x0 = (int) gc->transform[4];
        item->bbox.y0 = (int) gc->transform[5];
        item->bbox.x1 = item->bbox.x0 - 1;
        item->bbox.y1 = item->bbox.y0 - 1;
    }

    return NR_ARENA_ITEM_STATE_ALL;
}

#define FBITS 12
#define b2i (image->grid2px)

static unsigned int
nr_arena_image_render( cairo_t *ct, NRArenaItem *item, NRRectL */*area*/, NRPixBlock *pb, unsigned int /*flags*/ )
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    nr_arena_image_x_sample = prefs->getInt("/options/bitmapoversample/value", 1);
    nr_arena_image_y_sample = nr_arena_image_x_sample;

    bool outline = (item->arena->rendermode == Inkscape::RENDERMODE_OUTLINE);

    NRArenaImage *image = NR_ARENA_IMAGE (item);

    Geom::Matrix d2s;

    d2s[0] = b2i[0];
    d2s[1] = b2i[1];
    d2s[2] = b2i[2];
    d2s[3] = b2i[3];
    d2s[4] = b2i[0] * pb->area.x0 + b2i[2] * pb->area.y0 + b2i[4];
    d2s[5] = b2i[1] * pb->area.x0 + b2i[3] * pb->area.y0 + b2i[5];

    if (!outline) {

        if (!image->px) return item->state;

        guint32 Falpha = item->opacity;
        if (Falpha < 1) return item->state;

        unsigned char * dpx = NR_PIXBLOCK_PX (pb);
        int const drs = pb->rs;
        int const dw = pb->area.x1 - pb->area.x0;
        int const dh = pb->area.y1 - pb->area.y0;

        unsigned char * spx = image->px;
        int const srs = image->pxrs;
        int const sw = image->pxw;
        int const sh = image->pxh;

        if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8) {
            /* fixme: This is not implemented yet (Lauris) */
            /* nr_R8G8B8_R8G8B8_R8G8B8A8_N_TRANSFORM (dpx, dw, dh, drs, spx, sw, sh, srs, d2s, Falpha, nr_arena_image_x_sample, nr_arena_image_y_sample); */
        } else if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8P) {
            nr_R8G8B8A8_P_R8G8B8A8_P_R8G8B8A8_N_TRANSFORM (dpx, dw, dh, drs, spx, sw, sh, srs, d2s, Falpha, nr_arena_image_x_sample, nr_arena_image_y_sample);
        } else if (pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8N) {
            nr_R8G8B8A8_N_R8G8B8A8_N_R8G8B8A8_N_TRANSFORM (dpx, dw, dh, drs, spx, sw, sh, srs, d2s, Falpha, nr_arena_image_x_sample, nr_arena_image_y_sample);
        }

        pb->empty = FALSE;

    } else { // outline; draw a rect instead

        if (!ct)
            return item->state;

        guint32 rgba = prefs->getInt("/options/wireframecolors/images", 0xff0000ff);
        // FIXME: we use RGBA buffers but cairo writes BGRA (on i386), so we must cheat
        // by setting color channels in the "wrong" order
        cairo_set_source_rgba(ct, SP_RGBA32_B_F(rgba), SP_RGBA32_G_F(rgba), SP_RGBA32_R_F(rgba), SP_RGBA32_A_F(rgba));

        cairo_set_line_width(ct, 0.5);
        cairo_new_path(ct);

        Geom::Point shift(pb->area.x0, pb->area.y0);
        Geom::Point c00 = image->c00 - shift;
        Geom::Point c01 = image->c01 - shift;
        Geom::Point c11 = image->c11 - shift;
        Geom::Point c10 = image->c10 - shift;

        cairo_move_to (ct, c00[Geom::X], c00[Geom::Y]);

        // the box
        cairo_line_to (ct, c10[Geom::X], c10[Geom::Y]);
        cairo_line_to (ct, c11[Geom::X], c11[Geom::Y]);
        cairo_line_to (ct, c01[Geom::X], c01[Geom::Y]);
        cairo_line_to (ct, c00[Geom::X], c00[Geom::Y]);
        // the diagonals
        cairo_line_to (ct, c11[Geom::X], c11[Geom::Y]);
        cairo_move_to (ct, c10[Geom::X], c10[Geom::Y]);
        cairo_line_to (ct, c01[Geom::X], c01[Geom::Y]);

        cairo_stroke(ct);

        pb->empty = FALSE;
    }

    return item->state;
}

/** Calculates the closest distance from p to the segment a1-a2*/
double
distance_to_segment (Geom::Point p, Geom::Point a1, Geom::Point a2)
{
    // calculate sides of the triangle and their squares
    double d1 = Geom::L2(p - a1);
    double d1_2 = d1 * d1;
    double d2 = Geom::L2(p - a2);
    double d2_2 = d2 * d2;
    double a = Geom::L2(a1 - a2);
    double a_2 = a * a;

    // if one of the angles at the base is > 90, return the corresponding side
    if (d1_2 + a_2 <= d2_2) return d1;
    if (d2_2 + a_2 <= d1_2) return d2;

    // otherwise calculate the height to the base
    double peri = (a + d1 + d2)/2;
    return (2*sqrt(peri * (peri - a) * (peri - d1) * (peri - d2))/a);
}

static NRArenaItem *
nr_arena_image_pick( NRArenaItem *item, Geom::Point p, double delta, unsigned int /*flags*/ )
{
    NRArenaImage *image = NR_ARENA_IMAGE (item);

    if (!image->px) return NULL;

    bool outline = (item->arena->rendermode == Inkscape::RENDERMODE_OUTLINE);

    if (outline) {

        // frame
        if (distance_to_segment (p, image->c00, image->c10) < delta) return item;
        if (distance_to_segment (p, image->c10, image->c11) < delta) return item;
        if (distance_to_segment (p, image->c11, image->c01) < delta) return item;
        if (distance_to_segment (p, image->c01, image->c00) < delta) return item;

        // diagonals
        if (distance_to_segment (p, image->c00, image->c11) < delta) return item;
        if (distance_to_segment (p, image->c10, image->c01) < delta) return item;

        return NULL;

    } else {

        unsigned char *const pixels = image->px;
        int const width = image->pxw;
        int const height = image->pxh;
        int const rowstride = image->pxrs;
        Geom::Point tp = p * image->grid2px;
        int const ix = (int)(tp[Geom::X]);
        int const iy = (int)(tp[Geom::Y]);

        if ((ix < 0) || (iy < 0) || (ix >= width) || (iy >= height))
            return NULL;

        unsigned char *pix_ptr = pixels + iy * rowstride + ix * 4;
        // is the alpha not transparent?
        return (pix_ptr[3] > 0) ? item : NULL;
    }
}

/* Utility */

void
nr_arena_image_set_pixels (NRArenaImage *image, unsigned char const *px, unsigned int pxw, unsigned int pxh, unsigned int pxrs)
{
    nr_return_if_fail (image != NULL);
    nr_return_if_fail (NR_IS_ARENA_IMAGE (image));

    image->px = (unsigned char *) px;
    image->pxw = pxw;
    image->pxh = pxh;
    image->pxrs = pxrs;

    nr_arena_item_request_update (NR_ARENA_ITEM (image), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_image_set_geometry (NRArenaImage *image, double x, double y, double width, double height)
{
    nr_return_if_fail (image != NULL);
    nr_return_if_fail (NR_IS_ARENA_IMAGE (image));

    image->x = x;
    image->y = y;
    image->width = width;
    image->height = height;

    nr_arena_item_request_update (NR_ARENA_ITEM (image), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void nr_arena_image_set_style (NRArenaImage *image, SPStyle *style)
{
    g_return_if_fail(image != NULL);
    g_return_if_fail(NR_IS_ARENA_IMAGE(image));

    if (style) sp_style_ref(style);
    if (image->style) sp_style_unref(image->style);
    image->style = style;

    //if image has a filter
    if (style->filter.set && style->getFilter()) {
        if (!image->filter) {
            int primitives = sp_filter_primitive_count(SP_FILTER(style->getFilter()));
            image->filter = new Inkscape::Filters::Filter(primitives);
        }
        sp_filter_build_renderer(SP_FILTER(style->getFilter()), image->filter);
    } else {
        //no filter set for this image
        delete image->filter;
        image->filter = NULL;
    }

    if (style && style->enable_background.set
        && style->enable_background.value == SP_CSS_BACKGROUND_NEW) {
        image->background_new = true;
    }

    nr_arena_item_request_update(image, NR_ARENA_ITEM_STATE_ALL, FALSE);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
