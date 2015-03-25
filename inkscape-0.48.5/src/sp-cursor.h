#ifndef SP_CURSOR_H
#define SP_CURSOR_H

#include <gdk/gdk.h>

enum {
    SP_ELLIPSE_CURSOR = GDK_LAST_CURSOR,
    SP_3DBOX_CURSOR,
    SP_CONNECTOR_CURSOR,
    SP_NODE_CURSOR,
    SP_NODE_D_CURSOR,
    SP_DROPPER_CURSOR,
    SP_CALLIGRAPHY_CURSOR,
    SP_ERASER_CURSOR,
    SP_PAINTBUCKET_CURSOR,
    SP_GRADIENT_CURSOR,
    SP_GRADIENT_ADD_CURSOR,
    SP_CROSSHAIRS_CURSOR,
    SP_PEN_CURSOR,
    SP_PENCIL_CURSOR,
    SP_RECT_CURSOR,
    SP_SPIRAL_CURSOR,
    SP_SPRAY_CURSOR,
    SP_STAR_CURSOR,
    SP_TEXT_INSERT_CURSOR,
    SP_TEXT_CURSOR,
    SP_PUSH_CURSOR,
    SP_TWEAK_MOVE_CURSOR,
    SP_THICKEN_CURSOR,
    SP_THIN_CURSOR,
    SP_REPEL_CURSOR,
    SP_ATTRACT_CURSOR,
    SP_ROUGHEN_CURSOR,
    SP_COLOR_CURSOR,
    SP_ZOOM_CURSOR,
    SP_ZOOM_OUT_CURSOR,
    SP_SELECT_CURSOR,
    SP_SELECT_D_CURSOR,
    SP_NULL_CURSOR,
    SP_LAST_CURSOR
};
typedef int SPCursorType;

void sp_cursor_bitmap_and_mask_from_xpm(GdkBitmap **bitmap, GdkBitmap **mask, gchar const *const *xpm);
GdkCursor *sp_cursor_new_from_xpm(gchar const *const *xpm, gint hot_x, gint hot_y);
GdkCursor *sp_stock_gdk_cursor(SPCursorType shape);

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
