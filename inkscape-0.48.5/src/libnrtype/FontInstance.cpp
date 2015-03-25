/*
 *  FontInstance.cpp
 *  testICU
 *
 *   Authors:
 *     fred
 *     bulia byak <buliabyak@users.sf.net>
 *
 */

#define PANGO_ENABLE_ENGINE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ft2build.h>
#include FT_OUTLINE_H
#include FT_BBOX_H
#include FT_TRUETYPE_TAGS_H
#include FT_TRUETYPE_TABLES_H
#include <pango/pangoft2.h>
#include <2geom/pathvector.h>
#include "libnr/nr-rect.h"
#include "libnrtype/font-glyph.h"
#include "libnrtype/font-instance.h"
#include "libnrtype/RasterFont.h"
#include "livarot/Path.h"
#include "util/unordered-containers.h"

#ifdef USE_PANGO_OSX
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <pango/pangocoretext.h>
#endif

#if !PANGO_VERSION_CHECK(1,24,0)
#define PANGO_WEIGHT_THIN       static_cast<PangoWeight>(100)
#define PANGO_WEIGHT_BOOK       static_cast<PangoWeight>(380)
#define PANGO_WEIGHT_MEDIUM     static_cast<PangoWeight>(500)
#define PANGO_WEIGHT_ULTRAHEAVY static_cast<PangoWeight>(1000)
#endif


struct font_style_hash : public std::unary_function<font_style, size_t> {
    size_t operator()(font_style const &x) const;
};

struct font_style_equal : public std::binary_function<font_style, font_style, bool> {
    bool operator()(font_style const &a, font_style const &b) const;
};

typedef INK_UNORDERED_MAP<font_style, raster_font*, font_style_hash, font_style_equal> StyleMap;

static const double STROKE_WIDTH_THREASHOLD = 0.01;



size_t font_style_hash::operator()(const font_style &x) const {
    int h = 0;
    int n = static_cast<int>(floor(100 * x.stroke_width));
    h *= 12186;
    h += n;
    n = (x.vertical) ? 1:0;
    h *= 12186;
    h += n;
    if ( x.stroke_width >= STROKE_WIDTH_THREASHOLD ) {
        n = x.stroke_cap * 10 + x.stroke_join + static_cast<int>(x.stroke_miter_limit * 100);
        h *= 12186;
        h += n;
        if ( x.nbDash > 0 ) {
            n = x.nbDash;
            h *= 12186;
            h += n;
            n = static_cast<int>(floor(100 * x.dash_offset));
            h *= 12186;
            h += n;
            for (int i = 0; i < x.nbDash; i++) {
                n = static_cast<int>(floor(100 * x.dashes[i]));
                h *= 12186;
                h += n;
            }
        }
    }
    return h;
}

bool font_style_equal::operator()(const font_style &a,const font_style &b) const {
    bool same = true;
    for (int i = 0; (i < 6) && same; i++) {
        same = ( static_cast<int>(100 * a.transform[i]) == static_cast<int>(100 * b.transform[i]) );
    }
    same &= ( a.vertical == b.vertical )
        && ( a.stroke_width > STROKE_WIDTH_THREASHOLD ) == ( b.stroke_width > STROKE_WIDTH_THREASHOLD );
    if ( same && ( a.stroke_width > STROKE_WIDTH_THREASHOLD ) ) {
        same = ( a.stroke_cap == b.stroke_cap )
            && ( a.stroke_join == b.stroke_join )
            && ( static_cast<int>(a.stroke_miter_limit * 100) == static_cast<int>(b.stroke_miter_limit * 100) )
            && ( a.nbDash == b.nbDash );
        if ( same && ( a.nbDash > 0 ) ) {
            same = ( static_cast<int>(floor(100 * a.dash_offset)) == static_cast<int>(floor(100 * b.dash_offset)) );
            for (int i = 0; (i < a.nbDash) && same; i++) {
                same = ( static_cast<int>(floor(100 * a.dashes[i])) == static_cast<int>(floor(100 * b.dashes[i])) );
            }
        }
    }
    return same;
}

#ifdef USE_PANGO_FT
/*
 * Outline extraction
 */
typedef struct ft2_to_liv {
    Path*        theP;
    double       scale;
    Geom::Point  last;
} ft2_to_liv;

// Note: Freetype 2.2.1 redefined function signatures for functions to be placed in an
// FT_Outline_Funcs structure.  This is needed to keep backwards compatibility with the
// 2.1.x series.

