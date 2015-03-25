//
//  CompositeSettingsPage.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/9.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "CompositeSettingsPage.h"

#include <glibmm/i18n.h>
#include <desktop-handles.h>
#include <desktop-style.h>
#include <document.h>
#include <filter-chemistry.h>
#include <inkscape.h>
#include <selection.h>
#include <style.h>
#include <sp-item.h>
#include <svg/css-ostringstream.h>
#include <verbs.h>
#include <xml/repr.h>
#include <widgets/icon.h>
#include <ui/icon-names.h>
#include <ui/widget/object-composite-settings.h>
#include <display/sp-canvas.h>
#include <sigc++/sigc++.h>
#include <verbs.h>

using namespace Inkscape;

struct SubjectProxy;

@interface CompositeSettingsPage () {
    BOOL _blocked;
    UI::Widget::StyleSubject *_subject;
}
@property (readonly) UI::Widget::StyleSubject *subject;
@property SubjectProxy *proxy;
- (void)didSubjectChanged;
@end

struct SubjectProxy {
    sigc::connection _subject_changed;
    __weak CompositeSettingsPage *zPage;
    
    SubjectProxy (id owner) {
        zPage = owner;
        _subject_changed = zPage.subject->connectChanged(sigc::mem_fun(*this, &SubjectProxy::on_subject_changed));
    }
    virtual ~SubjectProxy() {
        _subject_changed.disconnect();
        zPage = nill;
    }
    void on_subject_changed() {
        [zPage didSubjectChanged];
    }
};

@implementation CompositeSettingsPage

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        _subject = new UI::Widget::StyleSubject::Selection();
    }
    return self;
}

- (void)dealloc
{
    if (self.proxy) {
        delete self.proxy;
        self.proxy = NULL;
    }
    delete _subject;
    _subject = NULL;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    self.proxy = new SubjectProxy(self);
    _subject->setDesktop(self.desktop);
    [self didSubjectChanged];
}

- (void)setDesktop:(struct SPDesktop *)desktop
{
    [super setDesktop:desktop];
    _subject->setDesktop(desktop);
}

- (UI::Widget::StyleSubject *)subject
{
    return _subject;
}

- (IBAction)didChangeBlur:(id)sender
{
    double blur = [sender doubleValue];
    if (sender == self.inputBlur) {
        self.sliderBlur.doubleValue = blur;
    }
    
    if (!_subject) {
        return;
    }
    
    SPDesktop *desktop = _subject->getDesktop();
    if (!desktop) {
        return;
    }
    SPDocument *document = sp_desktop_document (desktop);
    
    if (_blocked)
        return;
    _blocked = true;
    
    // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in crash 1580903
    //sp_canvas_force_full_redraw_after_interruptions(sp_desktop_canvas(desktop), 0);
    
    Geom::OptRect bbox = _subject->getBounds(SPItem::GEOMETRIC_BBOX);
    double radius;
    if (bbox) {
        double perimeter = bbox->dimensions()[Geom::X] + bbox->dimensions()[Geom::Y];   // fixme: this is only half the perimeter, is that correct?
        radius = blur * perimeter / 400;
    } else {
        radius = 0;
    }
    
    const Glib::ustring blendmode = "normal";
    
    //apply created filter to every selected item
    for (Inkscape::UI::Widget::StyleSubject::iterator i = _subject->begin() ; i != _subject->end() ; ++i ) {
        if (!SP_IS_ITEM(*i)) {
            continue;
        }
        
        SPItem * item = SP_ITEM(*i);
        SPStyle *style = SP_OBJECT_STYLE(item);
        g_assert(style != NULL);
        
        if (blendmode != "normal") {
            SPFilter *filter = new_filter_simple_from_item(document, item, blendmode.c_str(), radius);
            sp_style_set_property_url(item, "filter", filter, false);
        } else {
            sp_style_set_property_url(item, "filter", NULL, false);
        }
        
        if (radius == 0 && item->style->filter.set
            && filter_is_single_gaussian_blur(SP_FILTER(item->style->getFilter()))) {
            remove_filter(item, false);
        }
        else if (radius != 0) {
            SPFilter *filter = modify_filter_gaussian_blur_from_item(document, item, radius);
            sp_style_set_property_url(item, "filter", filter, false);
        }
        
        //request update
        item->requestDisplayUpdate(( SP_OBJECT_MODIFIED_FLAG |
                                    SP_OBJECT_STYLE_MODIFIED_FLAG ));
    }
    
    sp_document_maybe_done (document, "composite:blur", SP_VERB_NONE,
                            _("Change blur"));
    
    // resume interruptibility
    //sp_canvas_end_forced_full_redraws(sp_desktop_canvas(desktop));
    
    _blocked = false;
}

