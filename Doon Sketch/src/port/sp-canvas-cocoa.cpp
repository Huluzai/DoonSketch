//
//  sp-canvas-cocoa.cpp
//  Inxcape
//
//  Created by 张 光建 on 14-9-1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#include "sp-canvas-cocoa.h"
#include <cairo/cairo-quartz.h>
#include "CocoaCanvasView.h"
#include <display/sp-canvas.h>
#include <2geom/geom.h>
#include <cms-system.h>
#include <display/rendermode.h>
#include <libnr/nr-blit.h>
#include <display/inkscape-cairo.h>
#include <display/sp-canvas.h>
#include "preferences.h"
#include "SWImageTools.h"
#include <gobject/gobject.h>
#include <gdkmm.h>

//#include "CanvasController.h"

#define CANVAS_OUTPUT_VIA_CAIRO 1
#define RequestUpdateNotification   (@"RequestUpdateNotification")

#define left() ((this)->_allocateRect.origin.x)
#define width() ((this)->_allocateRect.size.width)
#define top() ((this)->_allocateRect.origin.y)
#define height() ((this)->_allocateRect.size.height)
#define right() (CGRectGetMaxX((this)->_allocateRect))
#define bottom() (CGRectGetMaxY((this)->_allocateRect))

static gint const sp_canvas_update_priority = G_PRIORITY_HIGH_IDLE;
static bool paint_interrupted = false;

struct PaintRectSetup {
    SPCocoaCanvas* canvas;
    NRRectL big_rect;
    GTimeVal start_time;
    int max_pixels;
    Geom::Point mouse_loc;
};


@interface IdleUpdater : NSObject
@property SPCocoaCanvas *canvas;
@property BOOL on;
- (void)requestUpdateWhenIdle:(id)sender;
@end


@implementation IdleUpdater

- (id)initWithCanvas:(SPCocoaCanvas *)canvas
{
    if (self = [super init]) {
        self.canvas = canvas;
        [defaultNfc addObserver:self selector:@selector(doUpdate:) name:RequestUpdateNotification object:nill];
    }
    return self;
}

- (void)dealloc
{
    [defaultNfc removeObserver:self];
}

- (void)requestUpdateWhenIdle:(id)sender
{
    NSNotificationQueue *nq = [NSNotificationQueue defaultQueue];
    NSNotification *n = [NSNotification notificationWithName:RequestUpdateNotification object:nill];
    [nq enqueueNotification:n postingStyle:NSPostWhenIdle];
    self.on = TRUE;
}

- (void)removeRequest
{
    self.on = FALSE;
}

- (void)doUpdate:(id)sender
{
    if (self.canvas) {
        int const ret = self.canvas->do_update ();
        self.on = self.canvas->needs_redraw();
        if (self.on) {
            [self requestUpdateWhenIdle:nill];
        }
    }
}

@end

/**
 * Idle handler for the canvas that deals with pending updates and redraws.
 */
static gint
idle_handler (gpointer data)
{
    GDK_THREADS_ENTER ();
    
    SPCocoaCanvas *canvas = (SPCocoaCanvas *)data;
    
    canvas->do_update();
    
    int ret = canvas->needs_redraw();
    
    if (!ret) {
#ifdef COCOA_CANVAS_PAINT_WITH_NO_LOCKFOCUS
        // we've had a full unaborted redraw, reset the full redraw counter
        if (canvas->forced_redraw_limit != -1) {
            canvas->forced_redraw_count = 0;
        }
#endif
        /* Reset idle id */
        canvas->idle_id = 0;
    }
    
    GDK_THREADS_LEAVE ();
    
    return ret;
}

/**
 * Convenience function to add an idle handler to a canvas.
 */
static void
add_idle (SPCocoaCanvas *canvas)
{
    if (canvas->idle_id != 0)
        return;
    
    canvas->idle_id = gtk_idle_add_priority (sp_canvas_update_priority, idle_handler, canvas);
}

/**
 * Convenience function to remove the idle handler of a canvas.
 */
static void
remove_idle (SPCocoaCanvas *canvas)
{
    g_message("remove idle %d", canvas->idle_id);
    if (canvas->idle_id) {
        gtk_idle_remove (canvas->idle_id);
        canvas->idle_id = 0;
    }
}

void SPCocoaCanvas::test() const
{
    g_message("const function in c struct is OK !");
}

