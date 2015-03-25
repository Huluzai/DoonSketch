#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <math.h>
#include <gtk/gtk.h>
#include <glibmm/i18n.h>
#include "../dialogs/dialog-events.h"
#include "sp-color-icc-selector.h"
#include "sp-color-scales.h"
#include "svg/svg-icc-color.h"
#include "document.h"
#include "inkscape.h"
#include "profile-manager.h"

#define noDEBUG_LCMS

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
#include "color-profile.h"
#include "cms-system.h"
#include "color-profile-cms-fns.h"

#ifdef DEBUG_LCMS
#include "preferences.h"
#endif // DEBUG_LCMS
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)


#ifdef DEBUG_LCMS
extern guint update_in_progress;
#define DEBUG_MESSAGE(key, ...) \
{\
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();\
    bool dump = prefs->getBool("/options/scislac/" #key);\
    bool dumpD = prefs->getBool("/options/scislac/" #key "D");\
    bool dumpD2 = prefs->getBool("/options/scislac/" #key "D2");\
    dumpD &&= ( (update_in_progress == 0) || dumpD2 );\
    if ( dump )\
    {\
        g_message( __VA_ARGS__ );\
\
    }\
    if ( dumpD )\
    {\
        GtkWidget *dialog = gtk_message_dialog_new(NULL,\
                                                   GTK_DIALOG_DESTROY_WITH_PARENT, \
                                                   GTK_MESSAGE_INFO,    \
                                                   GTK_BUTTONS_OK,      \
                                                   __VA_ARGS__          \
                                                   );\
        g_signal_connect_swapped(dialog, "response",\
                                 G_CALLBACK(gtk_widget_destroy),        \
                                 dialog);                               \
        gtk_widget_show_all( dialog );\
    }\
}
#endif // DEBUG_LCMS



G_BEGIN_DECLS

static void sp_color_icc_selector_class_init (SPColorICCSelectorClass *klass);
static void sp_color_icc_selector_init (SPColorICCSelector *cs);
static void sp_color_icc_selector_destroy (GtkObject *object);

static void sp_color_icc_selector_show_all (GtkWidget *widget);
static void sp_color_icc_selector_hide_all (GtkWidget *widget);


G_END_DECLS

static SPColorSelectorClass *parent_class;

#define XPAD 4
#define YPAD 1

GType
sp_color_icc_selector_get_type (void)
{
    static GType type = 0;
    if (!type) {
        static const GTypeInfo info = {
            sizeof (SPColorICCSelectorClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) sp_color_icc_selector_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (SPColorICCSelector),
            0,    /* n_preallocs */
            (GInstanceInitFunc) sp_color_icc_selector_init,
            0,    /* value_table */
        };

        type = g_type_register_static (SP_TYPE_COLOR_SELECTOR,
                                       "SPColorICCSelector",
                                       &info,
                                       static_cast< GTypeFlags > (0) );
    }
    return type;
}

static void
sp_color_icc_selector_class_init (SPColorICCSelectorClass *klass)
{
    static const gchar* nameset[] = {N_("CMS"), 0};
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    SPColorSelectorClass *selector_class;

    object_class = (GtkObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;
    selector_class = SP_COLOR_SELECTOR_CLASS (klass);

    parent_class = SP_COLOR_SELECTOR_CLASS (g_type_class_peek_parent (klass));

    selector_class->name = nameset;
    selector_class->submode_count = 1;

    object_class->destroy = sp_color_icc_selector_destroy;

    widget_class->show_all = sp_color_icc_selector_show_all;
    widget_class->hide_all = sp_color_icc_selector_hide_all;
}


ColorICCSelector::ColorICCSelector( SPColorSelector* csel )
    : ColorSelector( csel ),
      _updating( FALSE ),
      _dragging( FALSE ),
      _fixupNeeded(0),
      _fixupBtn(0),
      _profileSel(0),
      _fooCount(0),
      _fooScales(0),
      _fooAdj(0),
      _fooSlider(0),
      _fooBtn(0),
      _fooLabel(0),
      _fooMap(0),
      _adj(0),
      _slider(0),
      _sbtn(0),
      _label(0),
      _tt(0)
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    ,
      _profileName(""),
      _prof(),
      _profChannelCount(0),
      _profChangedID(0)
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
{
}

ColorICCSelector::~ColorICCSelector()
{
    _adj = 0;
    _sbtn = 0;
    _label = 0;
}

void sp_color_icc_selector_init (SPColorICCSelector *cs)
{
    SP_COLOR_SELECTOR(cs)->base = new ColorICCSelector( SP_COLOR_SELECTOR(cs) );

    if ( SP_COLOR_SELECTOR(cs)->base )
    {
        SP_COLOR_SELECTOR(cs)->base->init();
    }
}


/*
cmsSigRgbData
cmsSigCmykData
cmsSigCmyData
*/
#define SPACE_ID_RGB 0
#define SPACE_ID_CMY 1
#define SPACE_ID_CMYK 2


#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
static cmsUInt16Number* getScratch() {
    // bytes per pixel * input channels * width
    static cmsUInt16Number* scritch = static_cast<cmsUInt16Number*>(g_new(cmsUInt16Number, 4 * 1024));

    return scritch;
}

struct MapMap {
    cmsUInt32Number space;
    cmsUInt32Number inForm;
};

void getThings( cmsUInt32Number space, gchar const**& namers, gchar const**& tippies, guint const*& scalies ) {
    MapMap possible[] = {
        {cmsSigXYZData,   TYPE_XYZ_16},
        {cmsSigLabData,   TYPE_Lab_16},
        //cmsSigLuvData
        {cmsSigYCbCrData, TYPE_YCbCr_16},
        {cmsSigYxyData,   TYPE_Yxy_16},
        {cmsSigRgbData,   TYPE_RGB_16},
        {cmsSigGrayData,  TYPE_GRAY_16},
        {cmsSigHsvData,   TYPE_HSV_16},
        {cmsSigHlsData,   TYPE_HLS_16},
        {cmsSigCmykData,  TYPE_CMYK_16},
        {cmsSigCmyData,   TYPE_CMY_16},
    };

    static gchar const *names[][6] = {
        {"_X", "_Y", "_Z", "", "", ""},
        {"_L", "_a", "_b", "", "", ""},
        //
        {"_Y", "C_b", "C_r", "", "", ""},
        {"_Y", "_x", "y", "", "", ""},
        {_("_R"), _("_G"), _("_B"), "", "", ""},
        {_("_G"), "", "", "", "", ""},
        {_("_H"), _("_S"), "_V", "", "", ""},
        {_("_H"), _("_L"), _("_S"), "", "", ""},
        {_("_C"), _("_M"), _("_Y"), _("_K"), "", ""},
        {_("_C"), _("_M"), _("_Y"), "", "", ""},
    };

    static gchar const *tips[][6] = {
        {"X", "Y", "Z", "", "", ""},
        {"L", "a", "b", "", "", ""},
        //
        {"Y", "Cb", "Cr", "", "", ""},
        {"Y", "x", "y", "", "", ""},
        {_("Red"), _("Green"), _("Blue"), "", "", ""},
        {_("Gray"), "", "", "", "", ""},
        {_("Hue"), _("Saturation"), "Value", "", "", ""},
        {_("Hue"), _("Lightness"), _("Saturation"), "", "", ""},
        {_("Cyan"), _("Magenta"), _("Yellow"), _("Black"), "", ""},
        {_("Cyan"), _("Magenta"), _("Yellow"), "", "", ""},
    };

    static guint scales[][6] = {
        {2, 1, 2, 1, 1, 1},
        {100, 256, 256, 1, 1, 1},
        //
        {1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1},
        {360, 1, 1, 1, 1, 1},
        {360, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1},
    };

    int index = 0;
    for ( guint i = 0; i < G_N_ELEMENTS(possible); i++ ) {
        if ( possible[i].space == space ) {
            index = i;
            break;
        }
    }

    namers = names[index];
    tippies = tips[index];
    scalies = scales[index];
}


void getThings( Inkscape::ColorProfile *prof, gchar const**& namers, gchar const**& tippies, guint const*& scalies ) {
    getThings( asICColorSpaceSig(prof->getColorSpace()), namers, tippies, scalies );
}

#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)


void ColorICCSelector::init()
{
    GtkWidget *t;
    gint row = 0;

    _updating = FALSE;
    _dragging = FALSE;

    _tt = gtk_tooltips_new();

    t = gtk_table_new (5, 3, FALSE);
    gtk_widget_show (t);
    gtk_box_pack_start (GTK_BOX (_csel), t, TRUE, TRUE, 0);

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    //guint partCount = _cmsChannelsOf( cmsSigRgbData );
    gchar const** names = 0;
    gchar const** tips = 0;
    getThings( cmsSigRgbData, names, tips, _fooScales );
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    /* Create components */
    row = 0;


    _fixupBtn = gtk_button_new_with_label(_("Fix"));
    g_signal_connect( G_OBJECT(_fixupBtn), "clicked", G_CALLBACK(_fixupHit), (gpointer)this );
    gtk_widget_set_sensitive( _fixupBtn, FALSE );
    gtk_tooltips_set_tip( _tt, _fixupBtn, _("Fix RGB fallback to match icc-color() value."), NULL );
    //gtk_misc_set_alignment( GTK_MISC (_fixupBtn), 1.0, 0.5 );
    gtk_widget_show( _fixupBtn );
    gtk_table_attach( GTK_TABLE (t), _fixupBtn, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD );


#if GTK_CHECK_VERSION(2,24,0)
    _profileSel = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(_profileSel), _("<none>") );
#else
    _profileSel = gtk_combo_box_new_text();
    gtk_combo_box_append_text( GTK_COMBO_BOX(_profileSel), _("<none>") );
#endif
    gtk_widget_show( _profileSel );
    gtk_combo_box_set_active( GTK_COMBO_BOX(_profileSel), 0 );
    gtk_table_attach( GTK_TABLE(t), _profileSel, 1, 2, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD );

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    _profChangedID = g_signal_connect( G_OBJECT(_profileSel), "changed", G_CALLBACK(_profileSelected), (gpointer)this );
#else
    gtk_widget_set_sensitive( _profileSel, false );
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)


    row++;

    _fooCount = 4;
    _fooAdj = new GtkAdjustment*[_fooCount];
    _fooSlider = new GtkWidget*[_fooCount];
    _fooBtn = new GtkWidget*[_fooCount];
    _fooLabel = new GtkWidget*[_fooCount];
    _fooMap = new guchar*[_fooCount];

    for ( guint i = 0; i < _fooCount; i++ ) {
        /* Label */
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        _fooLabel[i] = gtk_label_new_with_mnemonic( names[i] );
#else
        _fooLabel[i] = gtk_label_new_with_mnemonic( "." );
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        gtk_misc_set_alignment( GTK_MISC (_fooLabel[i]), 1.0, 0.5 );
        gtk_widget_show( _fooLabel[i] );
        gtk_table_attach( GTK_TABLE (t), _fooLabel[i], 0, 1, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD );

        /* Adjustment */
        gdouble step = static_cast<gdouble>(_fooScales[i]) / 100.0;
        gdouble page = static_cast<gdouble>(_fooScales[i]) / 10.0;
        gint digits = (step > 0.9) ? 0 : 2;
        _fooAdj[i] = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, 0.0, _fooScales[i],  step, page, page ) );

        /* Slider */
        _fooSlider[i] = sp_color_slider_new( _fooAdj[i] );
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        gtk_tooltips_set_tip( _tt, _fooSlider[i], tips[i], NULL );
#else
        gtk_tooltips_set_tip( _tt, _fooSlider[i], ".", NULL );
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        gtk_widget_show( _fooSlider[i] );
        gtk_table_attach( GTK_TABLE (t), _fooSlider[i], 1, 2, row, row + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)GTK_FILL, XPAD, YPAD );

        _fooBtn[i] = gtk_spin_button_new( _fooAdj[i], step, digits );
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        gtk_tooltips_set_tip( _tt, _fooBtn[i], tips[i], NULL );
#else
        gtk_tooltips_set_tip( _tt, _fooBtn[i], ".", NULL );
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        sp_dialog_defocus_on_enter( _fooBtn[i] );
        gtk_label_set_mnemonic_widget( GTK_LABEL(_fooLabel[i]), _fooBtn[i] );
        gtk_widget_show( _fooBtn[i] );
        gtk_table_attach( GTK_TABLE (t), _fooBtn[i], 2, 3, row, row + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, XPAD, YPAD );

        _fooMap[i] = g_new( guchar, 4 * 1024 );
        memset( _fooMap[i], 0x0ff, 1024 * 4 );


        /* Signals */
        gtk_signal_connect( GTK_OBJECT( _fooAdj[i] ), "value_changed", GTK_SIGNAL_FUNC( _adjustmentChanged ), _csel );

        gtk_signal_connect( GTK_OBJECT( _fooSlider[i] ), "grabbed", GTK_SIGNAL_FUNC( _sliderGrabbed ), _csel );
        gtk_signal_connect( GTK_OBJECT( _fooSlider[i] ), "released", GTK_SIGNAL_FUNC( _sliderReleased ), _csel );
        gtk_signal_connect( GTK_OBJECT( _fooSlider[i] ), "changed", GTK_SIGNAL_FUNC( _sliderChanged ), _csel );

        row++;
    }

    /* Label */
    _label = gtk_label_new_with_mnemonic (_("_A"));
    gtk_misc_set_alignment (GTK_MISC (_label), 1.0, 0.5);
    gtk_widget_show (_label);
    gtk_table_attach (GTK_TABLE (t), _label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);

    /* Adjustment */
    _adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 255.0, 1.0, 10.0, 10.0);

    /* Slider */
    _slider = sp_color_slider_new (_adj);
    gtk_tooltips_set_tip (_tt, _slider, _("Alpha (opacity)"), NULL);
    gtk_widget_show (_slider);
    gtk_table_attach (GTK_TABLE (t), _slider, 1, 2, row, row + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)GTK_FILL, XPAD, YPAD);

    sp_color_slider_set_colors( SP_COLOR_SLIDER( _slider ),
                                SP_RGBA32_F_COMPOSE( 1.0, 1.0, 1.0, 0.0 ),
                                SP_RGBA32_F_COMPOSE( 1.0, 1.0, 1.0, 0.5 ),
                                SP_RGBA32_F_COMPOSE( 1.0, 1.0, 1.0, 1.0 ) );


    /* Spinbutton */
    _sbtn = gtk_spin_button_new (GTK_ADJUSTMENT (_adj), 1.0, 0);
    gtk_tooltips_set_tip (_tt, _sbtn, _("Alpha (opacity)"), NULL);
    sp_dialog_defocus_on_enter (_sbtn);
    gtk_label_set_mnemonic_widget (GTK_LABEL(_label), _sbtn);
    gtk_widget_show (_sbtn);
    gtk_table_attach (GTK_TABLE (t), _sbtn, 2, 3, row, row + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, XPAD, YPAD);

    /* Signals */
    gtk_signal_connect (GTK_OBJECT (_adj), "value_changed",
                        GTK_SIGNAL_FUNC (_adjustmentChanged), _csel);

    gtk_signal_connect (GTK_OBJECT (_slider), "grabbed",
                        GTK_SIGNAL_FUNC (_sliderGrabbed), _csel);
    gtk_signal_connect (GTK_OBJECT (_slider), "released",
                        GTK_SIGNAL_FUNC (_sliderReleased), _csel);
    gtk_signal_connect (GTK_OBJECT (_slider), "changed",
                        GTK_SIGNAL_FUNC (_sliderChanged), _csel);
}

