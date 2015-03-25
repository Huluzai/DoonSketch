//
//  ZGBox.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-20.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "ZGContainer.h"

@interface ZGBox : ZGContainer

@property CGFloat space;

- (void)packStart:(NSView *)view;
- (void)packEnd:(NSView *)view;

@end

@interface ZGBox (Override)
- (CGSize)increment;
- (void)sizeToFit;
@end


@interface ZGHBox : ZGBox
@end

@interface ZGVBox : ZGBox
@end