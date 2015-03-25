#define __SP_CURSOR_C__

/*
 * Some convenience stuff
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstdio>
#include <cstring>
#include <string>
#include <ctype.h>
#include "sp-cursor.h"

void
sp_cursor_bitmap_and_mask_from_xpm(GdkBitmap **bitmap, GdkBitmap **mask, gchar const *const *xpm)
{
    int height;
    int width;
    int colors;
    int pix;
    sscanf(xpm[0], "%d %d %d %d", &height, &width, &colors, &pix);

    g_return_if_fail (height == 32);
    g_return_if_fail (width == 32);
    g_return_if_fail (colors >= 3);

    int transparent_color = ' ';
    int black_color = '.';

    char pixmap_buffer[(32 * 32)/8];
    char mask_buffer[(32 * 32)/8];

    for (int i = 0; i < colors; i++) {

        char const *p = xpm[1 + i];
        char const ccode = *p;

        p++;
        while (isspace(*p)) {
            p++;
        }
        p++;
        while (isspace(*p)) {
            p++;
        }

	if (strcmp(p, "None") == 0) {
            transparent_color = ccode;
        }

        if (strcmp(p, "#000000") == 0) {
            black_color = ccode;
        }
    }

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; ) {

            char value = 0;
            char maskv = 0;
			
            for (int pix = 0; pix < 8; pix++, x++){
                if (xpm[4+y][x] != transparent_color) {
                    maskv |= 1 << pix;

                    if (xpm[4+y][x] == black_color) {
                        value |= 1 << pix;
                    }
                }
            }

            pixmap_buffer[(y * 4 + x/8)-1] = value;
            mask_buffer[(y * 4 + x/8)-1] = maskv;
        }
    }

    *bitmap = gdk_bitmap_create_from_data(NULL, pixmap_buffer, 32, 32);
    *mask   = gdk_bitmap_create_from_data(NULL, mask_buffer, 32, 32);
}

GdkCursor *
sp_cursor_new_from_xpm(gchar const *const *xpm, gint hot_x, gint hot_y)
{
    GdkColor const fg = { 0, 0, 0, 0 };
    GdkColor const bg = { 0, 65535, 65535, 65535 };

    GdkBitmap *bitmap = NULL;
    GdkBitmap *mask = NULL;

    sp_cursor_bitmap_and_mask_from_xpm (&bitmap, &mask, xpm);
    if ( bitmap != NULL && mask != NULL ) {
        GdkCursor *new_cursor = gdk_cursor_new_from_pixmap (bitmap, mask,
                                           &fg, &bg,
                                           hot_x, hot_y);
        g_object_unref (bitmap);
        g_object_unref (mask);
        return new_cursor;
    }

    return NULL;
}

#include "pixmaps/cursor-ellipse.xpm"
#include "pixmaps/cursor-3dbox.xpm"
#include "pixmaps/cursor-connector.xpm"
#include "pixmaps/cursor-node.xpm"
#include "pixmaps/cursor-node-d.xpm"
#include "pixmaps/cursor-dropper.xpm"
#include "pixmaps/cursor-calligraphy.xpm"
#include "pixmaps/cursor-eraser.xpm"
#include "pixmaps/cursor-paintbucket.xpm"
#include "pixmaps/cursor-gradient.xpm"
#include "pixmaps/cursor-gradient-add.xpm"
#include "pixmaps/cursor-crosshairs.xpm"
#include "pixmaps/cursor-pen.xpm"
#include "pixmaps/cursor-pencil.xpm"
#include "pixmaps/cursor-rect.xpm"
#include "pixmaps/cursor-spiral.xpm"
#include "pixmaps/cursor-spray.xpm"
#include "pixmaps/cursor-star.xpm"
#include "pixmaps/cursor-text-insert.xpm"
#include "pixmaps/cursor-text.xpm"
#include "pixmaps/cursor-tweak-move.xpm"
#include "pixmaps/cursor-thin.xpm"
#include "pixmaps/cursor-thicken.xpm"
#include "pixmaps/cursor-attract.xpm"
#include "pixmaps/cursor-repel.xpm"
#include "pixmaps/cursor-push.xpm"
#include "pixmaps/cursor-roughen.xpm"
#include "pixmaps/cursor-color.xpm"
#include "pixmaps/cursor-zoom.xpm"
#include "pixmaps/cursor-zoom-out.xpm"
#include "pixmaps/cursor-select-d.xpm"
#include "pixmaps/cursor-select-m.xpm"

struct SPCursor {
    gchar const* const* xpm;
    gint hot_x;
    gint hot_y;
};

static SPCursor _user_cursor_table[] = {
    {cursor_ellipse_xpm, 4, 4},         //0
    {cursor_3dbox_xpm, 4, 4},           //1
    {cursor_connector_xpm, 1, 1},       //2
    {cursor_node_xpm, 1, 1},    //?     //3
    {cursor_node_d_xpm, 1, 1},
    {cursor_dropper_xpm, 7, 7},
    {cursor_calligraphy_xpm, 4, 4},     //
    {cursor_eraser_xpm, 4, 4},          //
    {cursor_paintbucket_xpm, 11, 30},   //
    {cursor_gradient_xpm, 4, 4},        //
    {cursor_gradient_add_xpm, 4, 4},    //
    {cursor_crosshairs_xpm, 4, 4},      //
    {cursor_pen_xpm, 4, 4},             //
    {cursor_pencil_xpm, 4, 4},          //
    {cursor_rect_xpm, 4, 4},            //
    {cursor_spiral_xpm, 4, 4},          //
    {cursor_spray_xpm, 4, 4},           //
    {cursor_star_xpm, 4, 4},            //
    {cursor_text_insert_xpm, 7, 10},     //
    {cursor_text_xpm, 7, 7},            //
    {cursor_push_xpm, 4, 4},            //
    {cursor_tweak_move_xpm, 4, 4},      //
    {cursor_thicken_xpm, 4, 4},         //
    {cursor_thin_xpm, 4, 4},            //
    {cursor_repel_xpm, 4, 4},           //
    {cursor_attract_xpm, 4, 4},         //
    {cursor_roughen_xpm, 4, 4},         //
    {cursor_color_xpm, 4, 4},           //
    {cursor_zoom_xpm, 6, 6},            //
    {cursor_zoom_out_xpm, 6, 6},         //
    {cursor_select_m_xpm, 1, 1},
    {cursor_select_d_xpm, 1, 1}
};


static GdkCursor *_cursors[SP_LAST_CURSOR] = {0};

GdkCursor *
sp_stock_gdk_cursor(SPCursorType shape)
{
    g_assert(shape >= 0 && shape < SP_LAST_CURSOR);
    
    if (shape == SP_NULL_CURSOR) {
        return NULL;
    }
    
    if (_cursors[shape]) {
        return _cursors[shape];
    }
    
    GdkCursor *cursor = NULL;
    
    if (shape < GDK_LAST_CURSOR) {
        cursor = gdk_cursor_new((GdkCursorType)shape);
    } else {
        SPCursor c = _user_cursor_table[shape - GDK_LAST_CURSOR];
        cursor = sp_cursor_new_from_xpm(c.xpm, c.hot_x, c.hot_y);
    }
    
    _cursors[shape] = cursor;
    
    return cursor;
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
