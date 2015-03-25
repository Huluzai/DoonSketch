/** @file
 * @brief Fill and Stroke dialog
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Gustav Broberg <broberg@kth.se>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004--2007 Authors
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_FILL_AND_STROKE_H
#define INKSCAPE_UI_DIALOG_FILL_AND_STROKE_H

#include <gtkmm/adjustment.h>
#include <gtkmm/alignment.h>
#include <gtkmm/notebook.h>
#include <gtkmm/scale.h>
#include <gtkmm/spinbutton.h>
#include <glibmm/i18n.h>

#include "ui/widget/panel.h"
#include "ui/widget/notebook-page.h"
#include "ui/widget/object-composite-settings.h"
#include "ui/dialog/desktop-tracker.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

class FillAndStroke : public UI::Widget::Panel {
public:
    FillAndStroke();
    virtual ~FillAndStroke();

    static FillAndStroke &getInstance() { return *new FillAndStroke(); }


    virtual void setDesktop(SPDesktop *desktop);

    void selectionChanged(Inkscape::Application *inkscape,
                          Inkscape::Selection *selection);

    void showPageFill();
    void showPageStrokePaint();
    void showPageStrokeStyle();

protected:
    Gtk::Notebook   _notebook;

    UI::Widget::NotebookPage    _page_fill;
    UI::Widget::NotebookPage    _page_stroke_paint;
    UI::Widget::NotebookPage    _page_stroke_style;

    UI::Widget::StyleSubject::Selection _subject;
    UI::Widget::ObjectCompositeSettings _composite_settings;

    Gtk::HBox &_createPageTabLabel(const Glib::ustring &label,
                                   const char *label_image);

    void _layoutPageFill();
    void _layoutPageStrokePaint();
    void _layoutPageStrokeStyle();

private:
    FillAndStroke(FillAndStroke const &d);
    FillAndStroke& operator=(FillAndStroke const &d);

    void setTargetDesktop(SPDesktop *desktop);

    DesktopTracker deskTrack;
    SPDesktop *targetDesktop;
    Gtk::Widget *fillWdgt;
    Gtk::Widget *strokeWdgt;
    sigc::connection desktopChangeConn;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape


#endif // INKSCAPE_UI_DIALOG_FILL_AND_STROKE_H

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
