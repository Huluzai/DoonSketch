/*
 *  FontFactory.h
 *  testICU
 *
 */

#ifndef my_font_factory
#define my_font_factory

//#include <glibmm/ustring.h>

#include <functional>
#include <algorithm>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef _WIN32
#define USE_PANGO_WIN32
#endif

#ifdef __APPLE__
#define USE_PANGO_OSX
#endif

#if !defined(USE_PANGO_WIN32) && !defined(USE_PANGO_OSX)
#define USE_PANGO_FT
#endif

#include <pango/pango.h>
#include "nr-type-primitives.h"
#include "nr-type-pos-def.h"
#include "font-style-to-pos.h"
#include <libnrtype/nrtype-forward.h>
#include "../style.h"

/* Freetype */
#ifdef USE_PANGO_WIN32
#include <pango/pangowin32.h>
#elif defined USE_PANGO_OSX
//#include <pango/pangocoretext.h>
#else
#include <pango/pangoft2.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

namespace Glib
{
    class ustring;
}

// the font_factory keeps a hashmap of all the loaded font_instances, and uses the PangoFontDescription
// as index (nota: since pango already does that, using the PangoFont could work too)
struct font_descr_hash : public std::unary_function<PangoFontDescription*,size_t> {
    size_t operator()(PangoFontDescription *const &x) const;
};
struct font_descr_equal : public std::binary_function<PangoFontDescription*, PangoFontDescription*, bool> {
    bool operator()(PangoFontDescription *const &a, PangoFontDescription *const &b) const;
};

// Comparison functions for style names
int style_name_compare(char const *aa, char const *bb);
int family_name_compare(char const *a, char const *b);

// Map type for gathering UI family and style strings
typedef std::map<Glib::ustring, std::list<Glib::ustring> > FamilyToStylesMap;

class font_factory {
public:
    static font_factory *lUsine; /**< The default font_factory; i cannot think of why we would
                                  *   need more than one.
                                  *
                                  *   ("l'usine" is french for "the factory".)
                                  */

    /** A little cache for fonts, so that you don't loose your time looking up fonts in the font list
     *  each font in the cache is refcounted once (and deref'd when removed from the cache). */
    struct font_entry {
        font_instance *f;
        double age;
    };
    int nbEnt;   ///< Number of entries.
    int maxEnt;  ///< Cache size.
    font_entry *ents;

    // Pango data.  Backend-specific structures are cast to these opaque types.
    PangoFontMap *fontServer;
    PangoContext *fontContext;
#ifdef USE_PANGO_WIN32
    PangoWin32FontCache *pangoFontCache;
    HDC hScreenDC;
#endif
    double fontSize; /**< The huge fontsize used as workaround for hinting.
                      *   Different between freetype and win32. */

    font_factory();
    virtual ~font_factory();

    /// Returns the default font_factory.
    static font_factory*  Default();

    /// Constructs a pango string for use with the fontStringMap (see below)
    Glib::ustring         ConstructFontSpecification(PangoFontDescription *font);
    Glib::ustring         ConstructFontSpecification(font_instance *font);

    /// Returns strings to be used in the UI for family and face (or "style" as the column is labeled)
    Glib::ustring         GetUIFamilyString(PangoFontDescription const *fontDescr);
    Glib::ustring         GetUIStyleString(PangoFontDescription const *fontDescr);

    /// Modifiers for the font specification (returns new font specification)
    Glib::ustring         ReplaceFontSpecificationFamily(const Glib::ustring & fontSpec, const Glib::ustring & newFamily);
    Glib::ustring         FontSpecificationSetItalic(const Glib::ustring & fontSpec, bool turnOn);
    Glib::ustring         FontSpecificationSetBold(const Glib::ustring & fontSpec, bool turnOn);

    Glib::ustring         FontSpecificationBestMatch(const Glib::ustring& fontSpec );

    // Gathers all strings needed for UI while storing pango information in
    // fontInstanceMap and fontStringMap
    void                  GetUIFamiliesAndStyles(FamilyToStylesMap *map);

    /// Retrieve a font_instance from a style object, first trying to use the font-specification, the CSS information
    font_instance*        FaceFromStyle(SPStyle const *style);

    // Various functions to get a font_instance from different descriptions.
    font_instance*        FaceFromDescr(char const *family, char const *style);
    font_instance*        FaceFromUIStrings(char const *uiFamily, char const *uiStyle);
    font_instance*        FaceFromPangoString(char const *pangoString);
    font_instance*        FaceFromFontSpecification(char const *fontSpecification);
    font_instance*        Face(PangoFontDescription *descr, bool canFail=true);
    font_instance*        Face(char const *family,
                               int variant=PANGO_VARIANT_NORMAL, int style=PANGO_STYLE_NORMAL,
                               int weight=PANGO_WEIGHT_NORMAL, int stretch=PANGO_STRETCH_NORMAL,
                               int size=10, int spacing=0);
    font_instance*        Face(char const *family, NRTypePosDef apos);

    /// Semi-private: tells the font_factory taht the font_instance 'who' has died and should be removed from loadedFaces
    void                  UnrefFace(font_instance* who);

    // internal
    void                  AddInCache(font_instance *who);

private:
    void*                 loadedPtr;

    // These two maps are used for translating between what's in the UI and a pango
    // font description.  This is necessary because Pango cannot always
    // reproduce these structures from the names it gave us in the first place.

    // Key: A string produced by font_factory::ConstructFontSpecification
    // Value: The associated PangoFontDescription
    typedef std::map<Glib::ustring, PangoFontDescription *> PangoStringToDescrMap;
    PangoStringToDescrMap fontInstanceMap;

    // Key: Family name in UI + Style name in UI
    // Value: The associated string that should be produced with font_factory::ConstructFontSpecification
    typedef std::map<Glib::ustring, Glib::ustring> UIStringToPangoStringMap;
    UIStringToPangoStringMap fontStringMap;
};


#endif /* my_font_factory */


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