/* *** BEGIN #if HACK *** */
#if FREETYPE_MAJOR == 2 && FREETYPE_MINOR >= 2
typedef FT_Vector const FREETYPE_VECTOR;
#else
typedef FT_Vector FREETYPE_VECTOR;
#endif

// outline as returned by freetype -> livarot Path
// see nr-type-ft2.cpp for the freetype -> artBPath on which this code is based
static int ft2_move_to(FREETYPE_VECTOR *to, void * i_user) {
    ft2_to_liv* user=(ft2_to_liv*)i_user;
    Geom::Point p(user->scale*to->x,user->scale*to->y);
    //    printf("m  t=%f %f\n",p[0],p[1]);
    user->theP->MoveTo(p);
    user->last=p;
    return 0;
}

static int ft2_line_to(FREETYPE_VECTOR *to, void *i_user)
{
    ft2_to_liv* user=(ft2_to_liv*)i_user;
    Geom::Point p(user->scale*to->x,user->scale*to->y);
    //    printf("l  t=%f %f\n",p[0],p[1]);
    user->theP->LineTo(p);
    user->last=p;
    return 0;
}

static int ft2_conic_to(FREETYPE_VECTOR *control, FREETYPE_VECTOR *to, void *i_user)
{
    ft2_to_liv* user=(ft2_to_liv*)i_user;
    Geom::Point p(user->scale*to->x,user->scale*to->y),c(user->scale*control->x,user->scale*control->y);
    //    printf("b c=%f %f  t=%f %f\n",c[0],c[1],p[0],p[1]);
    user->theP->BezierTo(p);
    user->theP->IntermBezierTo(c);
    user->theP->EndBezierTo();
    user->last=p;
    return 0;
}

static int ft2_cubic_to(FREETYPE_VECTOR *control1, FREETYPE_VECTOR *control2, FREETYPE_VECTOR *to, void *i_user)
{
    ft2_to_liv* user=(ft2_to_liv*)i_user;
    Geom::Point p(user->scale*to->x,user->scale*to->y);
    Geom::Point c1(user->scale*control1->x,user->scale*control1->y);
    Geom::Point c2(user->scale*control2->x,user->scale*control2->y);
    //    printf("c c1=%f %f  c2=%f %f   t=%f %f\n",c1[0],c1[1],c2[0],c2[1],p[0],p[1]);
    user->theP->CubicTo(p,3*(c1-user->last),3*(p-c2));
    user->last=p;
    return 0;
}
#endif

/* *** END #if HACK *** */

/*
 *
 */

font_instance::font_instance(void) :
    pFont(0),
    descr(0),
    refCount(0),
    daddy(0),
    nbGlyph(0),
    maxGlyph(0),
    glyphs(0),
    loadedPtr(new StyleMap()),
    theFace(0)
{
    //printf("font instance born\n");
}

font_instance::~font_instance(void)
{
    if ( loadedPtr ) {
        StyleMap* tmp = static_cast<StyleMap*>(loadedPtr);
        delete tmp;
        loadedPtr = 0;
    }

    if ( daddy ) {
        daddy->UnrefFace(this);
        daddy = 0;
    }

    //printf("font instance death\n");
    if ( pFont ) {
        g_object_unref(pFont);
        pFont = 0;
    }

    if ( descr ) {
        pango_font_description_free(descr);
        descr = 0;
    }

    //    if ( theFace ) FT_Done_Face(theFace); // owned by pFont. don't touch
    theFace = 0;

    for (int i=0;i<nbGlyph;i++) {
        if ( glyphs[i].outline ) {
            delete glyphs[i].outline;
        }
        if ( glyphs[i].pathvector ) {
            delete glyphs[i].pathvector;
        }
    }
    if ( glyphs ) {
        free(glyphs);
        glyphs = 0;
    }
    nbGlyph = 0;
    maxGlyph = 0;
}

void font_instance::Ref(void)
{
    refCount++;
    //char *tc=pango_font_description_to_string(descr);
    //printf("font %x %s ref'd %i\n",this,tc,refCount);
    //free(tc);
}

void font_instance::Unref(void)
{
    refCount--;
    //char *tc=pango_font_description_to_string(descr);
    //printf("font %x %s unref'd %i\n",this,tc,refCount);
    //free(tc);
    if ( refCount <= 0 ) {
        if ( daddy ) {
            daddy->UnrefFace(this);
        }
        daddy=NULL;
        delete this;
    }
}

unsigned int font_instance::Name(gchar *str, unsigned int size)
{
    return Attribute("name", str, size);
}

unsigned int font_instance::Family(gchar *str, unsigned int size)
{
    return Attribute("family", str, size);
}

