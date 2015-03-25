//
//  CocoaAction.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Foundation/Foundation.h>

struct SPDesktop;

@interface CocoaAction : NSObject
@property BOOL sensitive;
@end

@interface CocoaActionGroup : CocoaAction
- (void)addAction:(CocoaAction *)act;
@end

@interface CocoaVerbAction : CocoaAction
- (id)initWithButton:(NSButton *)button verb:(int)verbId desktop:(SPDesktop *)desktop;
@end
