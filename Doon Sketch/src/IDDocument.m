//
//  Document.m
//  inkDraw
//
//  Created by 张 光建 on 14/11/13.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "IDDocument.h"

#import <desktop.h>
#import <desktop-handles.h>
#import <sigc++/sigc++.h>
#import <ui/view/edit-widget-interface.h>
#import <gtk/gtk.h>
#import <Cocoa/Cocoa.h>
#import <sp-namedview.h>
#import <unit-constants.h>
#import <tools-switch.h>

#import "CocoaDesktopWidget.h"
#import "sp-canvas-cocoa.h"
#import "CocoaCanvasView.h"
#import <document.h>
#import "application/editor.h"
#import <verbs.h>
#import <helper/action.h>
#import <inkscape-private.h>
#import <verbs.h>
#import <helper/action.h>
#import "ItemsPanel.h"
#import "GeomAttributesPage.h"
#import "StrokeStylePanel.h"
#import "PaintPanel.h"
#import "ZGDesktopStyle.h"
#import "ActiveInfoPanel.h"
#import "CocoaCanvasView.h"
#import "ZGDocument.h"
#import "CanvasClipView.h"
#import "ZGDesktop.h"
#import "GeomPropertiesPage.h"
#import "SelectToolbar.h"
#import "NodeToolbox.h"
#import "StarToolbox.h"
#import "RectToolbox.h"
#import "ArcToolbox.h"
#import "SpiralToolbox.h"
#import "PencilToolbox.h"
#import "PenToolbox.h"
#import "CalligraphyToolbox.h"
#import "TextToolbox.h"
#import "PaintbucketToolbox.h"
#import "StylePagesManager.h"
#import "ExportController.h"
#import "MessagePage.h"
#import "CanvasScrollView.h"
#import "AbsoluteValueScroller.h"
#import "FixedCanvasView.h"
#import "CanvasRulerView.h"
#import "PageSettings.h"

#define USE_AUTO_EXPANED_CANVAS (1)
#define MyScrollView ((CanvasScrollView *)self.canvasView)

using namespace Inkscape;

NSScrollView *nullView = nill;

@interface Document ()

@property SPDesktop *desktop;
@property CocoaEditWidget *editWidget;
@property SPDocument *document;

@property ZGDesktopStyle *desktopStyle;

@property NSArray *availableFontSizes;

@property ActiveInfoPanel *coordinateStatusPanel;
@property ActiveInfoPanel *messagePanel;
@property ZGDocument *docWrapper;
@property ZGDesktop *dtWrapper;

@property BOOL liveMagnify;
@property CGFloat savedMagnification;

@property NSTimer *scrollTestTimer;

@property SelectToolbar *selectToolbar;
@property NodeToolbox *nodeToolbox;
@property StarToolbox *starToolbox;
@property RectToolbox *rectToolbox;
@property ArcToolbox *arcToolbox;
@property SpiralToolbox *spiralToolbox;
@property PencilToolbox *pencilToolbox;
@property PenToolbox *penToolbox;
@property CalligraphyToolbox *cagrToolbox;
@property TextToolbox *textToolbox;
@property PaintbucketToolbox *pbToolbox;

@property NSSavePanel *exportPanel;
@property PageSettings *pageSettings;

@property NSUInteger undoStackDepth;
@property NSUInteger undoPos;

@end

struct ParserUserData {
    int current_attr;
    NSMutableAttributedString *result;
    NSMutableAttributedString *current_string;
};

NSDictionary *normal_parse_attrs()
{
    NSFont *fnt = [NSFont fontWithName:@"Copperplate" size:11.f];
    return [NSDictionary dictionaryWithObjectsAndKeys:
            [NSColor blackColor], NSForegroundColorAttributeName,
            fnt, NSFontAttributeName,
            NULL];
}

static void on_parse_start_element (GMarkupParseContext *context,//xml解析器上下文
                                    const gchar *element_name,//元素名，这里是node
                                    const gchar **attribute_names,//属性数组，这里是"att",
                                    const gchar **attribute_values,//属性值数组，这里是"value",
                                    gpointer user_data,//用户自定义数据
                                    GError **error)//保存返回的GError
{
    if (g_strcmp0(element_name, "h")) {
        ParserUserData *udata = (ParserUserData *)user_data;
        udata->current_attr = 1;
        udata->current_string = [[NSMutableAttributedString alloc] init];
    }
}

/* 遇到元素结束符号时调用，这里是 </node> */
static void on_parse_end_element (GMarkupParseContext *context,
                            const gchar *element_name,
                            gpointer user_data,
                            GError **error)
{
    ParserUserData *udata = (ParserUserData *)user_data;
    udata->current_attr = 0;
    if (udata->current_string) {
        NSDictionary *attrs = [NSDictionary dictionaryWithObject:[NSColor redColor]
                                                          forKey:NSForegroundColorAttributeName];
        [udata->current_string addAttributes:attrs range:NSMakeRange(0, udata->current_string.length)];
        [udata->result appendAttributedString:udata->current_string];
        udata->current_string = nill;
    }
}

/* 遇到字符值时调用，这里是 this is text*/
void on_parse_text (GMarkupParseContext *context, const gchar *text, //这里是this is text
              gsize text_len,//长度
              gpointer user_data, GError **error)
{
    ParserUserData *udata = (ParserUserData *)user_data;
    NSMutableAttributedString *str = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithUTF8String:text]
                                                                            attributes:normal_parse_attrs()];
    if (!udata->current_string) {
        [udata->result appendAttributedString:str];
    } else {
        [udata->current_string appendAttributedString:str];
    }
}

@implementation Document


#if 1
- (instancetype)init {
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
        [self buildWithSPDocument:NULL];
#if 1
        self.coordinateStatusPanel = [[ActiveInfoPanel alloc] init];
        self.coordinateStatusPanel.xAlign = NSLeftTextAlignment;
        self.coordinateStatusPanel.yAlign = NSLeftTextAlignment;
        self.messagePanel = [[ActiveInfoPanel alloc] init];
        self.messagePanel.xAlign = NSCenterTextAlignment;
        self.messagePanel.yAlign = NSCenterTextAlignment;
        
        self.availableFontSizes = [ZGDesktopStyle availableFontSizes];
        
        self.ec = [[ExportController alloc] initWithWindowNibName:@"ExportController"];
        self.exportPanel = [[NSSavePanel alloc] init];
        self.exportPanel.title = @"Export To ...";
        self.pageSettings = [[PageSettings alloc] initWithWindowNibName:@"PageSettings"];
        
        self.hasUndoManager = FALSE;
#endif
    }
    return self;
}