unsigned int font_instance::PSName(gchar *str, unsigned int size)
{
    return Attribute("psname", str, size);
}

unsigned int font_instance::Attribute(const gchar *key, gchar *str, unsigned int size)
{
    if ( descr == NULL ) {
        if ( size > 0 ) {
            str[0]=0;
        }
        return 0;
    }
    char*   res=NULL;
    bool    free_res=false;

    if ( strcmp(key,"name") == 0 ) {
        PangoFontDescription* td=pango_font_description_copy(descr);
        pango_font_description_unset_fields (td, PANGO_FONT_MASK_SIZE);
        res=pango_font_description_to_string (td);
        pango_font_description_free(td);
        free_res=true;
    } else if ( strcmp(key,"psname") == 0 ) {
#ifdef USE_PANGO_FT
         res = (char *) FT_Get_Postscript_Name (theFace); // that's the main method, seems to always work
#endif
         free_res=false;
         if (res == NULL) { // a very limited workaround, only bold, italic, and oblique will work
             PangoStyle style=pango_font_description_get_style(descr);
             bool i = (style == PANGO_STYLE_ITALIC);
             bool o = (style == PANGO_STYLE_OBLIQUE);
             PangoWeight weight=pango_font_description_get_weight(descr);
             bool b = (weight >= PANGO_WEIGHT_BOLD);

             res = g_strdup_printf ("%s%s%s%s",
                                    pango_font_description_get_family(descr),
                                    (b || i || o) ? "-" : "",
                                    (b) ? "Bold" : "",
                                    (i) ? "Italic" : ((o) ? "Oblique" : "")  );
             free_res = true;
         }
    } else if ( strcmp(key,"family") == 0 ) {
        res=(char*)pango_font_description_get_family(descr);
        free_res=false;
    } else if ( strcmp(key,"style") == 0 ) {
        PangoStyle v=pango_font_description_get_style(descr);
        if ( v == PANGO_STYLE_ITALIC ) {
            res=(char*)"italic";
        } else if ( v == PANGO_STYLE_OBLIQUE ) {
            res=(char*)"oblique";
        } else {
            res=(char*)"normal";
        }
        free_res=false;
    } else if ( strcmp(key,"weight") == 0 ) {
        PangoWeight v=pango_font_description_get_weight(descr);
        if ( v <= PANGO_WEIGHT_THIN ) {
            res=(char*)"100";
        } else if ( v <= PANGO_WEIGHT_ULTRALIGHT ) {
            res=(char*)"200";
        } else if ( v <= PANGO_WEIGHT_LIGHT ) {
            res=(char*)"300";
        } else if ( v <= PANGO_WEIGHT_BOOK ) {
            res=(char*)"380";
        } else if ( v <= PANGO_WEIGHT_NORMAL ) {
            res=(char*)"normal";
        } else if ( v <= PANGO_WEIGHT_MEDIUM ) {
            res=(char*)"500";
        } else if ( v <= PANGO_WEIGHT_SEMIBOLD ) {
            res=(char*)"600";
        } else if ( v <= PANGO_WEIGHT_BOLD ) {
            res=(char*)"bold";
        } else if ( v <= PANGO_WEIGHT_ULTRABOLD ) {
            res=(char*)"800";
        } else { // HEAVY   NB: Pango defines ULTRAHEAVY = 1000 but not CSS2
            res=(char*)"900";
        }
        free_res=false;
    } else if ( strcmp(key,"stretch") == 0 ) {
        PangoStretch v=pango_font_description_get_stretch(descr);
        if ( v <= PANGO_STRETCH_EXTRA_CONDENSED ) {
            res=(char*)"extra-condensed";
        } else if ( v <= PANGO_STRETCH_CONDENSED ) {
            res=(char*)"condensed";
        } else if ( v <= PANGO_STRETCH_SEMI_CONDENSED ) {
            res=(char*)"semi-condensed";
        } else if ( v <= PANGO_STRETCH_NORMAL ) {
            res=(char*)"normal";
        } else if ( v <= PANGO_STRETCH_SEMI_EXPANDED ) {
            res=(char*)"semi-expanded";
        } else if ( v <= PANGO_STRETCH_EXPANDED ) {
            res=(char*)"expanded";
        } else {
            res=(char*)"extra-expanded";
        }
        free_res=false;
    } else if ( strcmp(key,"variant") == 0 ) {
        PangoVariant v=pango_font_description_get_variant(descr);
        if ( v == PANGO_VARIANT_SMALL_CAPS ) {
            res=(char*)"small-caps";
        } else {
            res=(char*)"normal";
        }
        free_res=false;
    } else {
        res = NULL;
        free_res=false;
    }
    if ( res == NULL ) {
        if ( size > 0 ) {
            str[0] = 0;
        }
        return 0;
    }

    if (res) {
        unsigned int len=strlen(res);
        unsigned int rlen=(size-1<len)?size-1:len;
        if ( str ) {
            if ( rlen > 0 ) {
                memcpy(str, res, rlen);
            }
            if ( size > 0 ) {
                str[rlen] = 0;
            }
        }
        if (free_res) {
            g_free(res);
        }
        return len;
    }
    return 0;
}

