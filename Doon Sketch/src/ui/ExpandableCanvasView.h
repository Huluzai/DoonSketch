//
//  ExpandableCanvasView.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-17.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CocoaCanvasView.h"
#import <cairo.h>

struct SPCocoaCanvas;

@interface ExpandableCanvasView : CocoaCanvasView

@property CGRect focusRect;
@property SPDesktop *desktop;
@property BOOL allowsExpand;

- (cairo_surface_t *)cacheSurface;
- (CGPoint)convertPointToInkscape:(CGPoint)point;
- (CGPoint)convertPointFromInkscape:(CGPoint)point;

// Called by canvas controller
- (CGPoint)willScrollToPoint:(CGPoint)newOrigin;

@end
