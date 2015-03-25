//
//  CocoaCanvas.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#include <cocoa/Cocoa.h>
#include "sp-canvas-cocoa.h"

extern NSString *kCocoaCanvasViewWillStartGesture;
extern NSString *kCocoaCanvasViewDidEndGesture;
extern NSString *kCocoaCanvasViewDidMagnify;
extern NSString *kCocoaCanvasViewDidScrollNotification;

struct SPCocoaCanvas;
struct SPDesktop;

@interface CocoaCanvasView : NSView <CanvasControl>

@property SPDesktop *desktop;
@property SPCocoaCanvas *canvas;
@property NSTrackingArea *trackingArea;
@property BOOL needsUpdate;

- (CGPoint)mapEventPoint:(CGPoint)point;

- (void)didChangeBounds:(NSNotification *)nfc;

- (CGPoint)convertPointToWorld:(CGPoint)point;
- (CGPoint)convertPointFromWorld:(CGPoint)point;
- (CGPoint)convertPointToCanvas:(CGPoint)point;
- (CGPoint)convertPointFromCanvas:(CGPoint)point;

- (void)updateTrackingArea;

/*
- (IBAction)undo:(id)sender;
- (IBAction)redo:(id)sender;
*/
@end

