//
//  GradientEditor.m
//  Inxcape
//
//  Created by 张 光建 on 14-10-11.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "GradientEditor.h"
#import "NSColor+SPColor.h"
#import "ZGGradient.h"

#include <gradient-chemistry.h>
#include <document-private.h>
#include <gtk/gtk.h>
#include <macros.h>
#include <glibmm/i18n.h>
#include <display/nr-plain-stuff.h>
#include <xml/node.h>
#include <sp-stop.h>
#include <sp-gradient.h>
#include <svg/css-ostringstream.h>
#include <xml/repr.h>
#include <svg/svg-color.h>
#include <sigc++/sigc++.h>

struct GradientProxy;

@interface GradientEditor () {
    SPGradient *_gradient;
    BOOL blocked;
    GradientProxy *_proxy;
    CGFloat _offset;
}

@property NSMutableDictionary *stopsDic;

- (void)didGradientModified;
- (void)didGradientReleased;
- (void)setSensitive:(BOOL)sensitive;
@end

static guint32 sp_average_color(guint32 c1, guint32 c2, gdouble p);
static void verify_grad(SPGradient *gradient);

struct GradientProxy {
    sigc::connection release_connection;
    sigc::connection modified_connection;
    __weak GradientEditor *ed;
    
    GradientProxy(GradientEditor *owner) {
        ed = owner;
        SPGradient *gradient = ed.gradient;
        release_connection = gradient->connectRelease(sigc::mem_fun(*this, &GradientProxy::on_release));
        modified_connection = gradient->connectModified(sigc::mem_fun(*this, &GradientProxy::on_modified));
    }
    
    virtual ~GradientProxy() {
        release_connection.disconnect();
        modified_connection.disconnect();
    }

    void on_release(SPObject */*object*/)
    {
        [ed didGradientReleased];
    }
    
    void on_modified(SPObject *object, guint /*flags*/)
    {
        [ed didGradientModified];
    }
};


@implementation GradientEditor

