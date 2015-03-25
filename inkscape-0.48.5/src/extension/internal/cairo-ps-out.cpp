/*
 * A quick hack to use the Cairo renderer to write out a file.  This
 * then makes 'save as...' PS.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Ulf Erikson <ulferikson@users.sf.net>
 *   Adib Taraben <theAdib@yahoo.com>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_CAIRO_PDF

#include "cairo-ps-out.h"
#include "cairo-render-context.h"
#include "cairo-renderer.h"
#include "latex-text-renderer.h"
#include <print.h>
#include "extension/system.h"
#include "extension/print.h"
#include "extension/db.h"
#include "extension/output.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"

#include "display/curve.h"
#include "display/canvas-bpath.h"
#include "sp-item.h"
#include "style.h"
#include "sp-root.h"
#include "sp-shape.h"

#include "io/sys.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

bool CairoPsOutput::check (Inkscape::Extension::Extension * /*module*/)
{
    if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_PS)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

bool CairoEpsOutput::check (Inkscape::Extension::Extension * /*module*/)
{
    if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_EPS)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static bool
ps_print_document_to_file(SPDocument *doc, gchar const *filename, unsigned int level, bool texttopath, bool omittext,
                          bool filtertobitmap, int resolution, const gchar * const exportId, bool exportDrawing, bool exportCanvas, bool eps = false)
{
    sp_document_ensure_up_to_date(doc);

    SPItem *base = NULL;

    bool pageBoundingBox = TRUE;
    if (exportId && strcmp(exportId, "")) {
        // we want to export the given item only
        base = SP_ITEM(doc->getObjectById(exportId));
        pageBoundingBox = exportCanvas;
    }
    else {
        // we want to export the entire document from root
        base = SP_ITEM(sp_document_root(doc));
        pageBoundingBox = !exportDrawing;
    }

    if (!base)
        return false;

    /* Create new arena */
    NRArena *arena = NRArena::create();
    unsigned dkey = sp_item_display_key_new(1);
    sp_item_invoke_show(base, arena, dkey, SP_ITEM_SHOW_DISPLAY);

    /* Create renderer and context */
    CairoRenderer *renderer = new CairoRenderer();
    CairoRenderContext *ctx = renderer->createContext();
    ctx->setPSLevel(level);
    ctx->setEPS(eps);
    ctx->setTextToPath(texttopath);
    renderer->_omitText = omittext;
    ctx->setFilterToBitmap(filtertobitmap);
    ctx->setBitmapResolution(resolution);

    bool ret = ctx->setPsTarget(filename);
    if(ret) {
        /* Render document */
        ret = renderer->setupDocument(ctx, doc, pageBoundingBox, base);
        if (ret) {
            renderer->renderItem(ctx, base);
            ret = ctx->finish();
        }
    }

    /* Release arena */
    sp_item_invoke_hide(base, dkey);
    nr_object_unref((NRObject *) arena);

    renderer->destroyContext(ctx);
    delete renderer;

    return ret;
}


/**
    \brief  This function calls the output module with the filename
	\param  mod   unused
	\param  doc   Document to be saved
    \param  filename   Filename to save to (probably will end in .ps)
*/
void
CairoPsOutput::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *filename)
{
    Inkscape::Extension::Extension * ext;
    unsigned int ret;

    ext = Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_PS);
    if (ext == NULL)
        return;

    const gchar *new_level = NULL;
    int level = CAIRO_PS_LEVEL_2;
    try {
        new_level = mod->get_param_enum("PSlevel");
        if((new_level != NULL) && (g_ascii_strcasecmp("PS3", new_level) == 0))
            level = CAIRO_PS_LEVEL_3;
    } catch(...) {}

    bool new_textToPath  = FALSE;
    try {
        new_textToPath  = mod->get_param_bool("textToPath");
    } catch(...) {}

    bool new_textToLaTeX  = FALSE;
    try {
        new_textToLaTeX  = mod->get_param_bool("textToLaTeX");
    }
    catch(...) {
        g_warning("Parameter <textToLaTeX> might not exist");
    }

    bool new_blurToBitmap  = FALSE;
    try {
        new_blurToBitmap  = mod->get_param_bool("blurToBitmap");
    } catch(...) {}

    int new_bitmapResolution  = 72;
    try {
        new_bitmapResolution = mod->get_param_int("resolution");
    } catch(...) {}

    bool new_areaPage  = true;
    try {
        new_areaPage = mod->get_param_bool("areaPage");
    } catch(...) {}

    bool new_areaDrawing  = true;
    try {
        new_areaDrawing = mod->get_param_bool("areaDrawing");
    } catch(...) {}

    const gchar *new_exportId = NULL;
    try {
        new_exportId = mod->get_param_string("exportId");
    } catch(...) {}

    // Create PS
    {
        gchar * final_name;
        final_name = g_strdup_printf("> %s", filename);
        ret = ps_print_document_to_file(doc, final_name, level, new_textToPath, new_textToLaTeX, new_blurToBitmap, new_bitmapResolution, new_exportId, new_areaDrawing, new_areaPage);
        g_free(final_name);

        if (!ret)
            throw Inkscape::Extension::Output::save_failed();
    }

    // Create LaTeX file (if requested)
    if (new_textToLaTeX) {
        ret = latex_render_document_text_to_file(doc, filename, new_exportId, new_areaDrawing, new_areaPage, false);

        if (!ret)
            throw Inkscape::Extension::Output::save_failed();
    }
}