void font_instance::InitTheFace()
{
#ifdef USE_PANGO_WIN32
    if ( !theFace ) {
        LOGFONT *lf=pango_win32_font_logfont(pFont);
        g_assert(lf != NULL);
        theFace=pango_win32_font_cache_load(daddy->pangoFontCache,lf);
        g_free(lf);
    }
    XFORM identity = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
    SetWorldTransform(daddy->hScreenDC, &identity);
    SetGraphicsMode(daddy->hScreenDC, GM_COMPATIBLE);
    SelectObject(daddy->hScreenDC,theFace);
#elif defined(USE_PANGO_OSX)
    if (!theFace) {
        theFace = (void*)pango_core_text_font_get_ctfont((PangoCoreTextFont *)pFont);
    }
#else
    theFace=pango_ft2_font_get_face(pFont); // Deprecated, use pango_fc_font_lock_face() instead
    if ( theFace ) {
        FT_Select_Charmap(theFace,ft_encoding_unicode) && FT_Select_Charmap(theFace,ft_encoding_symbol);
    }
#endif
}

void font_instance::FreeTheFace()
{
#ifdef USE_PANGO_WIN32
    SelectObject(daddy->hScreenDC,GetStockObject(SYSTEM_FONT));
    pango_win32_font_cache_unload(daddy->pangoFontCache,theFace);
#elif defined(USE_PANGO_OSX)
#endif
    theFace=NULL;
}

void font_instance::InstallFace(PangoFont* iFace)
{
    if ( !iFace ) {
        return;
    }
    pFont=iFace;

    InitTheFace();

    if ( pFont && IsOutlineFont() == false ) {
        FreeTheFace();
        if ( pFont ) {
            g_object_unref(pFont);
        }
        pFont=NULL;
    }
}

bool font_instance::IsOutlineFont(void)
{
    if ( pFont == NULL ) {
        return false;
    }
    InitTheFace();
#ifdef USE_PANGO_WIN32
    TEXTMETRIC tm;
    return GetTextMetrics(daddy->hScreenDC,&tm) && tm.tmPitchAndFamily&(TMPF_TRUETYPE|TMPF_DEVICE);
#elif defined(USE_PANGO_OSX)
    return true;
#else
    return FT_IS_SCALABLE(theFace);
#endif
}

int font_instance::MapUnicodeChar(gunichar c)
{
    int res = 0;
    if ( pFont  ) {
#ifdef USE_PANGO_WIN32
        res = pango_win32_font_get_glyph_index(pFont, c);
#elif defined(USE_PANGO_OSX)
        theFace = (void*)pango_core_text_font_get_ctfont((PangoCoreTextFont *)pFont);
        const UniChar chs[1] = {static_cast<UniChar>(c)};
        CGGlyph glyphs[1] = {0};
        if (CTFontGetGlyphsForCharacters((CTFontRef)theFace, chs, glyphs, 1)) {
            res = glyphs[0];
        }
#else
        theFace = pango_ft2_font_get_face(pFont);
        if ( c > 0xf0000 ) {
            res = CLAMP(c, 0xf0000, 0x1fffff) - 0xf0000;
        } else {
            res = FT_Get_Char_Index(theFace, c);
        }
#endif
    }
    return res;
}


#ifdef USE_PANGO_WIN32
static inline Geom::Point pointfx_to_nrpoint(const POINTFX &p, double scale)
{
    return Geom::Point(*(long*)&p.x / 65536.0 * scale,
                     *(long*)&p.y / 65536.0 * scale);
}
#endif


#ifdef USE_PANGO_OSX

typedef struct cg_to_liv {
    Path*        theP;
    double       scale;
    Geom::Point  last;
} ft2_to_liv;

