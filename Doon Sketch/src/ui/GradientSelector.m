//
//  GradientSelector.m
//  Inxcape
//
//  Created by 张 光建 on 14-10-11.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#include "GradientSelector.h"
#include "ZGGradient.h"
#include "ZGDocument.h"
#include "GradientEditor.h"

#include <gtk/gtk.h>
#include <macros.h>
#include <glibmm/i18n.h>
#include <inkscape.h>
#include <document-private.h>
#include <gradient-chemistry.h>
#include <desktop-handles.h>

#include <xml/repr.h>

#include <svg/css-ostringstream.h>
#include <sp-stop.h>

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>
#include <gc.h>

@interface GradientSelector () {
    SPGradientSpread _spread;
}

@property BOOL swatched;
@property NSMutableArray *gradients;
@property ZGDocument *document;
@property ZGGradient *gr;
@property GradientEditor *ge;

- (void)rebuildGui;
- (void)didSelectGradientMenuItem:(id)sender;

@end


@implementation GradientSelector

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        self.units = SP_GRADIENT_UNITS_USERSPACEONUSE;
        self.spread = SP_GRADIENT_SPREAD_PAD;
        self.ge = [[GradientEditor alloc] initWithWindowNibName:@"GradientEditor"];
    }
    return self;
}

- (void)dealloc
{
    self.ge = nill;
}

- (void)awakeFromNib
{
    [self rebuildGui];
}

- (void)rebuildGui
{
    NSMenu *menu = [[NSMenu alloc] init];
    GSList *gl = NULL;
    self.gradients = [NSMutableArray array];
    NSUInteger pos = 0;
    
    if (self.gr) {
        const GSList *gradients = sp_document_get_resource_list(SP_OBJECT_DOCUMENT(self.gr.spGradient), "gradient");
        
        for (const GSList *curr = gradients; curr; curr = curr->next) {
            SPGradient* grad = SP_GRADIENT(curr->data);
            if ( grad->hasStops() && (grad->isSwatch() == self.swatched) ) {
                gl = g_slist_append(gl, curr->data);
            }
        }
    }
    
    if (gl) {
        while (gl) {
            SPGradient* grad = SP_GRADIENT(gl->data);
            ZGGradient *gr;
            
            if (grad == self.gr.spGradient) {
                gr = self.gr;
            } else {
                gr = [[ZGGradient alloc] initWithSPGradient:grad];
            }
            
            [self.gradients addObject:gr];
            
            NSMenuItem *mi = [[NSMenuItem alloc] init];
            mi.target = self;
            mi.action = @selector(didSelectGradientMenuItem:);
            mi.tag = [self.gradients indexOfObject:gr];
            [mi setTitle:[NSString stringWithUTF8String:grad->getId()]];
            [menu addItem:mi];
            gl = gl->next;
        }
        
        g_slist_free(gl);
    } else {
        [menu setTitle:@"Empty"];
    }
    
    [self.vectorsButton setMenu:menu];
    
    if (self.gr) {
        NSUInteger idx = [self.gradients indexOfObject:self.gr];
        [self.vectorsButton selectItemAtIndex:idx];
        self.imagePreview.image = [ZGGradient gradientImage:self.gr.spGradient];
        g_debug("Selected vector index = %d", idx);
    }
}

- (void)setGradient:(SPGradient *)gr toDocument:(SPDocument *)doc
{
    static BOOL suppress = FALSE;
    
    g_debug("set vector : %s[%p] document:%p", gr ? gr->getId() : "[null]", gr, doc);
    
    g_return_if_fail(!gr || (doc != NULL));
    g_return_if_fail(!gr || SP_IS_GRADIENT(gr));
    g_return_if_fail(!gr || (SP_OBJECT_DOCUMENT(gr) == doc));
    g_return_if_fail(!gr || gr->hasStops());

    if (doc != self.document.spDocument) {
        /* Disconnect signals */
        self.document = nill;
        self.gr = nill;
        
        // Connect signals
        if (doc) {
            self.document = [[ZGDocument alloc] initWithSPDocument:doc];
            self.document.delegate = self;
        }
        if (gr) {
            self.gr = [[ZGGradient alloc] initWithSPGradient:gr];
            self.gr.delegate = self;
        }
        
        [self rebuildGui];
        
        if (!suppress && [self.delegate respondsToSelector:@selector(didGradientSelectorChanged:)]) {
            [self.delegate didGradientSelectorChanged:self];
        }
    } else if (gr != self.gr.spGradient) {
        suppress = TRUE;
        [self setGradient:NULL toDocument:NULL];
        [self setGradient:gr toDocument:doc];
        suppress = FALSE;
        if ([self.delegate respondsToSelector:@selector(didGradientSelectorChanged:)]) {
            [self.delegate didGradientSelectorChanged:self];
        }
    }
}

- (void)setVector:(SPGradient *)vector toDocument:(SPDocument *)doc
{
    g_return_if_fail(!vector || SP_IS_GRADIENT(vector));
    g_return_if_fail(!vector || (SP_OBJECT_DOCUMENT(vector) == doc));
    
    if (vector && !vector->hasStops()) {
        return;
    }
    
    [self setGradient:vector toDocument:doc];
    
    if (vector) {
        if ((self.mode == SPGradientSelector::MODE_SWATCH) && vector->isSwatch() ) {
            if ( vector->isSolid() ) {
                //hide
            } else {
                //show all
            }
        }
        
        self.editButton.enabled = TRUE;
        self.dupButton.enabled = TRUE;
    } else {
        self.editButton.enabled = FALSE;
        self.dupButton.enabled = FALSE;
    }
}