- (void)dealloc
{
    [defaultNfc removeObserver:self];
    
    self.dtWrapper = nill;
    self.docWrapper = nill;
    
    self.coordinateStatusPanel = nill;
    self.messagePanel = nill;
    
    self.canvasView = nill;
    
    self.desktopStyle = nill;
    self.availableFontSizes = nill;
    
    self.selectToolbar = nill;
    self.nodeToolbox = nill;
    self.starToolbox = nill;
    self.rectToolbox = nill;
    self.arcToolbox = nill;
    self.spiralToolbox = nill;
    self.pencilToolbox = nill;
    self.penToolbox = nill;
    self.cagrToolbox = nill;
    self.textToolbox = nill;
    self.pbToolbox = nill;
    
    self.ec = nill;
    self.pageSettings = nill;
    self.exportPanel = nill;
    
    if (self.canvas) {
        self.canvas->shutdown_transients();
        self.canvas->unref();
    }
    
    if (self.desktop) {
        inkscape_remove_desktop(self.desktop);
        self.desktop->destroy();
        Inkscape::GC::release(self.desktop);
    }
    
    if (self.document) {
        inkscape_remove_document(self.document);
        sp_document_unref(self.document);
    }
    
    if (self.editWidget) {
        delete self.editWidget;
    }
}

- (void)didTestTimerFired:(NSTimer *)sender
{
    static CGFloat dstx = 0;
    static CGFloat dsty = 0;
    static GTimeVal start;
    static bool first_run = true;
    
    if (first_run) {
        first_run = false;
        g_get_current_time(&start);
    }
    GTimeVal now;
    g_get_current_time(&now);
    glong elapsed = now.tv_sec - start.tv_sec;
    g_debug("test lasts %d seconds", elapsed);
    
    if (self.canvas) {
        
        Geom::Rect r = self.canvas->getViewbox();
        CGFloat distx = dstx - r.min()[0];
        CGFloat disty = dsty - r.min()[1];
        
        if (fabs(distx) < 40) {
            dstx = (random() % 2000) - 1000;
        }
        
        if (fabs(disty) < 40) {
            dsty = (random() % 1500) - 1500;
        }
        
        int stepx = random() % 40 * (dstx > r.min()[0] ? 1 : -1);
        int stepy = random() % 40 * (dsty > r.min()[1] ? 1 : -1);
        
        self.canvas->scroll_to(r.min()[0] + stepx, r.min()[1] + stepy, TRUE);
    }
}