/**
    \brief  This function calls the output module with the filename
	\param  mod   unused
	\param  doc   Document to be saved
    \param  filename   Filename to save to (probably will end in .ps)
*/
void
CairoEpsOutput::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *filename)
{
    Inkscape::Extension::Extension * ext;
    unsigned int ret;

    ext = Inkscape::Extension::db.get(SP_MODULE_KEY_PRINT_CAIRO_PS);
    if (ext == NULL)
        return;

    const gchar *new_level = NULL;
    int level = CAIRO_PS_LEVEL_2;
    try {
        new_level = mod->get_param_enum("PSlevel");
        if((new_level != NULL) && (g_ascii_strcasecmp("PS3", new_level) == 0))
            level = CAIRO_PS_LEVEL_3;
    } catch(...) {}

    bool new_textToPath  = FALSE;
    try {
        new_textToPath  = mod->get_param_bool("textToPath");
    } catch(...) {}

    bool new_textToLaTeX  = FALSE;
    try {
        new_textToLaTeX  = mod->get_param_bool("textToLaTeX");
    }
    catch(...) {
        g_warning("Parameter <textToLaTeX> might not exist");
    }

    bool new_blurToBitmap  = FALSE;
    try {
        new_blurToBitmap  = mod->get_param_bool("blurToBitmap");
    } catch(...) {}

    int new_bitmapResolution  = 72;
    try {
        new_bitmapResolution = mod->get_param_int("resolution");
    } catch(...) {}

    bool new_areaPage  = true;
    try {
        new_areaPage = mod->get_param_bool("areaPage");
    } catch(...) {}

    bool new_areaDrawing  = true;
    try {
        new_areaDrawing = mod->get_param_bool("areaDrawing");
    } catch(...) {}

    const gchar *new_exportId = NULL;
    try {
        new_exportId = mod->get_param_string("exportId");
    } catch(...) {}

    // Create EPS
    {
        gchar * final_name;
        final_name = g_strdup_printf("> %s", filename);
        ret = ps_print_document_to_file(doc, final_name, level, new_textToPath, new_textToLaTeX, new_blurToBitmap, new_bitmapResolution, new_exportId, new_areaDrawing, new_areaPage, true);
        g_free(final_name);

        if (!ret)
            throw Inkscape::Extension::Output::save_failed();
    }

    // Create LaTeX file (if requested)
    if (new_textToLaTeX) {
        ret = latex_render_document_text_to_file(doc, filename, new_exportId, new_areaDrawing, new_areaPage, false);

        if (!ret)
            throw Inkscape::Extension::Output::save_failed();
    }
}


bool
CairoPsOutput::textToPath(Inkscape::Extension::Print * ext)
{
    return ext->get_param_bool("textToPath");
}

bool
CairoEpsOutput::textToPath(Inkscape::Extension::Print * ext)
{
    return ext->get_param_bool("textToPath");
}

#include "clear-n_.h"

