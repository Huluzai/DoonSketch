//
//  ZGObject-Private.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/28.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#ifndef Inxcape_ZGObject_Private_h
#define Inxcape_ZGObject_Private_h

#include <sp-object.h>

@interface ZGObject (Private)
//
- (id)initWithSPObject:(SPObject *)object takeCopy:(BOOL)copy;

@end

#endif