- (IBAction)randomScrollTest:(id)sender
{
    static BOOL state = FALSE;
    
    if (state != NSOnState) {
        srandom((UInt)time(NULL));
        self.scrollTestTimer = [NSTimer scheduledTimerWithTimeInterval:0.005
                                                                target:self
                                                              selector:@selector(didTestTimerFired:)
                                                              userInfo:nill
                                                               repeats:TRUE];
        state = NSOnState;
    } else {
        [self.scrollTestTimer invalidate];
        self.scrollTestTimer = nill;
        state = NSOffState;
    }
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController {
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
    
    /*
     // Load float blocks
     self.basicAttrBlock.name = @"basic_attr";
     self.basicAttrBlock.priority = 1000;
     
     [self.blocksController setInset:0];
     [self.blocksController addBlock:self.basicAttrBlock];
     [self.blocksController updateLayout];
     */
#define ScrollView nullView

#if 0
#define ScrollView ([self.canvasView enclosingScrollView])

    [ScrollView setHasHorizontalRuler:TRUE];
    [ScrollView setHasVerticalRuler:TRUE];
    [ScrollView setRulersVisible:TRUE];
    [[ScrollView horizontalRulerView] setOriginOffset:1000];
    [[ScrollView verticalRulerView] setOriginOffset:1000];

#ifndef USE_AUTO_EXPANED_CANVAS
    NSScrollView *scroll = ScrollView;
    self.canvasView = [[CocoaCanvasView alloc] initWithFrame:CGRectMake(0, 0, 2000, 2000)];
    [scroll setDocumentView:self.canvasView];
    [self.canvasView setBoundsOrigin:CGPointMake(0, -self.canvasView.bounds.size.height)];
#endif
    
    [self.canvasView setFrame:CGRectMake(0, 0, 2000, 2000)];
    [self.canvasView setBoundsOrigin:CGPointMake(0, -self.canvasView.bounds.size.height)];
#endif
    
#if 1
    self.stylesPagesMgr.desktop = self.dtWrapper;
    [self.stylesPagesMgr loadPages];
    
    // Toolbars
    self.selectToolbar = [[SelectToolbar alloc] initWithDesktop:self.desktop];
    self.nodeToolbox = [[NodeToolbox alloc] initWithSPDesktop:self.desktop];
    self.starToolbox = [[StarToolbox alloc] initWithSPDesktop:self.desktop];
    self.rectToolbox = [[RectToolbox alloc] initWithSPDesktop:self.desktop];
    self.arcToolbox = [[ArcToolbox alloc] initWithSPDesktop:self.desktop];
    self.spiralToolbox = [[SpiralToolbox alloc] initWithSPDesktop:self.desktop];
    self.pencilToolbox = [[PencilToolbox alloc] initWithSPDesktop:self.desktop];
    self.penToolbox = [[PenToolbox alloc] initWithSPDesktop:self.desktop];
    self.cagrToolbox = [[CalligraphyToolbox alloc] initWithSPDesktop:self.desktop];
    self.textToolbox = [[TextToolbox alloc] initWithSPDesktop:self.desktop];
    self.pbToolbox = [[PaintbucketToolbox alloc] initWithSPDesktop:self.desktop];
    
    self.ec.delegate = self;
    self.pageSettings.delegate = self;
    self.ec.zDt = self.dtWrapper;
#endif
    
    self.canvasView = self.scrollView.documentView;
    
    /*
    [defaultNfc addObserver:self
                   selector:@selector(scrollViewDidEndLiveMagnify:)
                       name:NSScrollViewDidEndLiveMagnifyNotification
                     object:ScrollView];
    [defaultNfc addObserver:self
                   selector:@selector(scrollViewWillStartLiveMagnify:)
                       name:NSScrollViewWillStartLiveMagnifyNotification
                     object:ScrollView];*/
#ifdef USE_AUTO_EXPANED_CANVAS
    [defaultNfc addObserver:self
                   selector:@selector(willClipViewScroll:)
                       name:kCanvasClipViewWillScrollNotification
                     object:ScrollView.contentView];
    [defaultNfc addObserver:self
                   selector:@selector(didClipViewScroll:)
                       name:kCanvasClipViewDidScrollNotification
                     object:ScrollView.contentView];
#endif
    [defaultNfc addObserver:self
                   selector:@selector(canvasViewWillStartGesture:)
                       name:kCocoaCanvasViewWillStartGesture
                     object:self.canvasView];
    [defaultNfc addObserver:self
                   selector:@selector(canvasViewDidEndGesture:)
                       name:kCocoaCanvasViewDidEndGesture
                     object:self.canvasView];
    [defaultNfc addObserver:self
                   selector:@selector(canvasViewDidMagnify:)
                       name:kCocoaCanvasViewDidMagnify
                     object:self.canvasView];
    [defaultNfc addObserver:self
                   selector:@selector(canvasViewDidScroll:)
                       name:kCocoaCanvasViewDidScrollNotification
                     object:self.canvasView];
    [defaultNfc addObserver:self
                   selector:@selector(desktopEventContextDidChange:)
                       name:kZGDesktopEventContextDidChangeNotification
                     object:self.dtWrapper];
    [defaultNfc addObserver:self
                   selector:@selector(didWindowBecomeMain:)
                       name:NSWindowDidBecomeMainNotification
                     object:self.window];
    
    tools_switch(self.desktop, TOOLS_SELECT);
    
    self.canvasView.canvas = self.canvas;
    
    self.itemsPanel.desktop = self.desktop;
    
    if (self.document) {
        // resize the window to match the document properties
        sp_namedview_window_from_document(self.desktop);
        sp_namedview_update_layers_from_document(self.desktop);
    }
}

- (void)autoAdjustCanvasFrame
{
    CGRect canvasBounds = [self bestCanvasBounds];
    g_debug("will adjust canvas bounds to : %.0f %.0f %.0f %.0f",
            canvasBounds.origin.x, canvasBounds.origin.y, canvasBounds.size.width, canvasBounds.size.height);
    [self.canvasView setFrame:CGRectMake(0, 0, canvasBounds.size.width, canvasBounds.size.height)];
    [self.canvasView setBounds:canvasBounds];
}

+ (BOOL)autosavesInPlace {
    return TRUE;
}

- (void)didDocument:(ZGDocument *)doc changeSize:(CGSize)size
{
    
}

- (void)scrollViewWillStartLiveMagnify:(NSNotification *)nt
{
    self.liveMagnify = TRUE;
}

- (void)scrollViewDidEndLiveMagnify:(NSNotification *)nt
{
    self.liveMagnify = FALSE;

    if (nt.object == ScrollView) {
        g_debug("end live with magnify = %.2f", ScrollView.magnification);
        Geom::Rect const d = self.desktop->get_display_area();
        self.desktop->zoom_relative(d.midpoint()[Geom::X], d.midpoint()[Geom::Y], ScrollView.magnification);
        ScrollView.magnification = 1.0;
#ifndef USE_AUTO_EXPANED_CANVAS
        [self autoAdjustCanvasFrame];
#endif
    }
}

- (void)canvasViewDidScroll:(NSNotification *)nt
{
    [self didTestTimerFired:nill];
}

- (IBAction)resizeView:(id)sender
{
    [self.canvasView setFrameSize:CGSizeMake(2000, 2000)];
    [self.canvasView setBoundsOrigin:CGPointMake(-1000, -1000)];
}

- (void)buildWithSPDocument:(SPDocument *)doc
{
    if (self.canvas) {
        self.canvas->unref();
        self.canvas = NULL;
    }
    
    if (self.desktop) {
        inkscape_remove_desktop(self.desktop);
        self.desktop->destroy();
        Inkscape::GC::release(self.desktop);
        self.desktop = NULL;
    }
    
    if (self.document) {
        sp_document_unref(self.document);
        self.document = NULL;
    }
    
    if (!doc) {
        self.document = sp_document_new(NULL, TRUE, TRUE);
    } else {
        self.document = sp_document_ref(doc);
    }
    
    if (self.document) {
        
        if (!self.editWidget) {
            self.editWidget = new CocoaEditWidget(self);
        }
        
        self.canvas = new SPCocoaCanvas();
        
        self.desktop = new SPDesktop();
        self.desktop->init(sp_document_namedview(self.document, NULL),
                           self.canvas,
                           self.editWidget);
        
        self.desktopStyle = [[ZGDesktopStyle alloc] initWithDesktop:self.desktop];
        self.dtWrapper = [[ZGDesktop alloc] initWithSPDesktop:self.desktop];
    
        inkscape_add_desktop(self.desktop);
        inkscape_activate_desktop(self.desktop);
    
        self.docWrapper = [[ZGDocument alloc] initWithSPDocument:self.document];
        self.docWrapper.delegate = self;
    }
    
    [self updateChangeCount:NSChangeCleared];
}

- (NSWindow *)window
{
    if ([[self windowControllers] count] <= 0) {
        return NULL;
    }
    return [[[self windowControllers] objectAtIndex:0] window];
}

- (void)requestCanvasUpdate
{
}

- (void)requestCanvasUpdateAndWait
{
    
}

- (NSString *)windowNibName {
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"Document";
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError {
    // Insert code here to write your document to data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning nil.
    // You can also choose to override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.
    
    Glib::ustring outStr = sp_repr_save_buf(sp_document_repr_doc(self.document));
    NSData *data = [NSData dataWithBytes:outStr.data() length:outStr.length()];
    return data;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError {
    // Insert code here to read your document from the given data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning NO.
    // You can also choose to override -readFromFileWrapper:ofType:error: or -readFromURL:ofType:error: instead.
    // If you override either of these, you should also override -isEntireFileLoaded to return NO if the contents are lazily loaded.
#if 1
    SPDocument *doc = sp_document_new_from_mem((gchar*)data.bytes, (guint)data.length, true);
    if (doc) {
        [self buildWithSPDocument:doc];
        sp_document_unref(doc);
    }
    return self.document ? TRUE : FALSE;
#else
    return FALSE;
#endif
}

- (void)performVerbWithCode:(int)verb
{
    Verb *v = Inkscape::Verb::get(verb);
    sp_action_perform(v->get_action(self.desktop), NULL);
}

- (void)performVerbWithId:(NSString *)verb_id
{
    Verb *verb = Inkscape::Verb::getbyid([verb_id UTF8String]);
    sp_action_perform(verb->get_action(self.desktop), NULL);
}

- (IBAction)performVerb:(id)sender
{
    int verb = SP_VERB_INVALID;
    
    if ([sender isKindOfClass:[NSMatrix class]]) {
        verb = (int)[[sender selectedCell] tag];
    } else {
        verb = (int)[sender tag];
    }
    
    Inkscape::Verb *v = Inkscape::Verb::get(verb);
    if (v && self.desktop) {
        SPAction *act = v->get_action(self.desktop);
        sp_action_perform(act, NULL);
    }
}

- (void)desktopEventContextDidChange:(NSNotification *)aNotification
{
    const NSArray *optViews = [NSArray arrayWithObjects:
                               [NSNull null], //0
                               self.selectToolbar,
                               self.nodeToolbox,
                               [NSNull null],
                               [NSNull null],
                               self.rectToolbox, //5
                               [NSNull null],
                               self.arcToolbox,
                               self.starToolbox,
                               self.spiralToolbox,
                               self.pencilToolbox,   //10
                               self.penToolbox,
                               self.cagrToolbox,
                               self.textToolbox,
                               [NSNull null],
                               [NSNull null], //15
                               [NSNull null],
                               [NSNull null],
                               self.pbToolbox,
                               [NSNull null],
                               [NSNull null], //20
                               NULL];
    
    int current_tool = TOOLS_INVALID;
    
    for (int i = TOOLS_INVALID + 1; i <= TOOLS_LPETOOL; i++) {
        if (tools_isactive(self.desktop, i)) {
            current_tool = i;
            break;
        }
    }
    
    if (current_tool != TOOLS_INVALID) {
        NSViewController *ctl = [optViews objectAtIndex:current_tool];
        if (![ctl isEqual:[NSNull null]]) {
            NSView *view = [ctl view];
            [view setFrame:self.toolbarPlace.bounds];
            [view setAutoresizingMask:NSViewMinXMargin | NSViewMaxXMargin | NSViewWidthSizable];
            [self.toolbarPlace setSubviews:[NSArray arrayWithObject:view]];
        }
        [self.matrixTools selectCellWithTag:current_tool];
    }
}

#pragma mark - delegate methods for cocoa edit widget

- (void)updateScrollbars:(double)scale
{
    static bool _update_s_f = false;
    
    if (_update_s_f || ![self.scrollView isKindOfClass:[CanvasScrollView class]]) {
        return;
    }

    _update_s_f = true;
    
    /* The desktop region we always show unconditionally */
    SPDocument *doc = self.document;
    Geom::Rect darea ( Geom::Point(-sp_document_width(doc), -sp_document_height(doc)),
                      Geom::Point(2 * sp_document_width(doc), 2 * sp_document_height(doc))  );
    SPObject* root = doc->root;
    SPItem* item = SP_ITEM(root);
    Geom::OptRect deskarea = Geom::unify(darea, sp_item_bbox_desktop(item));
    
    /* Canvas region we always show unconditionally */
    Geom::Rect carea( Geom::Point(deskarea->min()[Geom::X] * scale - 64, deskarea->max()[Geom::Y] * -scale - 64),
                     Geom::Point(deskarea->max()[Geom::X] * scale + 64, deskarea->min()[Geom::Y] * -scale + 64)  );
    
    Geom::Rect const viewbox = self.canvas->getViewbox();
    
    /* Viewbox is always included into scrollable region */
    carea = Geom::unify(carea, viewbox);
    
    CanvasScrollView *scrollView = self.scrollView;
    
    scrollView.horizontalScroller.absValue = viewbox.min()[Geom::X];
    scrollView.horizontalScroller.absLower = carea.min()[Geom::X];
    scrollView.horizontalScroller.absUpper = carea.max()[Geom::X];
    scrollView.horizontalScroller.knobProportion = viewbox.dimensions()[Geom::X] / (carea.max()[Geom::X] - carea.min()[Geom::X]);
    
    scrollView.verticalScroller.absValue = viewbox.min()[Geom::Y];
    scrollView.verticalScroller.absLower = carea.min()[Geom::Y];
    scrollView.verticalScroller.absUpper = carea.max()[Geom::Y];
    scrollView.verticalScroller.knobProportion = viewbox.dimensions()[Geom::Y] / (carea.max()[Geom::Y] - carea.min()[Geom::Y]);
    
    _update_s_f = false;
}

- (void)updateCoordinateStatus:(CGPoint)point
{/*
    NSString *msg = [NSString stringWithFormat:@"%.0f,%.0f", point.x, point.y];
    CGPoint pos = [self.window mouseLocationOutsideOfEventStream];
    CGRect r = [self.canvasView convertRect:self.canvasView.bounds toView:nill];
    
    if (CGRectContainsPoint(r, pos)) {
        pos = [self.window.contentView convertPoint:pos fromView:nill];
        [self.coordinateStatusPanel showContent:msg
                                        atPoint:CGPointMake(pos.x + 10, pos.y - 10)
                                       inWindow:self.window
                                        forTime:3];
    } else {
        [self.coordinateStatusPanel dismiss];
    }*/
    self.stylesPagesMgr.messagePage.pointInfo = [NSString stringWithFormat:@"x : %d \ty : %d", (int)point.x, (int)point.y];
}

- (void)showMessage:(NSString *)message
{
    /*    [self.messagePanel showContent:message
     forView:self.canvasView
     inWindow:self.window
     forTime:5];*/
    const GMarkupParser parser = {
        on_parse_start_element,
        on_parse_end_element,
        on_parse_text,
        NULL,
        NULL
    };
    
    NSString *tmp = [NSString stringWithFormat:@"<h>%@</h>", message];
    
    ParserUserData udata;
    udata.current_string = nill;
    udata.current_attr = 0;
    udata.result = [[NSMutableAttributedString alloc] initWithString:@""];
    
    GMarkupParseContext *context;
    context = g_markup_parse_context_new(&parser, (GMarkupParseFlags)0, (gpointer)&udata, (GDestroyNotify)0);
    g_markup_parse_context_parse(context, (const gchar *)tmp.UTF8String, (gssize)tmp.length, NULL);
    g_markup_parse_context_unref(context);
    
    self.stylesPagesMgr.messagePage.message = udata.result;
}

- (void)changeUnitsWithZoom:(double)zoomVal
{
    NSArray *upCycles = [NSArray arrayWithObject:[NSNumber numberWithFloat:2]];
    NSArray *downCycles = [NSArray arrayWithObject:[NSNumber numberWithFloat:0.5]];
    
    [NSRulerView registerUnitWithName:@"dynamic-points"
                         abbreviation:@"dp"
         unitToPointsConversionFactor:zoomVal
                          stepUpCycle:upCycles
                        stepDownCycle:downCycles];
    [ScrollView.verticalRulerView setMeasurementUnits:@"dynamic-points"];
    [ScrollView.horizontalRulerView setMeasurementUnits:@"dynamic-points"];
}

- (IBAction)toggleRulers:(id)sender
{
    [self.scrollView setRulersVisible:!self.scrollView.rulersVisible];
}

- (void)updateRulers
{
    //Geom::Point gridorigin = _namedview->gridorigin;
    /// \todo Why was the origin corrected for the grid origin? (johan)
#if 0
    Geom::Rect const viewbox = self.canvas->getViewbox();
    double lo, up, pos, max;
    double const scale = _desktop->current_zoom();
    double s = viewbox.min()[Geom::X] / scale; //- gridorigin[Geom::X];
    double e = viewbox.max()[Geom::X] / scale; //- gridorigin[Geom::X];
    _top_ruler.get_range(lo, up, pos, max);
    _top_ruler.set_range(s, e, pos, e);
    s = viewbox.min()[Geom::Y] / -scale; //- gridorigin[Geom::Y];
    e = viewbox.max()[Geom::Y] / -scale; //- gridorigin[Geom::Y];
    _left_ruler.set_range(s, e, 0 /*gridorigin[Geom::Y]*/, e);
    /// \todo is that correct?
#elif 1
    Geom::Rect const viewbox = self.canvas->getViewbox();
    double lo, up, pos, max;
    double const scale = _desktop->current_zoom();
    double s = viewbox.min()[Geom::X] / scale; //- gridorigin[Geom::X];
    double e = viewbox.max()[Geom::X] / scale; //- gridorigin[Geom::X];
    
//    double const scale = self.desktop->current_zoom();
    FixedCanvasView *view = (FixedCanvasView *)self.canvasView;
    
    g_debug("update rulers with zoom = %.2f", scale);
    //[self changeUnitsWithZoom:scale];
    [[self.scrollView verticalRulerView] setScale:scale];
    [[self.scrollView horizontalRulerView] setScale:scale];
    [[self.scrollView verticalRulerView] setOriginOffset:view.origin.y];
    [[self.scrollView horizontalRulerView] setOriginOffset:view.origin.x];
#else
    double const scale = self.desktop->current_zoom();
    
    g_debug("update rulers with zoom = %.2f", scale);
    [self changeUnitsWithZoom:scale];
    [[ScrollView verticalRulerView] setOriginOffset:-self.canvasView.bounds.origin.y];
    [[ScrollView horizontalRulerView] setOriginOffset:-self.canvasView.bounds.origin.x];
#endif
}

#pragma mark - Actions

- (void)switchToolWithVerbId:(int)verb
{
    [self performVerbWithCode:verb];
    
    NSView *toolboxView = nill;
    
    switch (verb) {
        case SP_VERB_CONTEXT_NODE:
            toolboxView = self.nodeToolbox.view;
            break;
        case SP_VERB_CONTEXT_SELECT:
            toolboxView = self.selectToolbar.view;
            break;
        case SP_VERB_CONTEXT_STAR:
            toolboxView = self.starToolbox.view;
            break;
        case SP_VERB_CONTEXT_RECT:
            toolboxView = self.rectToolbox.view;
            break;
        case SP_VERB_CONTEXT_ARC:
            toolboxView = self.arcToolbox.view;
            break;
        case SP_VERB_CONTEXT_SPIRAL:
            toolboxView = self.spiralToolbox.view;
            break;
        case SP_VERB_CONTEXT_PENCIL:
            toolboxView = self.pencilToolbox.view;
            break;
        case SP_VERB_CONTEXT_PEN:
            toolboxView = self.penToolbox.view;
            break;
        case SP_VERB_CONTEXT_CALLIGRAPHIC:
            toolboxView = self.cagrToolbox.view;
            break;
        case SP_VERB_CONTEXT_TEXT:
            toolboxView = self.textToolbox.view;
            break;
        case SP_VERB_CONTEXT_PAINTBUCKET:
            toolboxView = self.pbToolbox.view;
            break;
        default:;
    }
    
    if (toolboxView) {
        [toolboxView setFrame:self.toolbarPlace.bounds];
        [toolboxView setAutoresizingMask:NSViewMinXMargin | NSViewMaxXMargin | NSViewWidthSizable];
        [self.toolbarPlace setSubviews:[NSArray arrayWithObject:toolboxView]];
    }
}

- (IBAction)didSwitchTools:(id)sender
{
    int tool = TOOLS_INVALID;
    
    if ([sender isKindOfClass:[NSMatrix class]]) {
        tool = (int)[[sender selectedCell] tag];
    } else {
        tool = (int)[sender tag];
    }
    
    tools_switch(self.desktop, tool);
}

- (IBAction)zoomToDrawing:(id)sender
{
    [self performVerbWithCode:SP_VERB_ZOOM_DRAWING];
}

- (IBAction)zoomToPage:(id)sender
{
    [self performVerbWithCode:SP_VERB_ZOOM_PAGE];
}

- (IBAction)zoomToPageWidth:(id)sender
{
    [self performVerbWithCode:SP_VERB_ZOOM_PAGE_WIDTH];
}

- (IBAction)zoomIn:(id)sender
{
    [self performVerbWithCode:SP_VERB_ZOOM_IN];
}

- (IBAction)zoomOut:(id)sender
{
    [self performVerbWithCode:SP_VERB_ZOOM_OUT];
}

- (IBAction)zoomOrigin:(id)sender
{
    [self performVerbWithCode:SP_VERB_ZOOM_1_1];
}

- (IBAction)zoomDouble:(id)sender
{
    [self performVerbWithCode:SP_VERB_ZOOM_2_1];
}

- (IBAction)zoomHalf:(id)sender
{
    [self performVerbWithCode:SP_VERB_ZOOM_1_2];
}

- (IBAction)showGuides:(id)sender
{
    [self performVerbWithCode:SP_VERB_TOGGLE_GUIDES];
}

- (IBAction)showGrid:(id)sender
{
    [self performVerbWithCode:SP_VERB_TOGGLE_GRID];
}

- (IBAction)group:(id)sender
{
    [self performVerbWithCode:SP_VERB_SELECTION_GROUP];
}

- (IBAction)ungroup:(id)sender
{
    [self performVerbWithCode:SP_VERB_SELECTION_UNGROUP];
}

- (IBAction)unionOp:(id)sender
{
    [self performVerbWithCode:SP_VERB_SELECTION_UNION];
}

- (IBAction)diffOp:(id)sender
{
    [self performVerbWithCode:SP_VERB_SELECTION_DIFF];
}

- (IBAction)intersectOp:(id)sender
{
    [self performVerbWithCode:SP_VERB_SELECTION_INTERSECT];
}

- (IBAction)substractOp:(id)sender
{
    [self performVerbWithCode:SP_VERB_SELECTION_SYMDIFF];
}

- (IBAction)rotate90:(id)sender
{
    [self performVerbWithCode:SP_VERB_OBJECT_ROTATE_90_CCW];
}

- (IBAction)rotate90c:(id)sender
{
    [self performVerbWithCode:SP_VERB_OBJECT_ROTATE_90_CW];
}

- (IBAction)flipHorizontal:(id)sender
{
    [self performVerbWithCode:SP_VERB_OBJECT_FLIP_HORIZONTAL];
}

- (IBAction)flipVertical:(id)sender
{
    [self performVerbWithCode:SP_VERB_OBJECT_FLIP_VERTICAL];
}

- (IBAction)reverse:(id)sender
{
    [self performVerbWithCode:SP_VERB_SELECTION_REVERSE];
}

- (IBAction)simplify:(id)sender
{
    [self performVerbWithCode:SP_VERB_SELECTION_SIMPLIFY];
}

- (IBAction)convertToPath:(id)sender
{
    [self performVerbWithCode:SP_VERB_OBJECT_TO_CURVE];
}

- (IBAction)putOnPath:(id)sender
{
    [self performVerbWithCode:SP_VERB_SELECTION_TEXTTOPATH];
}

- (IBAction)removeFromPath:(id)sender
{
    [self performVerbWithCode:SP_VERB_SELECTION_TEXTFROMPATH];
}

- (IBAction)flowInFrame:(id)sender
{
    [self performVerbWithCode:SP_VERB_OBJECT_FLOW_TEXT];
}

- (IBAction)unflow:(id)sender
{
    [self performVerbWithCode:SP_VERB_OBJECT_UNFLOW_TEXT];
}

- (IBAction)bringForward:(id)sender
{
    [self performVerbWithCode:SP_VERB_LAYER_RAISE];
}

- (IBAction)bringBackward:(id)sender
{
    [self performVerbWithCode:SP_VERB_LAYER_LOWER];
}

- (IBAction)bringToFront:(id)sender
{
    [self performVerbWithCode:SP_VERB_LAYER_TO_TOP];
}

- (IBAction)bringToBack:(id)sender
{
    [self performVerbWithCode:SP_VERB_LAYER_TO_BOTTOM];
}

- (NSUInteger)nearestFontSize:(CGFloat)size
{
    CGFloat min = CGFLOAT_MAX;
    NSUInteger idx = 0;
    
    for (NSNumber *n in self.availableFontSizes) {
        CGFloat dif = n.floatValue - size;
        if (fabs(dif) < min) {
            min = fabs(dif);
            idx = [self.availableFontSizes indexOfObject:n];
        }
    }
    
    return idx;
}

- (IBAction)fontSizeBigger:(id)sender
{
    NSUInteger idx = [self nearestFontSize:self.desktopStyle.fontSize];
    idx = (idx + 1) % self.availableFontSizes.count;
    self.desktopStyle.fontSize = [[self.availableFontSizes objectAtIndex:idx] floatValue];
}

- (IBAction)fontSizeSmaller:(id)sender
{
    NSUInteger idx = [self nearestFontSize:self.desktopStyle.fontSize];
    idx = (idx - 1 + self.availableFontSizes.count) % self.availableFontSizes.count;
    self.desktopStyle.fontSize = [[self.availableFontSizes objectAtIndex:idx] floatValue];
}

- (IBAction)changeTextAlignMode:(id)sender
{
    NSTextAlignment mode = (NSTextAlignment)[sender tag];
    self.desktopStyle.textAlign = mode;
}

- (IBAction)toggleFontBold:(id)sender
{
    self.desktopStyle.fontBold = !self.desktopStyle.fontBold;
}

- (IBAction)toggleFontItalic:(id)sender
{
    self.desktopStyle.fontItalic = !self.desktopStyle.fontItalic;
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if (menuItem.action == @selector(fontSizeBigger:)) {
        return [self nearestFontSize:self.desktopStyle.fontSize] < self.availableFontSizes.count - 1;
    } else if (menuItem.action == @selector(fontSizeSmaller:)) {
        return [self nearestFontSize:self.desktopStyle.fontSize] > 0;
    } else if (menuItem.action == @selector(changeTextAlignMode:)) {
        NSTextAlignment mode = (NSTextAlignment)[menuItem tag];
        [menuItem setState:self.desktopStyle.textAlign == mode ? NSOnState : NSOffState];
    } else if (menuItem.action == @selector(toggleFontBold:)) {
        [menuItem setState:self.desktopStyle.fontBold ? NSOnState : NSOffState];
    } else if (menuItem.action == @selector(toggleFontItalic:)) {
        [menuItem setState:self.desktopStyle.fontItalic ? NSOnState : NSOffState];
    } else if (menuItem.action == @selector(docUndo:)) {
        return self.undoPos > 0;
    } else if (menuItem.action == @selector(docRedo:)) {
        return self.undoPos < self.undoStackDepth;
    } else if (menuItem.action == @selector(toggleRulers:)) {
        [menuItem setState:(self.scrollView.rulersVisible ? NSOnState : NSOffState)];
    }
    return TRUE;
}

- (IBAction)drawOutOfView:(id)sender
{
    NSView *view = [sender superview];
    if ([view lockFocusIfCanDraw]) {
        [[NSColor redColor] setFill];
        NSRectFill(CGRectMake(0, 0, 100, 100));
        [view unlockFocus];
    } else {
        g_debug("lock focus failed !!!");
    }
}

- (IBAction)drawOutOfViewInScroll:(id)sender
{
    g_assert(self.testScroll.documentView);
    if ([self.testScroll.documentView lockFocusIfCanDraw]) {
        [[NSColor redColor] setFill];
        NSRectFill(CGRectMake(0, 0, 100, 100));
        [self.testScroll.documentView unlockFocus];
        [self.window flushWindow];
    } else {
        g_debug("lock focus failed !!!");
    }
}


#pragma mark - scroll and expand view

- (void)adjustView:(NSView *)view toFrame:(CGRect)newFrame
{
    if (CGRectEqualToRect(newFrame, view.frame)) {
        return;
    }
    
    double dx = newFrame.origin.x - view.frame.origin.x;
    double dy = newFrame.origin.y - view.frame.origin.y;
    
    g_debug("Adjust view to frame : %.0f %.0f %.0f %.0f from: %.0f %.0f %.0f %.0f",
            newFrame.origin.x, newFrame.origin.y, newFrame.size.width, newFrame.size.height,
            view.frame.origin.x, view.frame.origin.y, view.frame.size.width, view.frame.size.height);
    
    [view setFrame:newFrame];
    
    CGRect r = view.bounds;
    r.origin.x += dx;
    r.origin.y += dy;
//    [view setBoundsOrigin:r.origin];
    [view translateOriginToPoint:CGPointMake(-dx, -dy)];
    [[[view enclosingScrollView] verticalRulerView] setOriginOffset:-r.origin.y];
    [[[view enclosingScrollView] horizontalRulerView] setOriginOffset:-r.origin.x];
    [ScrollView reflectScrolledClipView:ScrollView.contentView];
}

- (CGRect)bestCanvasBounds
{
    Geom::OptRect bbox = sp_item_bbox_desktop (SP_ITEM (SP_DOCUMENT_ROOT (self.document)), SPItem::APPROXIMATE_BBOX);
    Geom::Point drawingMin = self.desktop->doc2dt(Geom::Point(0.0, 0.0));
    Geom::Point drawingMax = self.desktop->doc2dt(Geom::Point(sp_document_width(self.document), sp_document_height(self.document)));
    Geom::OptRect drawing = Geom::Rect(Geom::Point(drawingMin[0], drawingMax[1]), Geom::Point(drawingMax[0], drawingMin[1]));
    Geom::Point center = drawing->midpoint();
    
    if (bbox) {
        drawing->unionWith(bbox);
    }

    Geom::Point wmid = self.desktop->d2w(center);
    Geom::Point wmin = self.desktop->d2w(drawing->min());
    Geom::Point wmax = self.desktop->d2w(drawing->max());

    double rx = MAX(fabs(wmax[0] - wmid[0]), fabs(wmid[0] - wmin[0]));
    double ry = MAX(fabs(wmin[1] - wmid[1]), fabs(wmid[1] - wmax[1]));
    rx = MAX(rx, [[NSScreen mainScreen] frame].size.width);
    ry = MAX(ry, [[NSScreen mainScreen] frame].size.height);
    
    return CGRectMake(wmid[0] - rx, wmid[1] - ry, 2 * rx, 2 * ry);
}

- (void)willClipViewScroll:(NSNotification *)aNotification
{
    if ([aNotification.object isEqual:ScrollView.contentView] && !self.liveMagnify) {
        NSValue *val = [[aNotification userInfo] objectForKey:@"newOrigin"];
        if (val) {
            NSClipView *clip = ScrollView.contentView;
            CGPoint newOrigin = [val pointValue];
            
            CGRect v = CGRectMake(newOrigin.x, newOrigin.y, clip.bounds.size.width, clip.bounds.size.height);

            CGRect r = [self bestCanvasBounds];
            r = [self.canvasView convertRect:r toView:clip];
            r = CGRectInset(r, 10, 10);
            
            CGRect u = CGRectUnion(v, r);
            u = CGRectUnion(clip.bounds, u);
            [self adjustView:clip.documentView toFrame:CGRectInset(u, -10, -10)];
        }
    }
}

- (void)didClipViewScroll:(NSNotification *)aNotification
{
    if ([aNotification.object isEqual:ScrollView.contentView]) {
        
    }
}

- (void)canvasViewWillStartGesture:(NSNotification *)aNotification
{

}

- (void)canvasViewDidEndGesture:(NSNotification *)aNotification
{
    self.liveMagnify = FALSE;
//    [self.canvasView setNeedsDisplay:TRUE];
//    ScrollView.magnification = 1.f;
}

- (void)canvasViewDidMagnify:(NSNotification *)aNotification
{
    if (!self.liveMagnify) {
        self.liveMagnify = TRUE;
        self.savedMagnification = self.desktop->current_zoom();
        g_debug("will begin magnify, saved magnification = %.2f", self.savedMagnification);
    }
    
    NSEvent *event = [aNotification.userInfo objectForKey:@"event"];
    g_assert(event);
    
    if (event.type == NSEventTypeMagnify && self.liveMagnify) {
        double m = 1.f + event.magnification;
        g_debug("live magnification %.2f , zoom to %.2f", event.magnification, m);
        Geom::Rect const d = self.desktop->get_display_area();
        self.desktop->zoom_relative( d.midpoint()[Geom::X], d.midpoint()[Geom::Y], m);
    }
}

- (IBAction)exportBitmap:(id)sender
{
    [self.ec prepareOptionView];
    self.exportPanel.accessoryView = self.ec.viewOption;

    NSString *docTitle = [[self.fileURL lastPathComponent] stringByDeletingPathExtension];
    
    if (docTitle) {
        docTitle = [docTitle stringByAppendingPathExtension:@"png"];
    } else {
        docTitle = @"Untitled.png";
    }
    
    [self.exportPanel setNameFieldStringValue:docTitle];
    
    [self.exportPanel beginSheetModalForWindow:self.window completionHandler:^(NSModalResponse r){
        if (r == NSModalResponseOK) {
      //      [self.ec showWindow:nill];
            [self.window beginSheet:self.ec.window completionHandler:NULL];
            [self.ec exportDocument:self.document toFile:self.exportPanel.URL];
        }
    }];
    
    /*
    if ([self.exportPanel runModal] == NSModalResponseOK) {
        //    [self.window beginSheet:self.exportPanel completionHandler:^ (NSModalResponse r) {
        //        if (r == NSModalResponseOK) {
        [self.exportPanel orderOut:nill];
        [self.window beginSheet:self.ec.window completionHandler:^(NSModalResponse r) {
        }];
        [self.ec exportDocument:self.document toFile:self.exportPanel.URL];
        g_message("export complete");
        [self.window endSheet:self.ec.window returnCode:NSModalResponseOK];
    }*/
    //    }];
}

- (void)exportControllerDidComplete:(ExportController *)ec
{
    g_message("export complete");
    [self.window endSheet:self.ec.window returnCode:NSModalResponseOK];
    [self.ec close];
}

- (IBAction)openPageSettingsPanel:(id)sender
{
    self.pageSettings.width = sp_document_width(self.document);
    self.pageSettings.height = sp_document_height(self.document);
    [self.window beginSheet:self.pageSettings.window completionHandler:^(NSModalResponse r) {}];
}

- (void)didPageSettings:(PageSettings *)settings finishWithCode:(NSInteger)code
{
    [self.window endSheet:self.pageSettings.window];
    
    if (code == NSModalResponseOK) {
        const SPUnit *u = sp_unit_get_by_abbreviation("px");
        sp_document_set_width(self.document, self.pageSettings.width, u);
        sp_document_set_height(self.document, self.pageSettings.height, u);
    }
}

#pragma mark - Undo

- (void)document:(ZGDocument *)doc didUndo:(Event *)e
{
    [self updateChangeCount:NSChangeUndone];
    self.undoPos--;
}

- (void)document:(ZGDocument *)doc didRedo:(Event *)e
{
    [self updateChangeCount:NSChangeRedone];
    self.undoPos++;
}

- (void)document:(ZGDocument *)doc didUndoCommitEvent:(Event *)e
{
    [self updateChangeCount:NSChangeDone];
    self.undoStackDepth++;
    self.undoPos++;
}

- (void)documentDidClearUndoEvent:(ZGDocument *)doc
{
    [self updateChangeCount:NSChangeCleared];
    self.undoStackDepth = 0;
    self.undoPos = 0;
}

- (void)documentDidClearRedoEvent:(ZGDocument *)doc
{
    [self updateChangeCount:NSChangeCleared];
    self.undoStackDepth = 0;
    self.undoPos = 0;
}

- (IBAction)docUndo:(id)sender
{
    sp_document_undo(self.document);
}

- (IBAction)docRedo:(id)sender
{
    sp_document_redo(self.document);
}

- (void)didWindowBecomeMain:(NSNotification *)aNotification
{
    inkscape_activate_desktop(self.desktop);
}

static NSModalSession session = 0;

//test
- (IBAction)showTestPanel:(id)sender
{
    session = [NSApp beginModalSessionForWindow:self.testPanel];
}

- (IBAction)testLockFocus:(id)sender
{
    NSGraphicsContext *ctx = NULL;
    
    //    if ([[self.window contentView] lockFocusIfCanDraw]) {
    ctx = [NSGraphicsContext currentContext];
    NSLog(@"current graphics context is %@", ctx);
    //        [[self.window contentView] unlockFocus];
    //    }
    [NSGraphicsContext saveGraphicsState];
    [[NSColor yellowColor] setFill];
    NSRectFill(CGRectMake(25, 25, 50, 50));
    [NSGraphicsContext restoreGraphicsState];
    
    ctx = [NSGraphicsContext graphicsContextWithWindow:self.window];
    NSLog(@"window graphics context is %@", ctx);
    [NSGraphicsContext setCurrentContext:ctx];
    
    [NSGraphicsContext saveGraphicsState];
    [[NSColor blueColor] setFill];
    NSRectFill(CGRectMake(50, 50, 50, 50));
    [NSGraphicsContext restoreGraphicsState];
    
    NSView *v = self.nudeView;//[[NSImageView alloc] initWithFrame:CGRectMake(0, 0, 200, 200)];
    //    [[self.wrappedView enclosingScrollView] setDocumentView:v];
    NSView *drawToView = v;//[[self.wrappedView enclosingScrollView] contentView];
    if ([drawToView lockFocusIfCanDraw]) {
        ctx = [NSGraphicsContext currentContext];
        NSLog(@"current graphics context is %@ after lockFocus on view [%@] ", ctx, drawToView);
        [NSGraphicsContext saveGraphicsState];
        [[NSColor greenColor] setFill];
        NSRectFill(CGRectMake(10, 10, 50, 50));
        [NSGraphicsContext restoreGraphicsState];
        [drawToView unlockFocus];
    } else {
        NSLog(@"can't lock focus");
    }
    
    [self.window flushWindow];
}

#endif
@end