void PathApplier( void *info, const CGPathElement *element )
{
    cg_to_liv *data = (cg_to_liv *)info;
    Path *path = data->theP;
    Geom::Point p, end;
    
    if (element->type != kCGPathElementCloseSubpath) {
        p = Geom::Point(element->points[0].x, element->points[0].y) * data->scale;
        g_debug("Path element point : [%.0f %.0f]", element->points[0].x, element->points[0].y);
    }
    
    switch (element->type) {
        case kCGPathElementMoveToPoint:
        {
            path->MoveTo(p);
            data->last = p;
        }
            break;
            
        case kCGPathElementAddLineToPoint:
        {
            path->LineTo(p);
            data->last = p;
        }
            break;
            
        case kCGPathElementAddQuadCurveToPoint:
        {
            Geom::Point c = p;
            end = Geom::Point(element->points[1].x, element->points[1].y) * data->scale;
            path->BezierTo(end);
            path->IntermBezierTo(c);
            path->EndBezierTo();
            data->last = end;
        }
            break;
        case kCGPathElementAddCurveToPoint:
        {
            Geom::Point c1 = p;
            Geom::Point c2 = Geom::Point(element->points[1].x, element->points[1].y) * data->scale;
            end = Geom::Point(element->points[2].x, element->points[2].y) * data->scale;
            //    printf("c c1=%f %f  c2=%f %f   t=%f %f\n",c1[0],c1[1],c2[0],c2[1],p[0],p[1]);
            path->CubicTo(end, 3 * (c1 - data->last), 3 * (end - c2));
            data->last = end;
        }
            break;
        
        case kCGPathElementCloseSubpath:
        {
            path->Close();
        }
            break;
            
        default:;
    }
}
#endif