- (void)didSelectGradientMenuItem:(id)sender
{
    g_assert([sender isKindOfClass:[NSMenuItem class]]);
    
    SPGradient *gr, *norm;
    NSMenuItem *mi = (NSMenuItem *)sender;
    
    gr = [[self.gradients objectAtIndex:[mi tag]] spGradient];
    
    /* Hmmm... bad things may happen here, if actual gradient is something new */
    /* Namely - menuitems etc. will be fucked up */
    /* Hmmm - probably we can just re-set it as menuitem data (Lauris) */
    
    //g_print("SPGradientVectorSelector: gradient %s activated\n", SP_OBJECT_ID(gr));
    //g_message("Setting to gradient %p   swatch:%d   solid:%d", gr, gr->isSwatch(), gr->isSolid());
    
    norm = sp_gradient_ensure_vector_normalized(gr);
    
    if (norm != gr) {
        //g_print("SPGradientVectorSelector: become %s after normalization\n", SP_OBJECT_ID(norm));
        /* But be careful that we do not have gradient saved anywhere else */
        ZGGradient *p = [[ZGGradient alloc] initWithSPGradient:norm];
//        p.delegate = self;
        [self.gradients replaceObjectAtIndex:mi.tag withObject:p];
    }

    [self setVector:norm];
    
    if (norm != gr) {
        /* We do extra undo push here */
        /* If handler has already done it, it is just NOP */
        // FIXME: looks like this is never a valid undo step, consider removing this
        sp_document_done(SP_OBJECT_DOCUMENT(norm), SP_VERB_CONTEXT_GRADIENT,
                         /* TODO: annotate */ "gradient-vector.cpp:350");
    }
}

- (void)setVector:(SPGradient *)gr
{
    static gboolean blocked = FALSE;
    
    if (!blocked) {
        blocked = TRUE;
        gr = sp_gradient_ensure_vector_normalized (gr);
        [self setVector:gr toDocument:((gr) ? SP_OBJECT_DOCUMENT (gr) : 0)];
        
        if ([self.delegate respondsToSelector:@selector(didGradientSelectorChanged:)]) {
            [self.delegate didGradientSelectorChanged:self];
        }
        blocked = FALSE;
    }
}

- (SPGradient *)vector
{
    return self.gr.spGradient;
}

- (void)didGradientReleased:(ZGGradient *)gr
{
//    g_debug("Release vector : %s[%p], doc = %p", gr.spGradient->getId(), gr.spGradient, SP_OBJECT_DOCUMENT(gr.spGradient));
    self.gr = nill;
    [self rebuildGui];
}

- (void)didDocumentDefsRelease:(ZGDocument *)doc
{
    self.document = nill;
    self.gr = nill;
    
    /* Rebuild GUI */
    [self rebuildGui];
}

- (void)didDocumentDefs:(ZGDocument *)doc modifiedWithFlags:(guint)flags
{
    [self rebuildGui];
}

- (IBAction)didDuplicateClicked:(id)sender
{
    if (!self.document.spDocument) {
        return;
    }
    
    g_debug("Current _total_hrefcount = %d", self.gr.spGradient->_total_hrefcount);
    
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(self.document.spDocument);
    Inkscape::XML::Node *repr = NULL;
    
    if (self.gr)
        repr = SP_OBJECT_REPR (self.gr.spGradient)->duplicate(xml_doc);
    else {
        repr = xml_doc->createElement("svg:linearGradient");
        Inkscape::XML::Node *stop = xml_doc->createElement("svg:stop");
        stop->setAttribute("offset", "0");
        stop->setAttribute("style", "stop-color:#000;stop-opacity:1;");
        repr->appendChild(stop);
        Inkscape::GC::release(stop);
        stop = xml_doc->createElement("svg:stop");
        stop->setAttribute("offset", "1");
        stop->setAttribute("style", "stop-color:#fff;stop-opacity:1;");
        repr->appendChild(stop);
        Inkscape::GC::release(stop);
    }
    
    SP_OBJECT_REPR (SP_DOCUMENT_DEFS (self.document.spDocument))->addChild(repr, NULL);
    
    SPGradient * gr = (SPGradient *) self.document.spDocument->getObjectByRepr(repr);
    g_debug("Duplicated vector : %s[%p], doc = %p", gr->getId(), gr, SP_OBJECT_DOCUMENT(gr));
    g_debug("Current vector : %s[%p], doc = %p", self.gr.spGradient->getId(), self.gr.spGradient, SP_OBJECT_DOCUMENT(self.gr.spGradient));
    
    g_debug("Current _total_hrefcount = %d", self.gr.spGradient->_total_hrefcount);
    
    [self setVector:gr toDocument:self.document.spDocument];
    
    Inkscape::GC::release(repr);
}

- (IBAction)didEditClicked:(id)sender
{
    [self.ge showWindow:nill];
    [self.ge loadGradient:self.vector];
}

- (void)setSpread:(SPGradientSpread)spread
{
    if (_spread != spread) {
        _spread = spread;
        if ([self.delegate respondsToSelector:@selector(didGradientSelectorChanged:)]) {
            [self.delegate didGradientSelectorChanged:self];
        }
    }
}

- (SPGradientSpread)spread
{
    return _spread;
}

@end
