#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_DROP_SHADOW_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_DROP_SHADOW_H__
/* Change the 'DROP_SHADOW' above to be your file name */

/*
 * Copyright (C) 2008 Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
/* ^^^ Change the copyright to be you and your e-mail address ^^^ */

#include "filter.h"

#include "extension/internal/clear-n_.h"
#include "extension/system.h"
#include "extension/extension.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Filter {

class DropShadow : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    DropShadow ( ) : Filter() { };
    virtual ~DropShadow ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Drop Shadow") "</name>\n"
              "<id>org.inkscape.effect.filter.drop-shadow</id>\n"
              "<param name=\"blur\" gui-text=\"" N_("Blur radius, px") "\" type=\"float\" min=\"0.0\" max=\"200.0\">2.0</param>\n"
              "<param name=\"opacity\" gui-text=\"" N_("Opacity, %") "\" type=\"float\" min=\"0.0\" max=\"100.0\">50</param>\n"
              "<param name=\"xoffset\" gui-text=\"" N_("Horizontal offset, px") "\" type=\"float\" min=\"-50.0\" max=\"50.0\">4.0</param>\n"
              "<param name=\"yoffset\" gui-text=\"" N_("Vertical offset, px") "\" type=\"float\" min=\"-50.0\" max=\"50.0\">4.0</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                  "<effects-menu>\n"
                    "<submenu name=\"" N_("Filters") "\" >\n"
                      "<submenu name=\"" N_("Shadows and Glows") "\"/>\n"
                    "</submenu>\n"
                  "</effects-menu>\n"
                  "<menu-tip>" N_("Black, blurred drop shadow") "</menu-tip>\n"
               "</effect>\n"
              "</inkscape-extension>\n", new DropShadow());
    };

};

gchar const *
DropShadow::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream blur;
    std::ostringstream opacity;
    std::ostringstream x;
    std::ostringstream y;

    opacity << ext->get_param_float("opacity") / 100;
    x << ext->get_param_float("xoffset");
    y << ext->get_param_float("yoffset");

    if (ext->get_param_float("blur") > 0) {
        blur << "<feGaussianBlur in=\"composite\" stdDeviation=\"" << ext->get_param_float("blur") << "\" result=\"blur\" />\n";
    } else {
        blur << "";
    }

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Drop Shadow\">\n"
          "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(0,0,0)\" result=\"flood\" />\n"
          "<feComposite in=\"flood\" in2=\"SourceGraphic\" operator=\"in\" result=\"composite1\" />\n"
          "%s"
          "<feOffset dx=\"%s\" dy=\"%s\" result=\"offset\" />\n"
          "<feComposite in=\"SourceGraphic\" in2=\"offset\" operator=\"over\" result=\"composite2\" />\n"
        "</filter>\n", opacity.str().c_str(), blur.str().c_str(), x.str().c_str(), y.str().c_str());
    return _filter;
};

class DropGlow : public Inkscape::Extension::Internal::Filter::Filter {
protected:
    virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
    DropGlow ( ) : Filter() { };
    virtual ~DropGlow ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

    static void init (void) {
        Inkscape::Extension::build_from_mem(
            "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
              "<name>" N_("Drop Glow") "</name>\n"
              "<id>org.inkscape.effect.filter.drop-glow</id>\n"
              "<param name=\"blur\" gui-text=\"" N_("Blur radius, px") "\" type=\"float\" min=\"0.0\" max=\"200.0\">2.0</param>\n"
              "<param name=\"opacity\" gui-text=\"" N_("Opacity, %") "\" type=\"float\" min=\"0.0\" max=\"100.0\">50</param>\n"                
              "<param name=\"xoffset\" gui-text=\"" N_("Horizontal offset, px") "\" type=\"float\" min=\"-50.0\" max=\"50.0\">4.0</param>\n"
              "<param name=\"yoffset\" gui-text=\"" N_("Vertical offset, px") "\" type=\"float\" min=\"-50.0\" max=\"50.0\">4.0</param>\n"
              "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                  "<submenu name=\"" N_("Filters") "\">\n"
                    "<submenu name=\"" N_("Shadows and Glows") "\"/>\n"
                  "</submenu>\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("White, blurred drop glow") "</menu-tip>\n"
              "</effect>\n"
            "</inkscape-extension>\n", new DropGlow());
    };

};

gchar const *
DropGlow::get_filter_text (Inkscape::Extension::Extension * ext)
{
    if (_filter != NULL) g_free((void *)_filter);

    std::ostringstream blur;
    std::ostringstream opacity;
    std::ostringstream x;
    std::ostringstream y;

    opacity << ext->get_param_float("opacity") / 100;
    x << ext->get_param_float("xoffset");
    y << ext->get_param_float("yoffset");

    if (ext->get_param_float("blur") > 0) {
        blur << "<feGaussianBlur in=\"composite\" stdDeviation=\"" << ext->get_param_float("blur") << "\" result=\"blur\" />\n";
    } else {
        blur << "";
    }

    _filter = g_strdup_printf(
        "<filter xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" style=\"color-interpolation-filters:sRGB;\" inkscape:label=\"Drop Shadow\">\n"
          "<feFlood flood-opacity=\"%s\" flood-color=\"rgb(255,255,255)\" result=\"flood\" />\n"
          "<feComposite in=\"flood\" in2=\"SourceGraphic\" operator=\"in\" result=\"composite1\" />\n"
          "%s"
          "<feOffset dx=\"%s\" dy=\"%s\" result=\"offset\" />\n"
          "<feComposite in=\"SourceGraphic\" in2=\"offset\" operator=\"over\" result=\"composite2\" />\n"
        "</filter>\n", opacity.str().c_str(), blur.str().c_str(), x.str().c_str(), y.str().c_str());
    return _filter;
};

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'DROP_SHADOW' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_DROP_SHADOW_H__ */