SPCocoaCanvas::SPCocoaCanvas()
{
    _ui = nill;
    idle_id = 0;
    _is_scrolling = FALSE;
    _allocateRect = CGRectZero;
    _idle_updater = [[IdleUpdater alloc] initWithCanvas:this];
}

SPCocoaCanvas::~SPCocoaCanvas()
{
    _ui = nill;
    shutdown_transients();
    _idle_updater = nill;
}

void SPCocoaCanvas::set_ui_delegate(id<CanvasControl> ui)
{
    if (_ui != ui) {
        shutdown_transients();
    }
    _ui = ui;
}

id<CanvasControl> SPCocoaCanvas::get_ui_delegate() const
{
    return _ui;
}

void SPCocoaCanvas::scroll_to(double cx, double cy, unsigned int clear, bool is_scrolling)
{
    int ix = (int) round(cx); // ix and iy are the new canvas coordinates (integer screen pixels)
    int iy = (int) round(cy); // cx might be negative, so (int)(cx + 0.5) will not do!
    int dx = ix - left(); // dx and dy specify the displacement (scroll) of the
    int dy = iy - top(); // canvas w.r.t its previous position
    
//    GThread *thr = g_thread_self();
//    g_debug("scrolled by thread %p", thr);
    
    if (dx == 0 && dy == 0) {
        return;
    }

    g_debug("canvas scroll to %.1f %.1f", cx, cy);
    
    left() = cx;
    top() = cy;
    
    resize_tiles(left(), top(), right(), bottom());
    
#ifdef COCOA_CANVAS_USE_BUFFER
    if ((dx != 0 || dy != 0) && _scroll_buffer && _image_buffer) {
        [SWImageTools drawToImage:_scroll_buffer
                        fromImage:_image_buffer
                          atPoint:CGPointMake(-dx, dy)
                  withComposition:NO];
        NSBitmapImageRep *t = _scroll_buffer;
        _scroll_buffer = _image_buffer;
        _image_buffer = t;
        
        CGRect hrect = CGRectZero, vrect = CGRectZero;
        
        hrect.origin.x = dx > 0 ? _location[Geom::X] + _allocateSize.width - dx : _location[Geom::X];
        hrect.size.width = dx > 0 ? dx : -dx;
        hrect.origin.y = _location[Geom::Y];
        hrect.size.height = _allocateSize.height;
        
        vrect.origin.y = dy > 0 ? _location[Geom::Y] + _allocateSize.height - dy : _location[Geom::Y];
        vrect.size.height = dy > 0 ? dy : -dy;
        vrect.origin.x = _location[Geom::X];
        vrect.size.width = _allocateSize.width;
        
        request_redraw(hrect.origin.x - 0.5, hrect.origin.y - 0.5, CGRectGetMaxX(hrect) + 0.5, CGRectGetMaxY(hrect) + 0.5);
        request_redraw(vrect.origin.x - 0.5, vrect.origin.y - 0.5, CGRectGetMaxX(vrect) + 0.5, CGRectGetMaxY(vrect) + 0.5);
    }

    if (_ui) {
        [_ui didCanvasScroll:CGPointMake(dx, dy)];
        [_ui didCanvasUpdateRect:CGRectMake(0, 0, _allocateSize.width, _allocateSize.height)];
    }
#else
    if (1/*!clear*/) {
        // scrolling without zoom; redraw only the newly exposed areas
        _is_scrolling = is_scrolling;
        [_ui scrollToPoint:CGPointMake(cx, cy)];
    } else {
        // scrolling as part of zoom; do nothing here - the next do_update will perform full redraw
    }
#endif
}

void SPCocoaCanvas::update_now()
{
    do_update ();
}

int SPCocoaCanvas::do_update()
{
    if (!root()) // canvas may have already be destroyed by closing desktop during interrupted display!
        return TRUE;
    
    /* Cause the update if necessary */
    if (need_update) {
        sp_canvas_item_invoke_update ((SPCanvasItem*)root(), Geom::identity(), 0);
        need_update = FALSE;
    }
    
#ifdef COCOA_CANVAS_PAINT_WITH_NO_LOCKFOCUS
    const CGRect *rects = NULL;
    int rects_num = 0;
    
    get_dirty_rects(&rects, &rects_num);
    
    paint_interrupted = false;
    
    for (int i = 0; i < rects_num && !paint_interrupted; i++) {
        [_ui requestUpdateRect:rects[i]];
    }
    
    if (rects) {
        delete[] rects;
    }
    
    return TRUE;
#else
    return paint ();
#endif
}

