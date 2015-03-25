//
//  ZGGObjectWrapper.h
//  Inxcape
//
//  Created by 张 光建 on 14-10-12.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ZGWrapper : NSObject

@property (weak) id delegate;

- (id)initWithObject:(void *)object;
+ (id)wrapperOfObject:(void *)object;

- (void *)object;

@end
