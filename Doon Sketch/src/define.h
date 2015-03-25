//
//  define.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-25.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#include <CoreGraphics/CoreGraphics.h>

#ifndef Inxcape_define_h
#define Inxcape_define_h

#define ZGFLOAT_NOT_SET       (261412.649f)
#define ZGFLOAT_MULTIPLE        (ZGFLOAT_NOT_SET - 0.1f)

#define ZGINT_NOT_SET          (261412649)
#define ZGINT_MULTIPLE          (ZGINT_NOT_SET - 1)

#define ZGSTRING_NOT_SET      (@"__NOT_SET__")
#define ZGSTRING_MULTIPLE       (@"__MULTI__")

#define ZGSmallFloat (0.001f)

static bool
__ZGFloatEqualToFloat(CGFloat f1, CGFloat f2)
{
    return fabs(f1 - f2) < ZGSmallFloat;
}
#define ZGFloatEqualToFloat __ZGFloatEqualToFloat

static bool
__ZGPointEqualToPoint(CGPoint p1, CGPoint p2)
{
    return ZGFloatEqualToFloat(p1.x, p2.x) && ZGFloatEqualToFloat(p1.y, p2.y);
}
#define ZGPointEqualToPoint __ZGPointEqualToPoint

static bool
__ZGSizeEqualToSize(CGSize s1, CGSize s2)
{
    return ZGFloatEqualToFloat(s1.width, s2.width) && ZGFloatEqualToFloat(s1.height, s2.height);
}
#define ZGSizeEqualToSize __ZGSizeEqualToSize

#define ZGFloatValids(fl)        (!ZGFloatEqualToFloat((fl), ZGFLOAT_NOT_SET) && !ZGFloatEqualToFloat((fl), ZGFLOAT_MULTIPLE))
#define ZGIntValids(num)        ((num) != ZGINT_NOT_SET && (num) != ZGINT_MULTIPLE)

#define defaultNfc          ([NSNotificationCenter defaultCenter])
#define standardDefaults    ([NSUserDefaults standardUserDefaults])

#endif