/**
	\brief   A function allocate a copy of this function.

	This is the definition of Cairo PS out.  This function just
	calls the extension system with the memory allocated XML that
	describes the data.
*/
void
CairoPsOutput::init (void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("PostScript") "</name>\n"
			"<id>" SP_MODULE_KEY_PRINT_CAIRO_PS "</id>\n"
			"<param name=\"PSlevel\" gui-text=\"" N_("Restrict to PS level") "\" type=\"enum\" >\n"
				"<_item value='PS3'>" N_("PostScript level 3") "</_item>\n"
#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 5, 2))
                "<_item value='PS2'>" N_("PostScript level 2") "</_item>\n"
#endif
            "</param>\n"
			"<param name=\"textToPath\" gui-text=\"" N_("Convert texts to paths") "\" type=\"boolean\">false</param>\n"
			"<param name=\"textToLaTeX\" gui-text=\"" N_("PS+LaTeX: Omit text in PS, and create LaTeX file") "\" type=\"boolean\">false</param>\n"
			"<param name=\"blurToBitmap\" gui-text=\"" N_("Rasterize filter effects") "\" type=\"boolean\">true</param>\n"
			"<param name=\"resolution\" gui-text=\"" N_("Resolution for rasterization (dpi)") "\" type=\"int\" min=\"1\" max=\"10000\">90</param>\n"
			"<param name=\"areaDrawing\" gui-text=\"" N_("Export area is drawing") "\" type=\"boolean\">true</param>\n"
			"<param name=\"areaPage\" gui-text=\"" N_("Export area is page") "\" type=\"boolean\">true</param>\n"
			"<param name=\"exportId\" gui-text=\"" N_("Limit export to the object with ID") "\" type=\"string\"></param>\n"
			"<output>\n"
				"<extension>.ps</extension>\n"
                                "<mimetype>image/x-postscript</mimetype>\n"
				"<filetypename>" N_("PostScript (*.ps)") "</filetypename>\n"
				"<filetypetooltip>" N_("PostScript File") "</filetypetooltip>\n"
			"</output>\n"
		"</inkscape-extension>", new CairoPsOutput());

	return;
}

/**
	\brief   A function allocate a copy of this function.

	This is the definition of Cairo EPS out.  This function just
	calls the extension system with the memory allocated XML that
	describes the data.
*/
void
CairoEpsOutput::init (void)
{
	Inkscape::Extension::build_from_mem(
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("Encapsulated PostScript") "</name>\n"
			"<id>" SP_MODULE_KEY_PRINT_CAIRO_EPS "</id>\n"
			"<param name=\"PSlevel\" gui-text=\"" N_("Restrict to PS level") "\" type=\"enum\" >\n"
				"<_item value='PS3'>" N_("PostScript level 3") "</_item>\n"
#if (CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 5, 2))
                "<_item value='PS2'>" N_("PostScript level 2") "</_item>\n"
#endif
            "</param>\n"
			"<param name=\"textToPath\" gui-text=\"" N_("Convert texts to paths") "\" type=\"boolean\">false</param>\n"
			"<param name=\"textToLaTeX\" gui-text=\"" N_("EPS+LaTeX: Omit text in EPS, and create LaTeX file") "\" type=\"boolean\">false</param>\n"
			"<param name=\"blurToBitmap\" gui-text=\"" N_("Rasterize filter effects") "\" type=\"boolean\">true</param>\n"
			"<param name=\"resolution\" gui-text=\"" N_("Resolution for rasterization (dpi)") "\" type=\"int\" min=\"1\" max=\"10000\">90</param>\n"
			"<param name=\"areaDrawing\" gui-text=\"" N_("Export area is drawing") "\" type=\"boolean\">true</param>\n"
			"<param name=\"areaPage\" gui-text=\"" N_("Export area is page") "\" type=\"boolean\">true</param>\n"
			"<param name=\"exportId\" gui-text=\"" N_("Limit export to the object with ID") "\" type=\"string\"></param>\n"
			"<output>\n"
				"<extension>.eps</extension>\n"
                                "<mimetype>image/x-e-postscript</mimetype>\n"
				"<filetypename>" N_("Encapsulated PostScript (*.eps)") "</filetypename>\n"
				"<filetypetooltip>" N_("Encapsulated PostScript File") "</filetypetooltip>\n"
			"</output>\n"
		"</inkscape-extension>", new CairoEpsOutput());

	return;
}

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* HAVE_CAIRO_PDF */