void font_instance::LoadGlyph(int glyph_id)
{
    if ( pFont == NULL ) {
        return;
    }
    InitTheFace();
#ifdef USE_PANGO_FT
    if ( !FT_IS_SCALABLE(theFace) ) {
        return; // bitmap font
    }
#endif

    if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
        if ( nbGlyph >= maxGlyph ) {
            maxGlyph=2*nbGlyph+1;
            glyphs=(font_glyph*)realloc(glyphs,maxGlyph*sizeof(font_glyph));
        }
        font_glyph  n_g;
        n_g.outline=NULL;
        n_g.pathvector=NULL;
        n_g.bbox[0]=n_g.bbox[1]=n_g.bbox[2]=n_g.bbox[3]=0;
        bool   doAdd=false;

#ifdef USE_PANGO_WIN32

#ifndef GGO_UNHINTED         // For compatibility with old SDKs.
#define GGO_UNHINTED 0x0100
#endif

        MAT2 identity = {{0,1},{0,0},{0,0},{0,1}};
        OUTLINETEXTMETRIC otm;
        GetOutlineTextMetrics(daddy->hScreenDC, sizeof(otm), &otm);
        GLYPHMETRICS metrics;
        DWORD bufferSize=GetGlyphOutline (daddy->hScreenDC, glyph_id, GGO_GLYPH_INDEX | GGO_NATIVE | GGO_UNHINTED, &metrics, 0, NULL, &identity);
        double scale=1.0/daddy->fontSize;
        n_g.h_advance=metrics.gmCellIncX*scale;
        n_g.v_advance=otm.otmTextMetrics.tmHeight*scale;
        n_g.h_width=metrics.gmBlackBoxX*scale;
        n_g.v_width=metrics.gmBlackBoxY*scale;
        n_g.outline=NULL;
        if ( bufferSize == GDI_ERROR) {
            // shit happened
        } else if ( bufferSize == 0) {
            // character has no visual representation, but is valid (eg whitespace)
            doAdd=true;
        } else {
            char *buffer = new char[bufferSize];
            if ( GetGlyphOutline (daddy->hScreenDC, glyph_id, GGO_GLYPH_INDEX | GGO_NATIVE | GGO_UNHINTED, &metrics, bufferSize, buffer, &identity) <= 0 ) {
                // shit happened
            } else {
                // Platform SDK is rubbish, read KB87115 instead
                n_g.outline=new Path;
                DWORD polyOffset=0;
                while ( polyOffset < bufferSize ) {
                    TTPOLYGONHEADER const *polyHeader=(TTPOLYGONHEADER const *)(buffer+polyOffset);
                    if (polyOffset+polyHeader->cb > bufferSize) break;

                    if (polyHeader->dwType == TT_POLYGON_TYPE) {
                        n_g.outline->MoveTo(pointfx_to_nrpoint(polyHeader->pfxStart, scale));
                        DWORD curveOffset=polyOffset+sizeof(TTPOLYGONHEADER);

                        while ( curveOffset < polyOffset+polyHeader->cb ) {
                            TTPOLYCURVE const *polyCurve=(TTPOLYCURVE const *)(buffer+curveOffset);
                            POINTFX const *p=polyCurve->apfx;
                            POINTFX const *endp=p+polyCurve->cpfx;

                            switch (polyCurve->wType) {
                                case TT_PRIM_LINE:
                                    while ( p != endp )
                                        n_g.outline->LineTo(pointfx_to_nrpoint(*p++, scale));
                                    break;

                                case TT_PRIM_QSPLINE:
                                {
                                    g_assert(polyCurve->cpfx >= 2);
                                    endp -= 2;
                                    Geom::Point this_mid=pointfx_to_nrpoint(p[0], scale);
                                    while ( p != endp ) {
                                        Geom::Point next_mid=pointfx_to_nrpoint(p[1], scale);
                                        n_g.outline->BezierTo((next_mid+this_mid)/2);
                                        n_g.outline->IntermBezierTo(this_mid);
                                        n_g.outline->EndBezierTo();
                                        ++p;
                                        this_mid=next_mid;
                                    }
                                    n_g.outline->BezierTo(pointfx_to_nrpoint(p[1], scale));
                                    n_g.outline->IntermBezierTo(this_mid);
                                    n_g.outline->EndBezierTo();
                                    break;
                                }

                                case 3:  // TT_PRIM_CSPLINE
                                    g_assert(polyCurve->cpfx % 3 == 0);
                                    while ( p != endp ) {
                                        n_g.outline->CubicTo(pointfx_to_nrpoint(p[2], scale), pointfx_to_nrpoint(p[0], scale), pointfx_to_nrpoint(p[1], scale));
                                        p += 3;
                                    }
                                    break;
                            }
                            curveOffset += sizeof(TTPOLYCURVE)+sizeof(POINTFX)*(polyCurve->cpfx-1);
                        }
                        n_g.outline->Close();
                    }
                    polyOffset += polyHeader->cb;
                }
                doAdd=true;
            }
            delete [] buffer;
        }
#elif defined(USE_PANGO_OSX)
        
        CGPathRef outline = CTFontCreatePathForGlyph((CTFontRef)theFace, (CGGlyph)glyph_id, NULL);

        if (outline && !CGPathIsEmpty(outline)) {
            
            CGRect outlineBox = CGPathGetBoundingBox(outline);
            CGRect box = CTFontGetBoundingBox((CTFontRef)theFace);
            CGFloat upem = CTFontGetUnitsPerEm((CTFontRef)theFace) / 4;
            CGGlyph gls[1] = {(CGGlyph)glyph_id};
            CGSize ads[1] = {0};
            
            g_debug("glyph %d outline box = [%.0f %.0f %.0f %.0f] font box = [%.0f %.0f %.0f %.0f]",
                    glyph_id,
                    outlineBox.origin.x, outlineBox.origin.y, outlineBox.size.width, outlineBox.size.height,
                    box.origin.x, box.origin.y, box.size.width, box.size.height);
            
            CTFontGetAdvancesForGlyphs((CTFontRef)theFace, kCTFontHorizontalOrientation, gls, ads, 1);
            n_g.h_width = box.size.width / upem;
            n_g.h_advance = n_g.h_width + ads[0].width / upem;
            
            CTFontGetAdvancesForGlyphs((CTFontRef)theFace, kCTFontVerticalOrientation, gls, ads, 1);
            n_g.v_width = box.size.height / upem;
            n_g.v_advance = n_g.v_width + ads[0].height / upem;
            n_g.outline = new Path;
            
            cg_to_liv tData;
            tData.theP = n_g.outline;
            tData.scale = 1.0 / upem;
            tData.last = Geom::Point(0,0);

            CGPathApply(outline, &tData, PathApplier);
            
            doAdd = true;
            
        } else {
            g_warning("CTFont %p glyph %d has not an outline", theFace, glyph_id);
        }
        
        if (outline) {
            CGPathRelease(outline);
        }
#else
        
        FT_Error e = FT_Load_Glyph (theFace, glyph_id, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
        if (e) {
            // shit happened
            g_warning("Load glyph [glyph_id = 0x%08x] error : 0x%02x", glyph_id, e);
        } else {
            if ( FT_HAS_HORIZONTAL(theFace) ) {
                n_g.h_advance=((double)theFace->glyph->metrics.horiAdvance)/((double)theFace->units_per_EM);
                n_g.h_width=((double)theFace->glyph->metrics.width)/((double)theFace->units_per_EM);
            } else {
                n_g.h_width=n_g.h_advance=((double)(theFace->bbox.xMax-theFace->bbox.xMin))/((double)theFace->units_per_EM);
            }
            if ( FT_HAS_VERTICAL(theFace) ) {
                n_g.v_advance=((double)theFace->glyph->metrics.vertAdvance)/((double)theFace->units_per_EM);
                n_g.v_width=((double)theFace->glyph->metrics.height)/((double)theFace->units_per_EM);
            } else {
                n_g.v_width=n_g.v_advance=((double)theFace->height)/((double)theFace->units_per_EM);
            }
            if ( theFace->glyph->format == ft_glyph_format_outline ) {
                FT_Outline_Funcs ft2_outline_funcs = {
                    ft2_move_to,
                    ft2_line_to,
                    ft2_conic_to,
                    ft2_cubic_to,
                    0, 0
                };
                n_g.outline=new Path;
                ft2_to_liv   tData;
                tData.theP=n_g.outline;
                tData.scale=1.0/((double)theFace->units_per_EM);
                tData.last=Geom::Point(0,0);
                FT_Outline_Decompose (&theFace->glyph->outline, &ft2_outline_funcs, &tData);
            }
            doAdd=true;
        }
#endif

        if ( doAdd ) {
            if ( n_g.outline ) {
                n_g.outline->FastBBox(n_g.bbox[0],n_g.bbox[1],n_g.bbox[2],n_g.bbox[3]);
                n_g.pathvector=n_g.outline->MakePathVector();
                g_debug("FastBBox = [%.0f %.0f %.0f %.0f]",
                        n_g.bbox[0], n_g.bbox[1], n_g.bbox[2], n_g.bbox[3]);
            }
            glyphs[nbGlyph]=n_g;
            id_to_no[glyph_id]=nbGlyph;
            nbGlyph++;
        }
    } else {
    }
}