void SPCocoaCanvas::request_update ()
{
    need_update = TRUE;
//    [_idle_updater requestUpdateWhenIdle:nill];
    add_idle(this);
//    Geom::Rect r = getViewbox();
//    [_ui requestUpdateRect:CGRectMake(r.min()[0], r.min()[1], r.dimensions()[0], r.dimensions()[1])];
}

void SPCocoaCanvas::request_redraw(int x0, int y0, int x1, int y1)
{
    NRRectL bbox;
    NRRectL visible;
    NRRectL clip;
    
    g_debug("request redraw rect %d %d %d %d", x0, y0, x1 - x0, y1 - y0);
    
    if ((x0 >= x1) || (y0 >= y1)) return;
    
    bbox.x0 = x0;
    bbox.y0 = y0;
    bbox.x1 = x1;
    bbox.y1 = y1;
    
#ifdef COCOA_CANVAS_USE_BUFFER
    Geom::Rect allvis = getViewbox();
    visible.x0 = allvis.min()[Geom::X];
    visible.y0 = allvis.min()[Geom::Y];
    visible.x1 = allvis.max()[Geom::X];
    visible.y1 = allvis.max()[Geom::Y];
#else
    visible.x0 = left();
    visible.y0 = top();
    visible.x1 = right();
    visible.y1 = bottom();
#endif
    
    nr_rect_l_intersect (&clip, &bbox, &visible);
    
    dirty_rect(clip.x0, clip.y0, clip.x1, clip.y1);
    
//    [_idle_updater requestUpdateWhenIdle:nill];
    add_idle(this);
//    [_ui requestUpdateRect:CGRectMake(clip.x0, clip.y0, clip.x1 - clip.x0, clip.y1 - clip.y0)];
}

Geom::Rect SPCocoaCanvas::getViewbox() const
{
    return Geom::Rect(Geom::Point(left(), top()), Geom::Point(right(), bottom()));
}

NR::IRect SPCocoaCanvas::getViewboxIntegers() const
{
    return NR::IRect(NR::IPoint(left(), top()), NR::IPoint(right(), bottom()));
}

void SPCocoaCanvas::get_dirty_rects(const CGRect **rects, int *count)
{
    Gdk::Region to_paint;
    
    for (int j=tTop; j<tBottom; j++) {
        for (int i=tLeft; i<tRight; i++) {
            int tile_index = (i - tLeft) + (j - tTop)*tileH;
            
            if ( tiles[tile_index] ) { // if this tile is dirtied (nonzero)
                to_paint.union_with_rect(Gdk::Rectangle(i*TILE_SIZE, j*TILE_SIZE,
                                                        TILE_SIZE, TILE_SIZE));
            }
            
        }
    }
    
    if (!to_paint.empty()) {
        Glib::ArrayHandle<Gdk::Rectangle> rect = to_paint.get_rectangles();
        
        if (rect.size() > 0) {
            CGRect *arr = new CGRect[rect.size()];
            int cnt = 0;
            
            typedef Glib::ArrayHandle<Gdk::Rectangle>::const_iterator Iter;
            for (Iter i=rect.begin(); i != rect.end(); ++i) {
                int x0 = (*i).get_x();
                int y0 = (*i).get_y();
                int x1 = x0 + (*i).get_width();
                int y1 = y0 + (*i).get_height();
                
                g_assert(cnt < rect.size());
                CGRect r = CGRectMake(x0, y0, x1 - x0, y1 - y0);
                
                if (_ui.view) {
                    if (CGRectIntersectsRect(r, _ui.view.visibleRect)) {
                        r = CGRectIntersection(r, _ui.view.visibleRect);
                        arr[cnt++] = r;
                    }
                } else {
                    arr[cnt++] = r;
                }
            }
            
            *count = cnt;
            *rects = (const CGRect *)arr;
        } else {
            *count = 0;
            *rects = NULL;
        }
    }
}