static void
sp_color_icc_selector_destroy (GtkObject *object)
{
    if (((GtkObjectClass *) (parent_class))->destroy)
        (* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_color_icc_selector_show_all (GtkWidget *widget)
{
    gtk_widget_show (widget);
}

static void
sp_color_icc_selector_hide_all (GtkWidget *widget)
{
    gtk_widget_hide (widget);
}

GtkWidget *
sp_color_icc_selector_new (void)
{
    SPColorICCSelector *csel;

    csel = (SPColorICCSelector*)gtk_type_new (SP_TYPE_COLOR_ICC_SELECTOR);

    return GTK_WIDGET (csel);
}


void ColorICCSelector::_fixupHit( GtkWidget* /*src*/, gpointer data )
{
    ColorICCSelector* self = reinterpret_cast<ColorICCSelector*>(data);
    gtk_widget_set_sensitive( self->_fixupBtn, FALSE );
    self->_adjustmentChanged( self->_fooAdj[0], SP_COLOR_ICC_SELECTOR(self->_csel) );
}

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void ColorICCSelector::_profileSelected( GtkWidget* /*src*/, gpointer data )
{
    ColorICCSelector* self = reinterpret_cast<ColorICCSelector*>(data);
    gint activeIndex = gtk_combo_box_get_active( GTK_COMBO_BOX(self->_profileSel) );
#if GTK_CHECK_VERSION(2,24,0)
    gchar* name = (activeIndex != 0) ? gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT(self->_profileSel) ) : 0;
#else
    gchar* name = (activeIndex != 0) ? gtk_combo_box_get_active_text( GTK_COMBO_BOX(self->_profileSel) ) : 0;
#endif //GTK_CHECK_VERSION
    self->_switchToProfile( name );
    if ( name ) {
        g_free( name );
    }
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void ColorICCSelector::_switchToProfile( gchar const* name )
{
    bool dirty = false;
    SPColor tmp( _color );

    if ( name ) {
        if ( tmp.icc && tmp.icc->colorProfile == name ) {
#ifdef DEBUG_LCMS
             g_message("Already at name [%s]", name );
#endif // DEBUG_LCMS
        } else {
#ifdef DEBUG_LCMS
             g_message("Need to switch to profile [%s]", name );
#endif // DEBUG_LCMS
            if ( tmp.icc ) {
                tmp.icc->colors.clear();
            } else {
                tmp.icc = new SVGICCColor();
            }
            tmp.icc->colorProfile = name;
            Inkscape::ColorProfile* newProf = SP_ACTIVE_DOCUMENT->profileManager->find(name);
            if ( newProf ) {
                cmsHTRANSFORM trans = newProf->getTransfFromSRGB8();
                if ( trans ) {
                    guint32 val = _color.toRGBA32(0);
                    guchar pre[4] = {
                        static_cast<guchar>(SP_RGBA32_R_U(val)),
                        static_cast<guchar>(SP_RGBA32_G_U(val)),
                        static_cast<guchar>(SP_RGBA32_B_U(val)),
                        255};
#ifdef DEBUG_LCMS
                    g_message("Shoving in [%02x] [%02x] [%02x]", pre[0], pre[1], pre[2]);
#endif // DEBUG_LCMS
                    cmsUInt16Number post[4] = {0,0,0,0};
                    cmsDoTransform( trans, pre, post, 1 );
#ifdef DEBUG_LCMS
                    g_message("got on out [%04x] [%04x] [%04x] [%04x]", post[0], post[1], post[2], post[3]);
#endif // DEBUG_LCMS
#if HAVE_LIBLCMS1
                    guint count = _cmsChannelsOf( asICColorSpaceSig(newProf->getColorSpace()) );
#elif HAVE_LIBLCMS2
                    guint count = cmsChannelsOf( asICColorSpaceSig(newProf->getColorSpace()) );
#endif

                    gchar const** names = 0;
                    gchar const** tips = 0;
                    guint const* scales = 0;
                    getThings( asICColorSpaceSig(newProf->getColorSpace()), names, tips, scales );

                    for ( guint i = 0; i < count; i++ ) {
                        gdouble val = (((gdouble)post[i])/65535.0) * (gdouble)scales[i];
#ifdef DEBUG_LCMS
                        g_message("     scaled %d by %d to be %f", i, scales[i], val);
#endif // DEBUG_LCMS
                        tmp.icc->colors.push_back(val);
                    }
                    cmsHTRANSFORM retrans = newProf->getTransfToSRGB8();
                    if ( retrans ) {
                        cmsDoTransform( retrans, post, pre, 1 );
#ifdef DEBUG_LCMS
                        g_message("  back out [%02x] [%02x] [%02x]", pre[0], pre[1], pre[2]);
#endif // DEBUG_LCMS
                        tmp.set(SP_RGBA32_U_COMPOSE(pre[0], pre[1], pre[2], 0xff));
                    }
                }
            }
            dirty = true;
        }
    } else {
#ifdef DEBUG_LCMS
         g_message("NUKE THE ICC");
#endif // DEBUG_LCMS
        if ( tmp.icc ) {
            delete tmp.icc;
            tmp.icc = 0;
            dirty = true;
            _fixupHit( 0, this );
        } else {
#ifdef DEBUG_LCMS
             g_message("No icc to nuke");
#endif // DEBUG_LCMS
        }
    }

    if ( dirty ) {
#ifdef DEBUG_LCMS
        g_message("+----------------");
        g_message("+   new color is [%s]", tmp.toString().c_str());
#endif // DEBUG_LCMS
        _setProfile( tmp.icc );
        //_adjustmentChanged( _fooAdj[0], SP_COLOR_ICC_SELECTOR(_csel) );
        setColorAlpha( tmp, _alpha, true );
#ifdef DEBUG_LCMS
        g_message("+_________________");
#endif // DEBUG_LCMS
    }
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void ColorICCSelector::_profilesChanged( std::string const & name )
{
    GtkComboBox* combo = GTK_COMBO_BOX(_profileSel);

    g_signal_handler_block( G_OBJECT(_profileSel), _profChangedID );

    GtkTreeModel* model = gtk_combo_box_get_model( combo );
    GtkTreeIter iter;
    while ( gtk_tree_model_get_iter_first( model, &iter ) ) {
#if GTK_CHECK_VERSION(2,24,0)
        gtk_combo_box_text_remove( GTK_COMBO_BOX_TEXT(combo), 0 );
#else
        gtk_combo_box_remove_text( combo, 0 );
#endif
    }

#if GTK_CHECK_VERSION(2,24,0)
    gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combo), _("<none>"));
#else
    gtk_combo_box_append_text( combo, _("<none>"));
#endif

    gtk_combo_box_set_active( combo, 0 );

    int index = 1;
    const GSList *current = sp_document_get_resource_list( SP_ACTIVE_DOCUMENT, "iccprofile" );
    while ( current ) {
        SPObject* obj = SP_OBJECT(current->data);
        Inkscape::ColorProfile* prof = reinterpret_cast<Inkscape::ColorProfile*>(obj);
#if GTK_CHECK_VERSION(2,24,0)
        gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combo), prof->name );
#else
        gtk_combo_box_append_text( combo, prof->name );
#endif
        if ( name == prof->name ) {
            gtk_combo_box_set_active( combo, index );
        }

        index++;
        current = g_slist_next(current);
    }

    g_signal_handler_unblock( G_OBJECT(_profileSel), _profChangedID );
}
#else
void ColorICCSelector::_profilesChanged( std::string const & /*name*/ )
{
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

/* Helpers for setting color value */

void ColorICCSelector::_colorChanged()
{
    _updating = TRUE;
     //sp_color_icc_set_color( SP_COLOR_ICC( _icc ), &color );

#ifdef DEBUG_LCMS
    g_message( "/^^^^^^^^^  %p::_colorChanged(%08x:%s)", this,
               _color.toRGBA32(_alpha), ( (_color.icc) ? _color.icc->colorProfile.c_str(): "<null>" )
               );
#endif // DEBUG_LCMS

#ifdef DEBUG_LCMS
    g_message("FLIPPIES!!!!     %p   '%s'", _color.icc, (_color.icc ? _color.icc->colorProfile.c_str():"<null>"));
#endif // DEBUG_LCMS

    _profilesChanged( (_color.icc) ? _color.icc->colorProfile : std::string("") );
    ColorScales::setScaled( _adj, _alpha );

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    _setProfile( _color.icc );
    _fixupNeeded = 0;
    gtk_widget_set_sensitive( _fixupBtn, FALSE );

    if ( _prof && _prof->getTransfToSRGB8() ) {
        cmsUInt16Number tmp[4];
        for ( guint i = 0; i < _profChannelCount; i++ ) {
            gdouble val = 0.0;
            if ( _color.icc->colors.size() > i ) {
                if ( _fooScales[i] == 256 ) {
                    val = (_color.icc->colors[i] + 128.0) / static_cast<gdouble>(_fooScales[i]);
                } else {
                    val = _color.icc->colors[i] / static_cast<gdouble>(_fooScales[i]);
                }
            }
            tmp[i] = val * 0x0ffff;
        }
        guchar post[4] = {0,0,0,0};
        cmsHTRANSFORM trans = _prof->getTransfToSRGB8();
        if ( trans ) {
            cmsDoTransform( trans, tmp, post, 1 );
            guint32 other = SP_RGBA32_U_COMPOSE(post[0], post[1], post[2], 255 );
            if ( other != _color.toRGBA32(255) ) {
                _fixupNeeded = other;
                gtk_widget_set_sensitive( _fixupBtn, TRUE );
#ifdef DEBUG_LCMS
                g_message("Color needs to change 0x%06x to 0x%06x", _color.toRGBA32(255) >> 8, other >> 8 );
#endif // DEBUG_LCMS
            }
        }
    }
#else
    //(void)color;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    _updateSliders( -1 );


    _updating = FALSE;
#ifdef DEBUG_LCMS
    g_message( "\\_________  %p::_colorChanged()", this );
#endif // DEBUG_LCMS
}

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void ColorICCSelector::_setProfile( SVGICCColor* profile )
{
#ifdef DEBUG_LCMS
    g_message( "/^^^^^^^^^  %p::_setProfile(%s)", this,
               ( (profile) ? profile->colorProfile.c_str() : "<null>")
               );
#endif // DEBUG_LCMS
    bool profChanged = false;
    if ( _prof && (!profile || (_profileName != profile->colorProfile) ) ) {
        // Need to clear out the prior one
        profChanged = true;
        _profileName.clear();
        _prof = 0;
        _profChannelCount = 0;
    } else if ( profile && !_prof ) {
        profChanged = true;
    }

    for ( guint i = 0; i < _fooCount; i++ ) {
        gtk_widget_hide( _fooLabel[i] );
        gtk_widget_hide( _fooSlider[i] );
        gtk_widget_hide( _fooBtn[i] );
    }

    if ( profile ) {
        _prof = SP_ACTIVE_DOCUMENT->profileManager->find(profile->colorProfile.c_str());
        if ( _prof && (asICColorProfileClassSig(_prof->getProfileClass()) != cmsSigNamedColorClass) ) {
#if HAVE_LIBLCMS1
            _profChannelCount = _cmsChannelsOf( asICColorSpaceSig(_prof->getColorSpace()) );
#elif HAVE_LIBLCMS2
            _profChannelCount = cmsChannelsOf( asICColorSpaceSig(_prof->getColorSpace()) );
#endif

            gchar const** names = 0;
            gchar const** tips = 0;
            getThings( asICColorSpaceSig(_prof->getColorSpace()), names, tips, _fooScales );

            if ( profChanged ) {
                for ( guint i = 0; i < _profChannelCount; i++ ) {
                    gtk_label_set_text_with_mnemonic( GTK_LABEL(_fooLabel[i]), names[i]);

                    gtk_tooltips_set_tip( _tt, _fooSlider[i], tips[i], NULL );
                    gtk_tooltips_set_tip( _tt, _fooBtn[i], tips[i], NULL );

                    sp_color_slider_set_colors( SP_COLOR_SLIDER(_fooSlider[i]),
                                                SPColor(0.0, 0.0, 0.0).toRGBA32(0xff),
                                                SPColor(0.5, 0.5, 0.5).toRGBA32(0xff),
                                                SPColor(1.0, 1.0, 1.0).toRGBA32(0xff) );
/*
                    _fooAdj[i] = GTK_ADJUSTMENT( gtk_adjustment_new( val, 0.0, _fooScales[i],  step, page, page ) );
                    gtk_signal_connect( GTK_OBJECT( _fooAdj[i] ), "value_changed", GTK_SIGNAL_FUNC( _adjustmentChanged ), _csel );

                    sp_color_slider_set_adjustment( SP_COLOR_SLIDER(_fooSlider[i]), _fooAdj[i] );
                    gtk_spin_button_set_adjustment( GTK_SPIN_BUTTON(_fooBtn[i]), _fooAdj[i] );
                    gtk_spin_button_set_digits( GTK_SPIN_BUTTON(_fooBtn[i]), digits );
*/
                    gtk_widget_show( _fooLabel[i] );
                    gtk_widget_show( _fooSlider[i] );
                    gtk_widget_show( _fooBtn[i] );
                    //gtk_adjustment_set_value( _fooAdj[i], 0.0 );
                    //gtk_adjustment_set_value( _fooAdj[i], val );
                }
                for ( guint i = _profChannelCount; i < _fooCount; i++ ) {
                    gtk_widget_hide( _fooLabel[i] );
                    gtk_widget_hide( _fooSlider[i] );
                    gtk_widget_hide( _fooBtn[i] );
                }
            }
        } else {
            // Give up for now on named colors
            _prof = 0;
        }
    }

#ifdef DEBUG_LCMS
    g_message( "\\_________  %p::_setProfile()", this );
#endif // DEBUG_LCMS
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

void ColorICCSelector::_updateSliders( gint ignore )
{
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    if ( _color.icc )
    {
        for ( guint i = 0; i < _profChannelCount; i++ ) {
            gdouble val = 0.0;
            if ( _color.icc->colors.size() > i ) {
                if ( _fooScales[i] == 256 ) {
                    val = (_color.icc->colors[i] + 128.0) / static_cast<gdouble>(_fooScales[i]);
                } else {
                    val = _color.icc->colors[i] / static_cast<gdouble>(_fooScales[i]);
                }
            }
            gtk_adjustment_set_value( _fooAdj[i], val );
        }

        if ( _prof && _prof->getTransfToSRGB8() ) {
            for ( guint i = 0; i < _profChannelCount; i++ ) {
                if ( static_cast<gint>(i) != ignore ) {
                    cmsUInt16Number* scratch = getScratch();
                    cmsUInt16Number filler[4] = {0, 0, 0, 0};
                    for ( guint j = 0; j < _profChannelCount; j++ ) {
                        filler[j] = 0x0ffff * ColorScales::getScaled( _fooAdj[j] );
                    }

                    cmsUInt16Number* p = scratch;
                    for ( guint x = 0; x < 1024; x++ ) {
                        for ( guint j = 0; j < _profChannelCount; j++ ) {
                            if ( j == i ) {
                                *p++ = x * 0x0ffff / 1024;
                            } else {
                                *p++ = filler[j];
                            }
                        }
                    }

                    cmsHTRANSFORM trans = _prof->getTransfToSRGB8();
                    if ( trans ) {
                        cmsDoTransform( trans, scratch, _fooMap[i], 1024 );
                        sp_color_slider_set_map( SP_COLOR_SLIDER(_fooSlider[i]), _fooMap[i] );
                    }
                }
            }
        }
    }
#else
    (void)ignore;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    guint32 start = _color.toRGBA32( 0x00 );
    guint32 mid = _color.toRGBA32( 0x7f );
    guint32 end = _color.toRGBA32( 0xff );

    sp_color_slider_set_colors( SP_COLOR_SLIDER(_slider), start, mid, end );

}


void ColorICCSelector::_adjustmentChanged( GtkAdjustment *adjustment, SPColorICCSelector *cs )
{
// // TODO check this. It looks questionable:
//     // if a value is entered between 0 and 1 exclusive, normalize it to (int) 0..255  or 0..100
//     if (adjustment->value > 0.0 && adjustment->value < 1.0) {
//         gtk_adjustment_set_value( adjustment, floor ((adjustment->value) * adjustment->upper + 0.5) );
//     }

#ifdef DEBUG_LCMS
    g_message( "/^^^^^^^^^  %p::_adjustmentChanged()", cs );
#endif // DEBUG_LCMS

     ColorICCSelector* iccSelector = (ColorICCSelector*)(SP_COLOR_SELECTOR(cs)->base);
     if (iccSelector->_updating) {
         return;
     }

     iccSelector->_updating = TRUE;

     gint match = -1;

     SPColor newColor( iccSelector->_color );
     gfloat scaled = ColorScales::getScaled( iccSelector->_adj );
     if ( iccSelector->_adj == adjustment ) {
#ifdef DEBUG_LCMS
         g_message("ALPHA");
#endif // DEBUG_LCMS
     } else {
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
         for ( guint i = 0; i < iccSelector->_fooCount; i++ ) {
             if ( iccSelector->_fooAdj[i] == adjustment ) {
                 match = i;
                 break;
             }
         }
         if ( match >= 0 ) {
#ifdef DEBUG_LCMS
             g_message(" channel %d", match );
#endif // DEBUG_LCMS
         }


         cmsUInt16Number tmp[4];
         for ( guint i = 0; i < 4; i++ ) {
             tmp[i] = ColorScales::getScaled( iccSelector->_fooAdj[i] ) * 0x0ffff;
         }
         guchar post[4] = {0,0,0,0};

         cmsHTRANSFORM trans = iccSelector->_prof->getTransfToSRGB8();
         if ( trans ) {
             cmsDoTransform( trans, tmp, post, 1 );
         }

         SPColor other( SP_RGBA32_U_COMPOSE(post[0], post[1], post[2], 255) );
         other.icc = new SVGICCColor();
         if ( iccSelector->_color.icc ) {
             other.icc->colorProfile = iccSelector->_color.icc->colorProfile;
         }

         guint32 prior = iccSelector->_color.toRGBA32(255);
         guint32 newer = other.toRGBA32(255);

         if ( prior != newer ) {
#ifdef DEBUG_LCMS
             g_message("Transformed color from 0x%08x to 0x%08x", prior, newer );
             g_message("      ~~~~ FLIP");
#endif // DEBUG_LCMS
             newColor = other;
             newColor.icc->colors.clear();
             for ( guint i = 0; i < iccSelector->_profChannelCount; i++ ) {
                 gdouble val = ColorScales::getScaled( iccSelector->_fooAdj[i] );
                 if ( iccSelector->_fooScales ) {
                     val *= iccSelector->_fooScales[i];
                     if ( iccSelector->_fooScales[i] == 256 ) {
                         val -= 128;
                     }
                 }
                 newColor.icc->colors.push_back( val );
             }
         }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
     }
     iccSelector->_updateInternals( newColor, scaled, iccSelector->_dragging );
     iccSelector->_updateSliders( match );

     iccSelector->_updating = FALSE;
#ifdef DEBUG_LCMS
     g_message( "\\_________  %p::_adjustmentChanged()", cs );
#endif // DEBUG_LCMS
}

void ColorICCSelector::_sliderGrabbed( SPColorSlider */*slider*/, SPColorICCSelector */*cs*/ )
{
//    ColorICCSelector* iccSelector = (ColorICCSelector*)(SP_COLOR_SELECTOR(cs)->base);
//     if (!iccSelector->_dragging) {
//         iccSelector->_dragging = TRUE;
//         iccSelector->_grabbed();
//         iccSelector->_updateInternals( iccSelector->_color, ColorScales::getScaled( iccSelector->_adj ), iccSelector->_dragging );
//     }
}

void ColorICCSelector::_sliderReleased( SPColorSlider */*slider*/, SPColorICCSelector */*cs*/ )
{
//     ColorICCSelector* iccSelector = (ColorICCSelector*)(SP_COLOR_SELECTOR(cs)->base);
//     if (iccSelector->_dragging) {
//         iccSelector->_dragging = FALSE;
//         iccSelector->_released();
//         iccSelector->_updateInternals( iccSelector->_color, ColorScales::getScaled( iccSelector->_adj ), iccSelector->_dragging );
//     }
}

#ifdef DEBUG_LCMS
void ColorICCSelector::_sliderChanged( SPColorSlider *slider, SPColorICCSelector *cs )
#else
void ColorICCSelector::_sliderChanged( SPColorSlider */*slider*/, SPColorICCSelector */*cs*/ )
#endif // DEBUG_LCMS
{
#ifdef DEBUG_LCMS
    g_message("Changed  %p and %p", slider, cs );
#endif // DEBUG_LCMS
//     ColorICCSelector* iccSelector = (ColorICCSelector*)(SP_COLOR_SELECTOR(cs)->base);

//     iccSelector->_updateInternals( iccSelector->_color, ColorScales::getScaled( iccSelector->_adj ), iccSelector->_dragging );
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
