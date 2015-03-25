//
//  ExportController.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/9.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ExportController.h"
#import "ZGDesktop.h"

#import <desktop-handles.h>
#import <desktop.h>
#import <document.h>
#import <2geom/geom.h>
#import <unit-constants.h>
#import <glib/gi18n.h>
#import <helper/png-write.h>
#import <sp-namedview.h>
#import <xml/repr.h>

#define DPI_BASE PX_PER_IN

/// Called for every progress iteration
static unsigned int sp_export_progress_callback (float value, void *data);

@interface ExportController () {
    NSInteger _width;
    NSInteger _height;
    CGFloat _dpi;
}
@property BOOL userCanceld;

@end


@implementation ExportController

- (void)dealloc
{
    self.zDt = nill;
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}
/*
{
    [self performSelectorInBackground:@selector(_exportDocumentToFile:) withObject:anUrl];
}
*/

- (void)exportDocument:(SPDocument *)doc toFile:(NSURL*)anUrl
{
    self.userCanceld = FALSE;
    [self performSelectorInBackground:@selector(_exportDocumentToFile:) withObject:anUrl];
}

- (void)_exportDocumentToFile:(NSURL*)anUrl
{
    if (!self.zDt.spDesktop) return;
    
    SPNamedView *nv = sp_desktop_namedview(self.zDt.spDesktop);
    gchar const *filename = [[anUrl path] UTF8String];
    
    float const x0 = self.exportRect.origin.x;
    float const y0 = self.exportRect.origin.y;
    float const x1 = CGRectGetMaxX(self.exportRect);
    float const y1 = CGRectGetMaxY(self.exportRect);
    float const xdpi = self.dpi;
    float const ydpi = self.dpi;
    unsigned long int const width = self.width;
    unsigned long int const height = self.height;
    
    if (filename == NULL || *filename == '\0') {
        g_error(_("You have to enter a filename"));
        return;
    }
    
    if (!((x1 > x0) && (y1 > y0) && (width > 0) && (height > 0))) {
        g_error(_("The chosen area to be exported is invalid"));
        return;
    }
    
    // make sure that .png is the extension of the file:
    gchar * filename_ext = g_strdup(filename);
    
    gchar *path = g_strdup(filename);
    
    gchar *fn = g_path_get_basename (path);
    self.labelProgMsg.stringValue = [NSString stringWithFormat:@"Exporting %s (%lu x %lu)", fn, width, height];
    g_free (fn);
    
    /* Do export */
    if (!sp_export_png_file (sp_desktop_document(self.zDt.spDesktop), path,
                             Geom::Rect(Geom::Point(x0, y0), Geom::Point(x1, y1)), width, height, xdpi, ydpi,
                             nv->pagecolor,
                             sp_export_progress_callback, (__bridge void *)self, TRUE,
                             NULL
                             )) {
        g_error(_("Could not export to filename %s.\n"), path);
    }
    
    /* Reset the filename so that it can be changed again by changing
     selections and all that */
#if 0
    Inkscape::XML::Node * repr = sp_document_repr_root(doc);
    bool modified = false;
    const gchar * temp_string;

    bool saved = sp_document_get_undo_sensitive(doc);
    sp_document_set_undo_sensitive(doc, false);
    
    temp_string = repr->attribute("inkscape:export-filename");
    if (temp_string == NULL || strcmp(temp_string, filename_ext)) {
        repr->setAttribute("inkscape:export-filename", filename_ext);
        modified = true;
    }
    temp_string = repr->attribute("inkscape:export-xdpi");
    if (temp_string == NULL || xdpi != atof(temp_string)) {
        sp_repr_set_svg_double(repr, "inkscape:export-xdpi", xdpi);
        modified = true;
    }
    temp_string = repr->attribute("inkscape:export-ydpi");
    if (temp_string == NULL || xdpi != atof(temp_string)) {
        sp_repr_set_svg_double(repr, "inkscape:export-ydpi", ydpi);
        modified = true;
    }
    sp_document_set_undo_sensitive(doc, saved);
    
    if (modified) {
        doc->setModifiedSinceSave();
    }
#endif
    g_free (filename_ext);
    g_free (path);
    /*
    [self.delegate exportControllerDidComplete:self];
    [self.delegate performSelectorOnMainThread:@selector(exportControllerDidComplete:)
                                    withObject:self
                                 waitUntilDone:TRUE];*/
    
    [self.delegate performSelectorOnMainThread:@selector(exportControllerDidComplete:) withObject:self waitUntilDone:FALSE];
}

- (void)didChangedProgress:(NSNumber *)value
{
    self.proExport.doubleValue = value.doubleValue * 100;
    self.window.title = [NSString stringWithFormat:@"Exporting ... %.1f%%", value.doubleValue * 100];
    self.labelProgMsg.stringValue = self.window.title;
}

- (void)prepareOptionView
{
    self.dpi = 90.f;
    self.width = self.exportRect.size.width;
    self.height = self.exportRect.size.height;
}

- (CGRect)exportRect
{
    float const x0 = 0.f;
    float const y0 = 0.f;
    float const x1 = sp_document_width(sp_desktop_document(self.zDt.spDesktop));
    float const y1 = sp_document_height(sp_desktop_document(self.zDt.spDesktop));
    return CGRectMake(x0, y0, x1 - x0, y1 - y0);
}

- (void)setDpi:(CGFloat)dpi
{
    if (dpi != _dpi) {
        _dpi = dpi;
        [self willChangeValueForKey:@"width"];
        _width = floor (self.exportRect.size.width * dpi / DPI_BASE + 0.5);
        [self didChangeValueForKey:@"width"];
        [self willChangeValueForKey:@"height"];
        _height = floor (self.exportRect.size.height * dpi / DPI_BASE + 0.5);
        [self didChangeValueForKey:@"height"];
    }
}

- (CGFloat)dpi
{
    return _dpi;
}

- (void)setWidth:(NSInteger)width
{
    if (width != _width) {
        _width = width;
        [self willChangeValueForKey:@"dpi"];
        _dpi = width * DPI_BASE / self.exportRect.size.width;
        [self didChangeValueForKey:@"dpi"];
        [self willChangeValueForKey:@"height"];
        _height = floor (self.exportRect.size.height * _dpi / DPI_BASE + 0.5);
        [self didChangeValueForKey:@"height"];
    }
}

- (NSInteger)width
{
    return _width;
}

- (void)setHeight:(NSInteger)height
{
    if (height != _height) {
        _height = height;
        [self willChangeValueForKey:@"dpi"];
        _dpi = height * DPI_BASE / self.exportRect.size.height;
        [self didChangeValueForKey:@"dpi"];
        [self willChangeValueForKey:@"width"];
        _width = floor (self.exportRect.size.width * _dpi / DPI_BASE + 0.5);
        [self didChangeValueForKey:@"width"];
    }
}

- (NSInteger)height
{
    return _height;
}

- (IBAction)cancel:(id)sender
{
    self.userCanceld = TRUE;
}

@end


/// Called for every progress iteration
static unsigned int sp_export_progress_callback (float value, void *data)
{
    ExportController *ec = (__bridge ExportController *)data;

    [ec performSelectorOnMainThread:@selector(didChangedProgress:)
                         withObject:[NSNumber numberWithDouble:value]
                      waitUntilDone:NO];
//    [ec didChangedProgress:[NSNumber numberWithDouble:value]];
    [NSThread sleepForTimeInterval:0.01];
    return !ec.userCanceld;
} // end of sp_export_progress_callback()