bool font_instance::FontMetrics(double &ascent,double &descent,double &leading)
{
    if ( pFont == NULL ) {
        return false;
    }
    InitTheFace();
    if ( theFace == NULL ) {
        return false;
    }
#ifdef USE_PANGO_WIN32
    OUTLINETEXTMETRIC otm;
    if ( !GetOutlineTextMetrics(daddy->hScreenDC,sizeof(otm),&otm) ) {
        return false;
    }
    double scale=1.0/daddy->fontSize;
    ascent=fabs(otm.otmAscent*scale);
    descent=fabs(otm.otmDescent*scale);
    leading=fabs(otm.otmLineGap*scale);
    //otmSubscriptSize, otmSubscriptOffset, otmSuperscriptSize, otmSuperscriptOffset,
#elif defined(USE_PANGO_OSX)
    unsigned em = CTFontGetUnitsPerEm((CTFontRef)theFace) / 4;
    if ( em == 0 ) {
        return false; // bitmap font
    }
    ascent=fabs(((double)CTFontGetAscent((CTFontRef)theFace))/(double)em);
    descent=fabs(((double)CTFontGetDescent((CTFontRef)theFace))/(double)em);
    leading=fabs(((double)CTFontGetLeading((CTFontRef)theFace))/(double)em);
#else
    if ( theFace->units_per_EM == 0 ) {
        return false; // bitmap font
    }
    ascent=fabs(((double)theFace->ascender)/((double)theFace->units_per_EM));
    descent=fabs(((double)theFace->descender)/((double)theFace->units_per_EM));
    leading=fabs(((double)theFace->height)/((double)theFace->units_per_EM));
    leading-=ascent+descent;
#endif
    return true;
}

bool font_instance::FontSlope(double &run, double &rise)
{
    run = 0.0;
    rise = 1.0;

    if ( pFont == NULL ) {
        return false;
    }
    InitTheFace();
    if ( theFace == NULL ) {
        return false;
    }

#ifdef USE_PANGO_WIN32
    OUTLINETEXTMETRIC otm;
    if ( !GetOutlineTextMetrics(daddy->hScreenDC,sizeof(otm),&otm) ) return false;
    run=otm.otmsCharSlopeRun;
    rise=otm.otmsCharSlopeRise;
#elif defined(USE_PANGO_OSX)
    CGFloat angle = CTFontGetSlantAngle((CTFontRef)theFace);
    run = CTFontGetXHeight((CTFontRef)theFace) * tan(angle);
    rise = CTFontGetXHeight((CTFontRef)theFace);
#else
    if ( !FT_IS_SCALABLE(theFace) ) {
        return false; // bitmap font
    }

    TT_HoriHeader *hhea = (TT_HoriHeader*)FT_Get_Sfnt_Table(theFace, ft_sfnt_hhea);
    if (hhea == NULL) {
        return false;
    }
    run = hhea->caret_Slope_Run;
    rise = hhea->caret_Slope_Rise;
#endif
    return true;
}

