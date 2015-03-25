//
//  ZGDesktop.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/5.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGWrapper.h"
#import <glib.h>

struct SPDesktop;
struct SPDocument;
struct SPObject;
struct SPEventContext;

extern NSString *kZGDesktopSubselectionChanged;
extern NSString *kZGDesktopEventContextDidChangeNotification;

@interface ZGDesktop : ZGWrapper

//retained
- (id)initWithSPDesktop:(SPDesktop *)desktop;
- (SPDesktop *)spDesktop;
- (NSArrayController *)objectsController;

@end


@interface ZGDesktop (Delegate)
- (void)didDesktop:(ZGDesktop *)desktop replacedDocument:(SPDocument *)doc;
- (void)didDesktop:(ZGDesktop *)desktop changedCurrentLayer:(SPObject *)object;
- (void)desktop:(ZGDesktop *)desktop didChangedSubselection:(gpointer)pointer;
- (void)desktop:(ZGDesktop *)desktop didChangedEventContext:(SPEventContext *)eventCtx;
@end

@interface ZGDesktop (Private)
- (void)didChangedEventContext:(SPEventContext *)eventCtx;
@end
