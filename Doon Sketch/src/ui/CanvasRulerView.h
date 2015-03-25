//
//  CanvasRulerView.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/24.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class FixedCanvasView;

extern NSString *kCanvasRulerViewDraggedNotification;

@interface CanvasRulerView : NSView

@property CGFloat scale;
@property NSRulerOrientation orientation;
@property CGFloat originOffset;
@property(assign) NSView *clientView;
@property(assign) NSScrollView *scrollView;

- (id)initWithOrientation:(NSRulerOrientation)orientation;

@end