void SPCocoaCanvas::paint_single_buffer (int x0, int y0, int x1, int y1, int draw_x1, int draw_y1, int draw_x2, int draw_y2, int sw)
{
    SPCanvasBuf buf;
    
#ifdef COCOA_CANVAS_USE_BUFFER
    g_assert(_image_buffer);
#endif
    
    g_debug("really draw rect : %d %d %d %d", x0, y0, x1 - x0, y1 - y0);
    
    // Mark the region clean
    mark_rect(x0, y0, x1, y1, 0);
    
    buf.rect.x0 = x0;
    buf.rect.y0 = y0;
    buf.rect.x1 = x1;
    buf.rect.y1 = y1;
    buf.visible_rect.x0 = draw_x1;
    buf.visible_rect.y0 = draw_y1;
    buf.visible_rect.x1 = draw_x2;
    buf.visible_rect.y1 = draw_y2;
    
#ifdef COCOA_CANVAS_USE_BUFFER
    // Find the optimal buffer dimensions
    int vx = x0 - _location[Geom::X];
    int vy = y0 - _location[Geom::Y];
    buf.buf = _image_buffer.bitmapData + vy * _image_buffer.bytesPerRow + vx * 4;
    buf.buf_rowstride = (int)_image_buffer.bytesPerRow;
#else
    if (rendermode != Inkscape::RENDERMODE_OUTLINE) {
        buf.buf = nr_pixelstore_256K_new (FALSE, 0);
    } else {
        buf.buf = nr_pixelstore_1M_new (FALSE, 0);
    }
    buf.buf_rowstride = sw * 4;
#endif
    
    buf.bg_color = 0xffffffff;
    buf.is_empty = true;
    buf.ct = nr_create_cairo_context_canvasbuf (&(buf.visible_rect), &buf);
    
    if (SP_CANVAS_ITEM(root())->flags & SP_CANVAS_ITEM_VISIBLE) {
#if 0
    int i = rand() % 10000;
        while (i--) {
            int j = rand() % 1000;
            while (j--) {
                int m = 2932 * j;
                (void)m;
            }
        }
#elif 0
        int j = rand() % 20;
        while(j--) {
        for (int i = 0; i < (y1 - y0) * buf.buf_rowstride; i++) {
            char c = rand() % 0xff;
            buf.buf[i] = c;
        }
        }
        buf.is_empty = false;
#else
        SP_CANVAS_ITEM_GET_CLASS (root())->render (SP_CANVAS_ITEM(root()), &buf);
#endif
    }
    
#undef HAVE_LIBLCMS1
#undef HAVE_LIBLCMS2
    
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    cmsHTRANSFORM transf = 0;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool fromDisplay = prefs->getBool( "/options/displayprofile/from_display");
    if ( fromDisplay ) {
        transf = Inkscape::CMSSystem::getDisplayPer( *(cms_key) );
    } else {
        transf = Inkscape::CMSSystem::getDisplayTransform();
    }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    
    NSGraphicsContext *gc = [NSGraphicsContext currentContext];
    int width = x1 - x0, height = y1 - y0;
    CGPoint pos = [_ui convertPointFromCanvas:CGPointMake(x0, y0)];
    
    if (buf.is_empty) {
//        g_warning("Draw result is a empty buffer !");
        
#ifndef COCOA_CANVAS_USE_BUFFER
        [gc setCompositingOperation:NSCompositeClear];
        [[NSColor whiteColor] setFill];
        NSRectFill(CGRectMake(pos.x, pos.y, width, height));
#endif
    } else {
        
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        if ( transf && enable_cms_display_adj ) {
            for ( gint yy = 0; yy < (y1 - y0); yy++ ) {
                guchar* p = buf.buf + (buf.buf_rowstride * yy);
                Inkscape::CMSSystem::doTransform( transf, p, p, (x1 - x0) );
            }
        }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

#ifndef COCOA_CANVAS_USE_BUFFER
        unsigned char *planes[1] = {buf.buf};
        NSBitmapImageRep *bi = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: planes
                                                                       pixelsWide: width
                                                                       pixelsHigh: height
                                                                    bitsPerSample: 8
                                                                  samplesPerPixel: 4
                                                                         hasAlpha: YES
                                                                         isPlanar: NO
                                                                   colorSpaceName: NSCalibratedRGBColorSpace
                                                                     bitmapFormat: 0
                                                                      bytesPerRow: buf.buf_rowstride
                                                                     bitsPerPixel: 32];
//        [SWImageTools flipImageVertical:bi];
        [gc setCompositingOperation:NSCompositeCopy];
        [bi drawInRect:CGRectMake(pos.x, pos.y, width, height)
              fromRect:CGRectMake(0, 0, width, height)
             operation:NSCompositeCopy
              fraction:1.0
        respectFlipped:TRUE
                 hints:nill];
//        [bi drawAtPoint:pos];
        
//        [[NSColor redColor] setFill];
//        NSFrameRectWithWidth(CGRectMake(pos.x, pos.y, width, height), 0.2f);
#endif
    }
    
    cairo_surface_t *cst = cairo_get_target(buf.ct);
    cairo_destroy (buf.ct);
    cairo_surface_flush (cst);
    cairo_surface_destroy (cst);
    
