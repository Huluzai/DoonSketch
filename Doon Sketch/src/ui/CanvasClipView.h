//
//  CanvasClipView.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-19.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString *kCanvasClipViewWillScrollNotification;
extern NSString *kCanvasClipViewDidScrollNotification;

@interface CanvasClipView : NSClipView

@property id scrollDelegate;

@end


@interface CanvasClipView (DelegateMethods)

- (CGPoint)clipView:(CanvasClipView *)clipView willScrollToPoint:(CGPoint)newOrigin;

@end