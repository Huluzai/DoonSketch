//
//  CanvasScrollView.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/19.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "CocoaCanvasView.h"

@class AbsoluteValueScroller;
@class FixedCanvasView;
@class CanvasRulerView;

@interface CanvasScrollView : NSView

@property FixedCanvasView *documentView;
@property BOOL hasHorizontalScroller;
@property BOOL hasVerticalScroller;
@property BOOL hasHorizontalRuler;
@property BOOL hasVerticalRuler;
@property BOOL rulersVisible;
@property(strong) CanvasRulerView *horizontalRulerView;
@property(strong) CanvasRulerView *verticalRulerView;
@property(strong) AbsoluteValueScroller *verticalScroller;
@property(strong) AbsoluteValueScroller *horizontalScroller;

@end
