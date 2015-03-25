//
//  ZGWrapper.h
//  Inxcape
//
//  Created by 张 光建 on 14-10-12.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <glib.h>
#import <glib-object.h>
#import "ZGWrapper.h"

extern NSString *kZGObjectDidModifiedNotification;
extern NSString *kZGGObjectDidReleasedNotification;

struct SPObject;

@interface ZGGObject : ZGWrapper

+ (id)wrapGObject:(GObject *)spObj;

// no retain
- (id)initWithGObject:(GObject *)object;
//
- (id)initWithGObject:(GObject *)object takeCopy:(BOOL)copy;
- (GObject *)gObj;

@end


@interface ZGGObject (Delegate)

- (void)didObjectReleased:(ZGGObject *)object;

@end


@interface ZGObject : ZGGObject

+ (id)wrapSPObject:(SPObject *)spObj;

// no retain
- (id)initWithSPObject:(SPObject *)object;

- (SPObject *)spObj;

//
- (void)didObjectModified:(ZGObject *)sender;

@end