#ifndef COCOA_CANVAS_USE_BUFFER
    if (rendermode != Inkscape::RENDERMODE_OUTLINE) {
        nr_pixelstore_256K_free (buf.buf);
    } else {
        nr_pixelstore_1M_free (buf.buf);
    }
#endif
}

/**
 * Paint the given rect, recursively subdividing the region until it is the size of a single
 * buffer.
 *
 * @return true if the drawing completes
 */
int SPCocoaCanvas::paint_rect_internal (PaintRectSetup const *setup, NRRectL this_rect)
{
    GTimeVal now;
    g_get_current_time (&now);
    
    glong elapsed = (now.tv_sec - setup->start_time.tv_sec) * 1000000
    + (now.tv_usec - setup->start_time.tv_usec);
    
    // Allow only very fast buffers to be run together;
    // as soon as the total redraw time exceeds 1ms, cancel;
    // this returns control to the idle loop and allows Inkscape to process user input
    // (potentially interrupting the redraw); as soon as Inkscape has some more idle time,
    // it will get back and finish painting what remains to paint.
    if (elapsed > 1000) {
        // Interrupting redraw isn't always good.
        // For example, when you drag one node of a big path, only the buffer containing
        // the mouse cursor will be redrawn again and again, and the rest of the path
        // will remain stale because Inkscape never has enough idle time to redraw all
        // of the screen. To work around this, such operations set a forced_redraw_limit > 0.
        // If this limit is set, and if we have aborted redraw more times than is allowed,
        // interrupting is blocked and we're forced to redraw full screen once
        // (after which we can again interrupt forced_redraw_limit times).
        if (setup->canvas->forced_redraw_limit < 0 ||
            setup->canvas->forced_redraw_count < setup->canvas->forced_redraw_limit) {
            
            if (setup->canvas->forced_redraw_limit != -1) {
                setup->canvas->forced_redraw_count++;
            }
            
//            g_warning("refuse redraw : last too long : %ld %ld", setup->start_time.tv_sec, elapsed);
            paint_interrupted = true;
            return false;
        }
    }
    
    // Find the optimal buffer dimensions
    int bw = this_rect.x1 - this_rect.x0;
    int bh = this_rect.y1 - this_rect.y0;
    if ((bw < 1) || (bh < 1))
        return 1;
    
    if (bw * bh < setup->max_pixels) {
        // We are small enough
        paint_single_buffer (this_rect.x0, this_rect.y0,
                             this_rect.x1, this_rect.y1,
                             setup->big_rect.x0, setup->big_rect.y0,
                             setup->big_rect.x1, setup->big_rect.y1, bw);
        return 1;
    }
    
    NRRectL lo = this_rect;
    NRRectL hi = this_rect;
    
    /*
     This test determines the redraw strategy:
     
     bw < bh (strips mode) splits across the smaller dimension of the rect and therefore (on
     horizontally-stretched windows) results in redrawing in horizontal strips (from cursor point, in
     both directions if the cursor is in the middle). This is traditional for Inkscape since old days,
     and seems to be faster for drawings with many smaller objects at zoom-out.
     
     bw > bh (chunks mode) splits across the larger dimension of the rect and therefore paints in
     almost-square chunks, again from the cursor point. It's sometimes faster for drawings with few slow
     (e.g. blurred) objects crossing the entire screen. It also appears to be somewhat psychologically
     faster.
     
     The default for now is the strips mode.
     */
    if (bw < bh || bh < 2 * TILE_SIZE) {
        // to correctly calculate the mean of two ints, we need to sum them into a larger int type
        gint32 mid = (gint32)((long long) this_rect.x0 + (long long) this_rect.x1) / 2;
        // Make sure that mid lies on a tile boundary
        mid = (mid / TILE_SIZE) * TILE_SIZE;
        
        lo.x1 = mid;
        hi.x0 = mid;
        
        if (setup->mouse_loc[Geom::X] < mid) {
            // Always paint towards the mouse first
            return paint_rect_internal(setup, lo)
            && paint_rect_internal(setup, hi);
        } else {
            return paint_rect_internal(setup, hi)
            && paint_rect_internal(setup, lo);
        }
    } else {
        // to correctly calculate the mean of two ints, we need to sum them into a larger int type
        gint32 mid = (gint32)((long long) this_rect.y0 + (long long) this_rect.y1) / 2;
        // Make sure that mid lies on a tile boundary
        mid = (mid / TILE_SIZE) * TILE_SIZE;
        
        lo.y1 = mid;
        hi.y0 = mid;
        
        if (setup->mouse_loc[Geom::Y] < mid) {
            // Always paint towards the mouse first
            return paint_rect_internal(setup, lo)
            && paint_rect_internal(setup, hi);
        } else {
            return paint_rect_internal(setup, hi)
            && paint_rect_internal(setup, lo);
        }
    }
    
    return 1;
}