- (void)dealloc
{
    if (_proxy) {
        delete _proxy;
        _proxy = NULL;
    }
    _gradient = NULL;
    self.stopsDic = nill;
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (SPGradient *)gradient
{
    return _gradient;
}

- (SPStop *)currentStop
{
    void *p = [[self.stopsDic objectForKey:self.popupStops.titleOfSelectedItem] pointerValue];
    if (p) {
        return SP_STOP(p);
    }
    return NULL;
}

- (void)didGradientModified
{
    if (!blocked) {
        blocked = TRUE;
        [self loadGradient:self.gradient];
        blocked = FALSE;
    }
}

- (void)didGradientReleased
{
    [self loadGradient:NULL];
}

- (void)setSensitive:(BOOL)sensitive
{
    self.popupStops.enabled = sensitive;
    self.colorStop.enabled = sensitive;
    self.sliderOffset.enabled = sensitive;
    self.inputOffset.editable = sensitive;
    self.buttonAddStop.enabled = sensitive;
    self.buttonDelStop.enabled = sensitive;
}

- (void)loadGradient:(SPGradient *)gradient
{
    blocked = TRUE;
    
    SPGradient *old = self.gradient;
    
    if (old != gradient) {
        
        _gradient = gradient;
        
        if (_proxy) {
            delete _proxy;
            _proxy = NULL;
        }
        
        if (gradient) {
            _proxy = new GradientProxy(self);
        }
    }
    
    if (gradient) {
        
        [self setSensitive:TRUE];
        
        gradient->ensureVector();
        
        SPStop *stop = [self currentStop];
        
        /* Fill preview */
        self.imagePreview.image = [ZGGradient gradientImage:gradient];
        
        [self updateStopList:gradient newStop:NULL];
        
        // Once the user edits a gradient, it stops being auto-collectable
        if (SP_OBJECT_REPR(gradient)->attribute("inkscape:collect")) {
            SPDocument *document = SP_OBJECT_DOCUMENT(gradient);
            bool saved = sp_document_get_undo_sensitive(document);
            sp_document_set_undo_sensitive(document, false);
            SP_OBJECT_REPR(gradient)->setAttribute("inkscape:collect", NULL);
            sp_document_set_undo_sensitive(document, saved);
        }
    } else { // no gradient, disable everything
        [self setSensitive:FALSE];
    }
    
    blocked = FALSE;
}

- (IBAction)didAddStop:(id)sender
{
    SPGradient *gradient = self.gradient;
    verify_grad(gradient);
    
    SPStop *stop = [self currentStop];
    
    if (stop == NULL) {
        return;
    }
    
    Inkscape::XML::Node *new_stop_repr = NULL;
    
    SPStop *next = stop->getNextStop();
    
    if (next == NULL) {
        SPStop *prev = stop->getPrevStop();
        if (prev != NULL) {
            next = stop;
            stop = prev;
        }
    }
    
    if (next != NULL) {
        new_stop_repr = SP_OBJECT_REPR(stop)->duplicate(SP_OBJECT_REPR(gradient)->document());
        SP_OBJECT_REPR(gradient)->addChild(new_stop_repr, SP_OBJECT_REPR(stop));
    } else {
        next = stop;
        new_stop_repr = SP_OBJECT_REPR(stop->getPrevStop())->duplicate(SP_OBJECT_REPR(gradient)->document());
        SP_OBJECT_REPR(gradient)->addChild(new_stop_repr, SP_OBJECT_REPR(stop->getPrevStop()));
    }
    
    SPStop *newstop = (SPStop *) SP_OBJECT_DOCUMENT(gradient)->getObjectByRepr(new_stop_repr);
    
    newstop->offset = (stop->offset + next->offset) * 0.5 ;
    
    guint32 const c1 = sp_stop_get_rgba32(stop);
    guint32 const c2 = sp_stop_get_rgba32(next);
    guint32 cnew = sp_average_color(c1, c2, 0.5);
    
    Inkscape::CSSOStringStream os;
    gchar c[64];
    sp_svg_write_color(c, sizeof(c), cnew);
    gdouble opacity = static_cast<gdouble>(SP_RGBA32_A_F(cnew));
    os << "stop-color:" << c << ";stop-opacity:" << opacity <<";";
    SP_OBJECT_REPR (newstop)->setAttribute("style", os.str().c_str());
    sp_repr_set_css_double( SP_OBJECT_REPR(newstop), "offset", (double)newstop->offset);
    
    [self loadGradient:gradient];
    Inkscape::GC::release(new_stop_repr);
    [self updateStopList:gradient newStop:newstop];
    self.sliderOffset.enabled = TRUE;
    self.inputOffset.enabled = TRUE;

    sp_document_done(SP_OBJECT_DOCUMENT(gradient), SP_VERB_CONTEXT_GRADIENT,
                     _("Add gradient stop"));
}

- (IBAction)didDeleteStop:(id)sender
{
    SPGradient *gradient = self.gradient;
    SPStop *stop = [self currentStop];

    if (!stop) {
        return;
    }
    
    if (gradient->vector.stops.size() > 2) { // 2 is the minimum
        
        // if we delete first or last stop, move the next/previous to the edge
        if (stop->offset == 0) {
            SPStop *next = stop->getNextStop();
            if (next) {
                next->offset = 0;
                sp_repr_set_css_double(SP_OBJECT_REPR(next), "offset", 0);
            }
        } else if (stop->offset == 1) {
            SPStop *prev = stop->getPrevStop();
            if (prev) {
                prev->offset = 1;
                sp_repr_set_css_double(SP_OBJECT_REPR(prev), "offset", 1);
            }
        }
        
        SP_OBJECT_REPR(gradient)->removeChild(SP_OBJECT_REPR(stop));
        [self loadGradient:gradient];
        [self updateStopList:gradient newStop:NULL];
        sp_document_done(SP_OBJECT_DOCUMENT(gradient), SP_VERB_CONTEXT_GRADIENT,
                         _("Delete gradient stop"));
    }
}

- (IBAction)didChangeStopColor:(id)sender
{
    SPGradient *gradient, *ngr;
    
    if (blocked) {
        return;
    }
    
    gradient = self.gradient;
    if (!gradient) {
        return;
    }
    
    blocked = TRUE;
    
    ngr = sp_gradient_ensure_vector_normalized(gradient);
    if (ngr != gradient) {
        /* Our master gradient has changed */
        [self loadGradient:ngr];
    }
    
    ngr->ensureVector();
    
    SPStop *stop = [self currentStop];
    
    SPColor color = *[[self.colorStop color] spColor];
    float alpha = self.colorStop.color.alphaComponent;
    
    sp_repr_set_css_double(SP_OBJECT_REPR(stop), "offset", stop->offset);
    Inkscape::CSSOStringStream os;
    os << "stop-color:" << color.toString() << ";stop-opacity:" << static_cast<gdouble>(alpha) <<";";
    SP_OBJECT_REPR(stop)->setAttribute("style", os.str().c_str());
    // g_snprintf(c, 256, "stop-color:#%06x;stop-opacity:%g;", rgb >> 8, static_cast<gdouble>(alpha));
    //SP_OBJECT_REPR(stop)->setAttribute("style", c);
    /* zhangguangjian , undo unvailable
    sp_document_done(SP_OBJECT_DOCUMENT(ngr), SP_VERB_CONTEXT_GRADIENT,
                     _("Change gradient stop color"));
    */
    blocked = FALSE;
}

- (void)setOffset:(CGFloat)offset
{
    if (!blocked && offset != _offset) {
        blocked = TRUE;
        _offset = offset;
        
        if ( self.currentStop ) {
            SPStop *stop = self.currentStop;
            
            stop->offset = offset;
            sp_repr_set_css_double(SP_OBJECT_REPR(stop), "offset", stop->offset);
            
            sp_document_maybe_done(SP_OBJECT_DOCUMENT(stop), "gradient:stop:offset", SP_VERB_CONTEXT_GRADIENT,
                                   _("Change gradient stop offset"));
        }
        blocked = FALSE;
        [self didGradientModified];
    }
}

- (CGFloat)offset
{
    return _offset;
}

- (void)didSelectStop:(id)sender
{
    SPStop *stop = [self currentStop];
    if (!stop) {
        return;
    }
    
    blocked = TRUE;
    
    SPColor c = stop->getEffectiveColor();
    NSColor *clr = [NSColor colorWithSPColor:&c alpha:stop->opacity];
    self.colorStop.color = clr;
    
    bool isEndStop = false;
    double lower = 0.f;
    double upper = 0.f;
    
    NSNumberFormatter *fmt = self.inputOffset.formatter;
    
    SPStop *prev = NULL;
    prev = stop->getPrevStop();
    if (prev != NULL )  {
        lower = prev->offset;
    } else {
        isEndStop = true;
        lower = 0.0;
    }
    
    SPStop *next = NULL;
    next = stop->getNextStop();
    if (next != NULL ) {
        upper = next->offset;
    } else {
        isEndStop = true;
        upper = 1.0;
    }

    [fmt setMinimum:[NSNumber numberWithDouble:lower]];
    [fmt setMaximum:[NSNumber numberWithDouble:upper]];
    [self.sliderOffset setMaxValue:upper];
    [self.sliderOffset setMinValue:lower];
    
    //fixme: does this work on all possible input gradients?
    if (!isEndStop) {
        self.sliderOffset.enabled = TRUE;
        self.inputOffset.editable = TRUE;
    } else {
        self.sliderOffset.enabled = FALSE;
        self.inputOffset.editable = FALSE;
    }
    
    [self willChangeValueForKey:@"offset"];
    _offset = stop->offset;
    [self didChangeValueForKey:@"offset"];
    
    blocked = FALSE;
}

- (void)updateStopList:(SPGradient *)gradient newStop:(SPStop *)new_stop
{
    if (!SP_IS_GRADIENT(gradient)) {
        return;
    }
    
    blocked = TRUE;
    
    NSString *savedSelected = [self.popupStops titleOfSelectedItem];
    
    /* Clear old menu, if there is any */
    [self.popupStops.menu removeAllItems];
    self.stopsDic = [NSMutableDictionary dictionary];
    /* Create new menu widget */
    GSList *sl = NULL;
    if ( gradient->hasStops() ) {
        for ( SPObject *ochild = sp_object_first_child(SP_OBJECT(gradient)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT(ochild) ) {
            if (SP_IS_STOP(ochild)) {
                sl = g_slist_append(sl, ochild);
            }
        }
    }
    if (!sl) {
        [self.popupStops setStringValue:@"No stops in gradient"];
        self.popupStops.enabled = FALSE;
    } else {
        
        for (; sl != NULL; sl = sl->next){
            if (SP_IS_STOP(sl->data)){
                Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) sl->data);
                
                SPStop *stop = SP_STOP(sl->data);
                NSMenuItem *item = [[NSMenuItem alloc] init];
                item.target = self;
                item.action = @selector(didSelectStop:);
                item.title = [NSString stringWithUTF8String:repr->attribute("id")];
                
                [self.popupStops.menu addItem:item];
                [self.stopsDic setObject:[NSValue valueWithPointer:stop] forKey:item.title];
            }
        }
        
        self.popupStops.enabled = TRUE;
    }
    
    /* Set history */
    if (new_stop == NULL) {
        NSMenuItem *item = [self.popupStops itemWithTitle:savedSelected];
        if (item) {
            [self.popupStops selectItem:item];
        } else {
            [self.popupStops selectItemAtIndex:0];
        }
    } else {
        Inkscape::XML::Node *repr = SP_OBJECT_REPR(new_stop);
        NSString *title = [NSString stringWithUTF8String:repr->attribute("id")];
        [self.popupStops selectItemWithTitle:title];
    }
    [self didSelectStop:nill];
    
    blocked = FALSE;
}

