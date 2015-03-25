//
//  CanvasController.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-17.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <cairo/cairo.h>

struct SPCocoaCanvas;
struct SPDesktop;

@interface CanvasController : NSObject

@property NSScrollView *scrollView;
@property SPDesktop *desktop;

- (SPCocoaCanvas *)canvas;

- (void)scrollToPoint:(NSPoint)point;

- (cairo_surface_t *)cacheSurface;

- (void)markDirtyRect:(NSRect)dirtyRect;

- (void)requestUpdateWhenIdle;

@end