bool SPCocoaCanvas::paint_rect(SPTilesCanvas */*canvas*/, int xx0, int yy0, int xx1, int yy1)
{
    if (need_update) {
//        return false;
        g_warning("need to update before paint");
    }
    
#if !defined(COCOA_CANVAS_USE_BUFFER) && !defined(COCOA_CANVAS_PAINT_WITH_NO_LOCKFOCUS)
    if (![_ui.view lockFocusIfCanDraw]) {
        return false;
    }
#endif
    
    NRRectL visible, bbox, clip;
#ifdef COCOA_CANVAS_USE_BUFFER
    visible.x0 = _location[Geom::X];
    visible.y0 = _location[Geom::Y];
    visible.x1 = visible.x0 + _allocateSize.width;
    visible.y1 = visible.y0 + _allocateSize.height;
#else
    visible.x0 = left();
    visible.y0 = top();
    visible.x1 = right();
    visible.y1 = bottom();
#endif
    bbox.x0 = xx0;
    bbox.y0 = yy0;
    bbox.x1 = xx1;
    bbox.y1 = yy1;
    
    nr_rect_l_intersect (&clip, &bbox, &visible);
    
#ifdef COCOA_CANVAS_USE_BUFFER
    double wx = 0, wy = 0;
    world_to_window(clip.x0, clip.y0, &wx, &wy);
    [_ui didCanvasUpdateRect:CGRectMake(wx, wy, clip.x1 - clip.x0, clip.y1 - clip.y0)];
#endif
    
#ifdef DEBUG_REDRAW
    // paint the area to redraw yellow
    gdk_rgb_gc_set_foreground (_pixmap_gc, 0xFFFF00);
    gdk_draw_rectangle (SP_GTK_CANVAS_WINDOW (this),
                        _pixmap_gc,
                        TRUE,
                        rect.x0, rect.y0,
                        rect.x1 - rect.x0, rect.y1 - rect.y0);
#endif
    
    PaintRectSetup setup;
    
    setup.canvas = this;
    setup.big_rect = /*clip*/bbox;
    
    // Save the mouse location
    CGPoint loc = [_ui mouseLocation];
    setup.mouse_loc = Geom::Point(loc.x, loc.y);
    
    if (rendermode != Inkscape::RENDERMODE_OUTLINE) {
        // use 256K as a compromise to not slow down gradients
        // 256K is the cached buffer and we need 4 channels
        setup.max_pixels = 65536; // 256K/4
    } else {
        // paths only, so 1M works faster
        // 1M is the cached buffer and we need 4 channels
        setup.max_pixels = 262144;
    }
    
    // Start the clock
    g_get_current_time(&(setup.start_time));
    
    bool ret = paint_rect_internal(&setup, /*clip*/bbox);
    
#if !defined(COCOA_CANVAS_USE_BUFFER) && !defined(COCOA_CANVAS_PAINT_WITH_NO_LOCKFOCUS)
    [_ui.view unlockFocus];
    [_ui.view.window flushWindow];
#endif
    
    // Go
    return ret;
}

