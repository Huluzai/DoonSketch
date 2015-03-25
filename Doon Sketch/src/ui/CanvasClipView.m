//
//  CanvasClipView.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-19.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "CanvasClipView.h"
#import <inkscape.h>
#import <desktop.h>
#import <2geom/geom.h>

#import "CocoaCanvasView.h"
#import "sp-canvas-cocoa.h"

#define theCanvas   (((CocoaCanvasView *)self.documentView).canvas)
#define theDesktop  (SP_ACTIVE_DESKTOP)

NSString *kCanvasClipViewWillScrollNotification = @"CanvasClipViewWillScrollNotification";
NSString *kCanvasClipViewDidScrollNotification = @"CanvasClipViewDidScrollNotification";

@implementation CanvasClipView

- (void)scrollToPoint:(NSPoint)newOrigin
{
    static BOOL isScrolling = FALSE;
    
    if (isScrolling) {
        return;
    }
    isScrolling = TRUE;
    
    NSDictionary *userInfo = [NSDictionary dictionaryWithObject:[NSValue valueWithPoint:newOrigin]
                                                         forKey:@"newOrigin"];
    [defaultNfc postNotificationName:kCanvasClipViewWillScrollNotification
                              object:self
                            userInfo:userInfo];
    [super scrollToPoint:newOrigin];
    [defaultNfc postNotificationName:kCanvasClipViewDidScrollNotification object:self];
    
    isScrolling = FALSE;
}

@end