Geom::OptRect font_instance::BBox(int glyph_id)
{
    int no = -1;
    if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
        LoadGlyph(glyph_id);
        if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
            // didn't load
        } else {
            no = id_to_no[glyph_id];
        }
    } else {
        no = id_to_no[glyph_id];
    }
    if ( no < 0 ) {
        return Geom::OptRect();
    } else {
        Geom::Point rmin(glyphs[no].bbox[0],glyphs[no].bbox[1]);
        Geom::Point rmax(glyphs[no].bbox[2],glyphs[no].bbox[3]);
        return Geom::Rect(rmin, rmax);
    }
}

Path* font_instance::Outline(int glyph_id,Path* copyInto)
{
    int no = -1;
    if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
        LoadGlyph(glyph_id);
        if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
            // didn't load
        } else {
            no = id_to_no[glyph_id];
        }
    } else {
        no = id_to_no[glyph_id];
    }
    if ( no < 0 ) return NULL;
    Path *src_o = glyphs[no].outline;
    if ( copyInto ) {
        copyInto->Reset();
        copyInto->Copy(src_o);
        return copyInto;
    }
    return src_o;
}

Geom::PathVector* font_instance::PathVector(int glyph_id)
{
    int no = -1;
    if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
        LoadGlyph(glyph_id);
        if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
            // didn't load
        } else {
            no = id_to_no[glyph_id];
        }
    } else {
        no = id_to_no[glyph_id];
    }
    if ( no < 0 ) return NULL;
    return glyphs[no].pathvector;
}

double font_instance::Advance(int glyph_id,bool vertical)
{
    int no = -1;
    if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
        LoadGlyph(glyph_id);
        if ( id_to_no.find(glyph_id) == id_to_no.end() ) {
            // didn't load
        } else {
            no=id_to_no[glyph_id];
        }
    } else {
        no = id_to_no[glyph_id];
    }
    if ( no >= 0 ) {
        if ( vertical ) {
            return glyphs[no].v_advance;
        } else {
            return glyphs[no].h_advance;
        }
    }
    return 0;
}


raster_font* font_instance::RasterFont(const Geom::Matrix &trs, double stroke_width, bool vertical, JoinType stroke_join, ButtType stroke_cap, float /*miter_limit*/)
{
    font_style  nStyle;
    nStyle.transform=trs;
    nStyle.vertical=vertical;
    nStyle.stroke_width=stroke_width;
    nStyle.stroke_cap=stroke_cap;
    nStyle.stroke_join=stroke_join;
    nStyle.nbDash=0;
    nStyle.dash_offset=0;
    nStyle.dashes=NULL;
    return RasterFont(nStyle);
}

raster_font* font_instance::RasterFont(const font_style &inStyle)
{
    raster_font  *res=NULL;
    double *savDashes=NULL;
    font_style nStyle=inStyle;
    // for some evil reason font_style doesn't have a copy ctor, so the
    // stuff that should be done there is done here instead (because the
    // raster_font ctor copies nStyle).
    if ( (nStyle.stroke_width > 0) && (nStyle.nbDash > 0) && nStyle.dashes ) {
        savDashes=nStyle.dashes;
        nStyle.dashes=(double*)malloc(nStyle.nbDash*sizeof(double));
        memcpy(nStyle.dashes,savDashes,nStyle.nbDash*sizeof(double));
    }
    StyleMap& loadedStyles = *static_cast<StyleMap*>(loadedPtr);
    if ( loadedStyles.find(nStyle) == loadedStyles.end() ) {
        raster_font *nR = new raster_font(nStyle);
        nR->Ref();
        nR->daddy=this;
        loadedStyles[nStyle]=nR;
        res=nR;
        if ( res ) {
            Ref();
        }
    } else {
        res=loadedStyles[nStyle];
        res->Ref();
        if ( nStyle.dashes ) {
            free(nStyle.dashes); // since they're not taken by a new rasterfont
        }
    }
    nStyle.dashes=savDashes;
    return res;
}

void font_instance::RemoveRasterFont(raster_font* who)
{
    if ( who ) {
        StyleMap& loadedStyles = *static_cast<StyleMap*>(loadedPtr);
        if ( loadedStyles.find(who->style) == loadedStyles.end() ) {
            //g_print("RemoveRasterFont failed \n");
            // not found
        } else {
            loadedStyles.erase(loadedStyles.find(who->style));
            //g_print("RemoveRasterFont\n");
            Unref();
        }
    }
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