bool SPCocoaCanvas::on_expose_event(GdkEventExpose* event)
{
    int n_rects;
    GdkRectangle *rects;
    gdk_region_get_rectangles (event->region, &rects, &n_rects);
    
    for (int i = 0; i < n_rects; i++) {
        NRRectL rect;
        
        rect.x0 = rects[i].x;
        rect.y0 = rects[i].y;
        rect.x1 = rect.x0 + rects[i].width;
        rect.y1 = rect.y0 + rects[i].height;
        
        request_redraw (rect.x0, rect.y0, rect.x1, rect.y1);
    }
    
    if (n_rects > 0)
        g_free (rects);
    
    return FALSE;
}

void SPCocoaCanvas::on_size_allocate(GtkAllocation& allocation)
{
    if (left() == allocation.x
        && top() == allocation.y
        && width() == allocation.width
        && height() == allocation.height) {
        return;
    }
    
    left() = allocation.x;
    top() = allocation.y;
    width() = allocation.width;
    height() = allocation.height;
    
    g_debug("allocate location = (%.0f %.0f) size = (%.0f %.0f)", left(), top(), width(), height());
    
#ifdef COCOA_CANVAS_USE_BUFFER
    _scroll_buffer = [SWImageTools createBitmapImageWithSize:_allocateSize];
    
    if (_image_buffer) {
        [SWImageTools drawToImage:_scroll_buffer
                        fromImage:_image_buffer
                          atPoint:CGPointMake(0, _allocateSize.height - oldSize.height)
                  withComposition:NO];
    }
    
    _image_buffer = _scroll_buffer;
    _scroll_buffer = [SWImageTools createBitmapImageWithSize:_allocateSize];
#endif
    
    resize_tiles(left(), top(), right(), bottom());
    
#ifdef COCOA_CANVAS_USE_BUFFER
    /* Schedule redraw of new region */
    if (allocation.width > oldSize.width) {
        request_redraw (_location[Geom::X] + oldSize.width,
                        _location[Geom::Y],
                        _location[Geom::X] + allocation.width,
                        _location[Geom::Y] + allocation.height);
    }
    if (allocation.height > oldSize.height) {
        request_redraw (_location[Geom::X],
                        _location[Geom::Y] + oldSize.height,
                        _location[Geom::X] + allocation.width,
                        _location[Geom::Y] + allocation.height);
    }

    if (_ui) {
        [_ui didCanvasUpdateRect:CGRectMake(0, 0, _allocateSize.width, _allocateSize.height)];
    }
#endif
}

/**
 * Button event handler for the canvas.
 */
gint SPCocoaCanvas::do_button_event (GdkEventButton *event)
{
    SPCocoaCanvas *canvas = this;
    
    int retval = FALSE;
    
    int mask;
    switch (event->button) {
        case 1:
            mask = GDK_BUTTON1_MASK;
            break;
        case 2:
            mask = GDK_BUTTON2_MASK;
            break;
        case 3:
            mask = GDK_BUTTON3_MASK;
            break;
        case 4:
            mask = GDK_BUTTON4_MASK;
            break;
        case 5:
            mask = GDK_BUTTON5_MASK;
            break;
        default:
            mask = 0;
    }
    
    switch (event->type) {
        case GDK_BUTTON_PRESS:
        case GDK_2BUTTON_PRESS:
        case GDK_3BUTTON_PRESS:
            /* Pick the current item as if the button were not pressed, and
             * then process the event.
             */
            canvas->state = event->state;
            pick_current_item ((GdkEvent *) event);
            canvas->state ^= mask;
            retval = emit_event ((GdkEvent *) event);
            break;
            
        case GDK_BUTTON_RELEASE:
            /* Process the event as if the button were pressed, then repick
             * after the button has been released
             */
            canvas->state = event->state;
            retval = emit_event ((GdkEvent *) event);
            event->state ^= mask;
            canvas->state = event->state;
            pick_current_item ((GdkEvent *) event);
            event->state ^= mask;
            
            break;
            
        default:
            g_assert_not_reached ();
    }
    
    return retval;
}

bool SPCocoaCanvas::on_button_press_event(GdkEventButton* event)
{
    return do_button_event(event);
}

bool SPCocoaCanvas::on_button_release_event(GdkEventButton* event)
{
    return do_button_event(event);
}

