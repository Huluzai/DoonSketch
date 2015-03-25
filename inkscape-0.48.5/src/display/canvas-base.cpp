//
//  canvas-base.c
//  InkscapeCore
//
//  Created by 张 光建 on 14/11/11.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#include "canvas-base.h"
#include "sp-canvas.h"
#include "display/rendermode.h"

CanvasBase::CanvasBase()
{
    grabbed_item = NULL;
    current_item = NULL;
    focused_item = NULL;
    rendermode = Inkscape::RENDERMODE_OUTLINE;
    gen_all_enter_events = 0;
    new_current_item = NULL;
    need_repick = 0;
    close_enough = 0;
    grabbed_event_mask = 0;
    data_table = g_hash_table_new(&g_str_hash, &g_direct_equal);
    reference_cnt = 1;
}

CanvasBase::~CanvasBase()
{
    g_hash_table_unref(data_table);
}

CanvasBase *CanvasBase::ref()
{
    reference_cnt++;
    return this;
}

void CanvasBase::unref()
{
    reference_cnt--;
    if (reference_cnt == 0) {
        delete this;
    }
}

void CanvasBase::set_data(char *key, void *value)
{
    g_hash_table_insert(data_table, key, value);
}

void *CanvasBase::get_data(char *key)
{
    return g_hash_table_lookup(data_table, key);
}


#if 0
/**
 * Registers the SPCanvas class if necessary, and returns the type ID
 * associated to it.
 *
 * \return The type ID of the SPCanvas class.
 **/
GType sp_canvas_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPCanvasBaseClass),
            0, // base_init
            0, // base_finalize
            0,
            0, // class_finalize
            0, // class_data
            sizeof(SPCanvasBase),
            0, // n_preallocs
            0,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_WIDGET, "CanvasBase", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

SPCanvasGroup *sp_canvas_root(SPCanvasBase *canvas)
{
    return SP_CANVAS_CLASS(canvas)->get_root(canvas);
}

void sp_canvas_scroll_to(SPCanvasBase *canvas, double cx, double cy, unsigned int clear, bool is_scrolling)
{
    SP_CANVAS_CLASS(canvas)->scroll_to(canvas, cx, cy, clear, is_scrolling);
}

void sp_canvas_update_now(SPCanvasBase *canvas)
{
    SP_CANVAS_CLASS(canvas)->update_now(canvas);
}

void sp_canvas_request_redraw(SPCanvasBase *canvas, int x1, int y1, int x2, int y2)
{
    SP_CANVAS_CLASS(canvas)->request_redraw(canvas, x1, y1, x2, y2);
}

void sp_canvas_force_full_redraw_after_interruptions(SPCanvasBase *canvas, unsigned int count)
{
    SP_CANVAS_CLASS(canvas)->force_full_redraw_after_interruptions(canvas, count);
}

void sp_canvas_end_forced_full_redraws(SPCanvasBase *canvas)
{
    SP_CANVAS_CLASS(canvas)->end_forced_full_redraws(canvas);
}

bool sp_canvas_world_pt_inside_window(SPCanvasBase const *canvas, Geom::Point const &world)
{
    return SP_CANVAS_CLASS(canvas)->world_pt_inside_window(canvas, world);
}

void sp_canvas_window_to_world(SPCanvasBase const *canvas, double winx, double winy, double *worldx, double *worldy)
{
    SP_CANVAS_CLASS(canvas)->window_to_world(canvas,winx, winy, worldx, worldy);
}

void sp_canvas_world_to_window(SPCanvasBase const *canvas, double worldx, double worldy, double *winx, double *winy)
{
    SP_CANVAS_CLASS(canvas)->world_to_window(canvas, worldx, worldy, winx, winy);
}

Geom::Point sp_canvas_window_to_world(SPCanvasBase const *canvas, Geom::Point const win)
{
    return SP_CANVAS_CLASS(canvas)->window_to_world_point(canvas, win);
}

Geom::Point sp_canvas_world_to_window(SPCanvasBase const *canvas, Geom::Point const world)
{
    return SP_CANVAS_CLASS(canvas)->world_to_window_point(canvas, world);
}

Geom::Rect sp_canvas_get_view_box(SPCanvasBase const *canvas)
{
    return SP_CANVAS_CLASS(canvas)->get_view_box(canvas);
}

NR::IRect sp_canvas_get_view_box_integers(SPCanvasBase const *canvas)
{
    return SP_CANVAS_CLASS(canvas)->get_view_box_integers(canvas);
}
#endif

