//
//  Document.h
//  inkDraw
//
//  Created by 张 光建 on 14/11/13.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CocoaDesktopWidget.h"

@class CocoaCanvasView;
struct SPCocoaCanvas;
@class StylePagesManager;
@class ExportController;
@class CanvasScrollView;
@class PageSettings;
@class ItemsPanel;

@interface Document : NSDocument <CocoaEditWidgetDelegate>

@property IBOutlet CocoaCanvasView *canvasView;
@property IBOutlet CanvasScrollView *scrollView;
@property IBOutlet NSView *toolbarPlace;
//@property IBOutlet BlockListViewController *blocksController;
//@property IBOutlet FloatBlock *basicAttrBlock;
@property IBOutlet NSTabView *attrsTabView;
@property IBOutlet NSView *testView;
@property IBOutlet NSScrollView *testScroll;
@property IBOutlet NSPanel *testPanel;
@property IBOutlet NSMatrix *matrixTools;
@property IBOutlet NSView *viewStyles;
@property IBOutlet StylePagesManager *stylesPagesMgr;
@property IBOutlet ExportController *ec;
@property IBOutlet ItemsPanel *itemsPanel;

@property SPCocoaCanvas *canvas;

- (IBAction)docUndo:(id)sender;
- (IBAction)docRedo:(id)sender;

- (IBAction)didSwitchTools:(id)sender;
- (IBAction)performVerb:(id)sender;
- (IBAction)exportBitmap:(id)sender;

- (IBAction)zoomToDrawing:(id)sender;
- (IBAction)zoomToPage:(id)sender;
- (IBAction)zoomToPageWidth:(id)sender;
- (IBAction)zoomIn:(id)sender;
- (IBAction)zoomOut:(id)sender;
- (IBAction)zoomOrigin:(id)sender;
- (IBAction)zoomDouble:(id)sender;
- (IBAction)zoomHalf:(id)sender;

- (IBAction)showGuides:(id)sender;
- (IBAction)showGrid:(id)sender;

- (IBAction)group:(id)sender;
- (IBAction)ungroup:(id)sender;

- (IBAction)unionOp:(id)sender;
- (IBAction)diffOp:(id)sender;
- (IBAction)intersectOp:(id)sender;
- (IBAction)substractOp:(id)sender;

- (IBAction)rotate90:(id)sender;
- (IBAction)rotate90c:(id)sender;
- (IBAction)flipHorizontal:(id)sender;
- (IBAction)flipVertical:(id)sender;

- (IBAction)reverse:(id)sender;
- (IBAction)simplify:(id)sender;

- (IBAction)convertToPath:(id)sender;

- (IBAction)putOnPath:(id)sender;
- (IBAction)removeFromPath:(id)sender;

- (IBAction)flowInFrame:(id)sender;
- (IBAction)unflow:(id)sender;

- (IBAction)bringForward:(id)sender;
- (IBAction)bringBackward:(id)sender;
- (IBAction)bringToFront:(id)sender;
- (IBAction)bringToBack:(id)sender;

- (IBAction)fontSizeBigger:(id)sender;
- (IBAction)fontSizeSmaller:(id)sender;
- (IBAction)changeTextAlignMode:(id)sender;
- (IBAction)toggleFontBold:(id)sender;
- (IBAction)toggleFontItalic:(id)sender;

- (IBAction)resizeView:(id)sender;
- (IBAction)randomScrollTest:(id)sender;
- (IBAction)drawOutOfView:(id)sender;
- (IBAction)drawOutOfViewInScroll:(id)sender;

- (IBAction)toggleRulers:(id)sender;
- (IBAction)openPageSettingsPanel:(id)sender;

@property IBOutlet NSView *nudeView;

- (IBAction)testLockFocus:(id)sender;
- (IBAction)testScroll:(id)sender;

//test
- (IBAction)showTestPanel:(id)sender;

@end