bool SPCocoaCanvas::on_motion_notify_event(GdkEventMotion* event)
{
    int status;
    
    state = event->state;
    pick_current_item ((GdkEvent *) event);
    status = emit_event ((GdkEvent *) event);
    
    return status;
}

/**
 * Crossing event handler for the canvas.
 */
gint SPCocoaCanvas::do_crossing_event (GdkEventCrossing *event)
{
    state = event->state;
    return pick_current_item ((GdkEvent *) event);
}

bool SPCocoaCanvas::on_enter_notify_event(GdkEventCrossing* event)
{
    return do_crossing_event(event);
}

bool SPCocoaCanvas::on_leave_notify_event(GdkEventCrossing* event)
{
    return do_crossing_event(event);
}

gint SPCocoaCanvas::do_key_event(GdkEventKey *event)
{
    return emit_event ((GdkEvent *) event);
}

bool SPCocoaCanvas::on_key_press_event(GdkEventKey* event)
{
    return do_key_event(event);
}

bool SPCocoaCanvas::on_key_release_event(GdkEventKey* event)
{
    return do_key_event(event);
}

/*
 * Removes the transient state of the canvas (idle handler, grabs).
 */
void SPCocoaCanvas::shutdown_transients ()
{
    /* We turn off the need_redraw flag, since if the canvas is mapped again
     * it will request a redraw anyways.  We do not turn off the need_update
     * flag, though, because updates are not queued when the canvas remaps
     * itself.
     */
    mark_rect(left(), right(), width(), height(), 0);
    
    if (grabbed_item) {
        grabbed_item = NULL;
//        gdk_pointer_ungrab (GDK_CURRENT_TIME);
    }
    
//    [_idle_updater removeRequest];
    remove_idle(this);
}

/**
 * Sets world coordinates from win and canvas.
 */
void SPCocoaCanvas::window_to_world(double winx, double winy, double *worldx, double *worldy)
{
    if (worldx) *worldx = left() + winx;
    if (worldy) *worldy = top() + winy;
}

/**
 * Sets win coordinates from world and canvas.
 */
void SPCocoaCanvas::world_to_window(double worldx, double worldy, double *winx, double *winy)
{
    if (winx) *winx = worldx - left();
    if (winy) *winy = worldy - top();
}

/**
 * Converts point from win to world coordinates.
 */
Geom::Point SPCocoaCanvas::window_to_world(Geom::Point const win)
{
    double x, y;
    window_to_world(win[0], win[1], &x, &y);
    return Geom::Point(x, y);
}

/**
 * Converts point from world to win coordinates.
 */
Geom::Point SPCocoaCanvas::world_to_window(Geom::Point const world)
{
    double x, y;
    world_to_window(world[0], world[1], &x, &y);
    return Geom::Point(x, y);
}


#if 0
static GObjectClass *parent_class = NULL;

/**
 * Callback: object initialization for SPCanvas.
 */
static void sp_cocoa_canvas_init (SPCocoaCanvas *canvas)
{
    canvas->init();
}

static void sp_cocoa_canvas_finalize (GObject *object)
{
    SPCocoaCanvas *canvas = (SPCocoaCanvas *)object;
    canvas->shutdown_transients ();
    canvas->destroy();
    if (parent_class->finalize) {
        parent_class->finalize(G_OBJECT(canvas));
    }
}

/**
 * Initializes the SPObject vtable.
 */
static void sp_cocoa_canvas_class_init(SPCocoaCanvas *klass)
{
    GObjectClass *object_class;
    object_class = (GObjectClass *) klass;
    parent_class = (GObjectClass *) g_type_class_ref(GTK_TYPE_WIDGET);
    object_class->finalize = sp_cocoa_canvas_finalize;
}

/**
 * Registers the SPCanvas class if necessary, and returns the type ID
 * associated to it.
 *
 * \return The type ID of the SPCanvas class.
 **/
GType sp_cocoa_canvas_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPCocoaCanvasClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_cocoa_canvas_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPCocoaCanvas),
            0, // n_preallocs
            (GInstanceInitFunc)sp_cocoa_canvas_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_WIDGET, "SPCocoaCanvas", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}


SPCocoaCanvas *sp_cocoa_canvas_new()
{
    return (SPCocoaCanvas *)gtk_type_new(sp_cocoa_canvas_get_type());
}
#endif
