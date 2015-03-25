//
//  FixedCanvasView.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/24.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "CocoaCanvasView.h"

extern NSString *kFixedCanvasChangedOriginNotification;

@interface FixedCanvasView : CocoaCanvasView

@property CGPoint origin;

@end