- (IBAction)didChangeOpacity:(id)sender
{
    double opacity = [sender doubleValue];
    if (sender == self.inputOpacity) {
        self.sliderOpacity.doubleValue = opacity;
    }
    
    if (!_subject) {
        return;
    }
    
    SPDesktop *desktop = _subject->getDesktop();
    if (!desktop) {
        return;
    }
    
    if (_blocked)
        return;
    _blocked = true;
    
    // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in crash 1580903
    // UPDATE: crash fixed in GTK+ 2.10.7 (bug 374378), remove this as soon as it's reasonably common
    // (though this only fixes the crash, not the multiple change events)
    //sp_canvas_force_full_redraw_after_interruptions(sp_desktop_canvas(desktop), 0);
    
    SPCSSAttr *css = sp_repr_css_attr_new ();
    
    Inkscape::CSSOStringStream os;
    os << CLAMP (opacity / 100, 0.0, 1.0);
    sp_repr_css_set_property (css, "opacity", os.str().c_str());
    
    _subject->setCSS(css);
    
    sp_repr_css_attr_unref (css);
    
    sp_document_maybe_done (sp_desktop_document (desktop), "composite:opacity", SP_VERB_NONE,
                            _("Change opacity"));
    
    // resume interruptibility
    //sp_canvas_end_forced_full_redraws(sp_desktop_canvas(desktop));
    
    _blocked = false;
}

- (void)enableOpacity:(BOOL)enabled
{
    self.inputOpacity.enabled = enabled;
    self.sliderOpacity.enabled = enabled;
}

- (void)enableBlur:(BOOL)enabled
{
    self.inputBlur.enabled = enabled;
    self.sliderBlur.enabled = enabled;
}

- (void)didSubjectChanged
{
    if (!_subject) {
        return;
    }
    
    SPDesktop *desktop = _subject->getDesktop();
    if (!desktop) {
        return;
    }
    
    if (_blocked)
        return;
    _blocked = true;
    
    SPStyle *query = sp_style_new (sp_desktop_document(desktop));
    int result = _subject->queryStyle(query, QUERY_STYLE_PROPERTY_MASTEROPACITY);
    
    switch (result) {
        case QUERY_STYLE_NOTHING:
            [self enableOpacity:FALSE];
            // gtk_widget_set_sensitive (opa, FALSE);
            break;
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_AVERAGED: // TODO: treat this slightly differently
        case QUERY_STYLE_MULTIPLE_SAME:
            [self enableOpacity:TRUE];
            double val = (100 * SP_SCALE24_TO_FLOAT(query->opacity.value));
            [self.inputOpacity setDoubleValue:val];
            [self.sliderOpacity setDoubleValue:val];
            break;
    }
    
    //query now for current filter mode and average blurring of selection
    const int blend_result = _subject->queryStyle(query, QUERY_STYLE_PROPERTY_BLEND);
    switch(blend_result) {
        case QUERY_STYLE_NOTHING:
            [self enableBlur:FALSE];
            break;
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_SAME:
            [self enableBlur:TRUE];
//            _fe_cb.set_blend_mode(query->filter_blend_mode.value);
//            _fe_cb.set_sensitive(true);
            break;
        case QUERY_STYLE_MULTIPLE_DIFFERENT:
            // TODO: set text
//            _fe_cb.set_sensitive(false);
            [self enableBlur:FALSE];
            break;
    }

    if(blend_result == QUERY_STYLE_SINGLE || blend_result == QUERY_STYLE_MULTIPLE_SAME) {
        int blur_result = _subject->queryStyle(query, QUERY_STYLE_PROPERTY_BLUR);
        switch (blur_result) {
            case QUERY_STYLE_NOTHING: //no blurring
                [self enableBlur:FALSE];
                break;
            case QUERY_STYLE_SINGLE:
            case QUERY_STYLE_MULTIPLE_AVERAGED:
            case QUERY_STYLE_MULTIPLE_SAME:
                Geom::OptRect bbox = _subject->getBounds(SPItem::GEOMETRIC_BBOX);
                if (bbox) {
                    double perimeter = bbox->dimensions()[Geom::X] + bbox->dimensions()[Geom::Y];   // fixme: this is only half the perimeter, is that correct?
                    [self enableBlur:TRUE];
                    //update blur widget value
                    float radius = query->filter_gaussianBlur_deviation.value;
                    float percent = radius * 400 / perimeter; // so that for a square, 100% == half side
                    self.inputBlur.doubleValue = percent;
                    self.sliderBlur.doubleValue = percent;
                }
                break;
        }
    }
    
    sp_style_unref(query);
    
    _blocked = false;
}

@end