@end


static guint32 sp_average_color(guint32 c1, guint32 c2, gdouble p)
{
    guint32 r = (guint32) (SP_RGBA32_R_U(c1) * p + SP_RGBA32_R_U(c2) * (1 - p));
    guint32 g = (guint32) (SP_RGBA32_G_U(c1) * p + SP_RGBA32_G_U(c2) * (1 - p));
    guint32 b = (guint32) (SP_RGBA32_B_U(c1) * p + SP_RGBA32_B_U(c2) * (1 - p));
    guint32 a = (guint32) (SP_RGBA32_A_U(c1) * p + SP_RGBA32_A_U(c2) * (1 - p));
    
    return SP_RGBA32_U_COMPOSE(r, g, b, a);
}

static void verify_grad(SPGradient *gradient)
{
    int i = 0;
    SPStop *stop = NULL;
    /* count stops */
    for ( SPObject *ochild = sp_object_first_child(SP_OBJECT(gradient)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if (SP_IS_STOP(ochild)) {
            i++;
            stop = SP_STOP(ochild);
        }
    }
    
    Inkscape::XML::Document *xml_doc;
    xml_doc = SP_OBJECT_REPR(gradient)->document();
    
    if (i < 1) {
        Inkscape::CSSOStringStream os;
        os << "stop-color: #000000;stop-opacity:" << 1.0 << ";";
        
        Inkscape::XML::Node *child;
        
        child = xml_doc->createElement("svg:stop");
        sp_repr_set_css_double(child, "offset", 0.0);
        child->setAttribute("style", os.str().c_str());
        SP_OBJECT_REPR(gradient)->addChild(child, NULL);
        Inkscape::GC::release(child);
        
        child = xml_doc->createElement("svg:stop");
        sp_repr_set_css_double(child, "offset", 1.0);
        child->setAttribute("style", os.str().c_str());
        SP_OBJECT_REPR(gradient)->addChild(child, NULL);
        Inkscape::GC::release(child);
    }
    if (i < 2) {
        sp_repr_set_css_double(SP_OBJECT_REPR(stop), "offset", 0.0);
        Inkscape::XML::Node *child = SP_OBJECT_REPR(stop)->duplicate(SP_OBJECT_REPR(gradient)->document());
        sp_repr_set_css_double(child, "offset", 1.0);
        SP_OBJECT_REPR(gradient)->addChild(child, SP_OBJECT_REPR(stop));
        Inkscape::GC::release(child);
    }
}