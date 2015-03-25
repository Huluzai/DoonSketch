//
//  StylePagesManager.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/9.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class ZGDesktop;
@class MessagePage;

@interface StylePagesManager : NSViewController

@property ZGDesktop *desktop;

- (void)loadPages;

- (MessagePage *)messagePage;

@end
