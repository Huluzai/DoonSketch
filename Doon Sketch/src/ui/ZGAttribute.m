//
//  ZGAttribute.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-23.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGAttribute.h"
#import "ZGObject.h"
#import "define.h"

#import <sp-item-transform.h>
#import <sp-rect.h>
#import <sp-ellipse.h>
#import <sp-text.h>
#import <sp-flowtext.h>

@interface ZGAttribute ()

@property SEL setter;
@property SEL getter;
@property SEL checker;
@end

static NSDictionary *attrsTable = nill;

@implementation ZGAttribute

+ (id)attrWithCode:(guint)code
            setter:(SEL)setter
            getter:(SEL)getter
           checker:(SEL)checker
{
    ZGAttribute *attr = [[ZGAttribute alloc] init];
    attr.code = code;
    attr.setter = setter;
    attr.getter = getter;
    attr.checker = checker;
    return attr;
}

#define _A(code,setter,getter,checker) ([ZGAttribute attrWithCode:(code) setter:(setter) getter:(getter) checker:(checker)])
#define _S(method)  (@selector(method))
#define _K(code)    ([NSNumber numberWithUnsignedInteger:(code)])
#define _ATT(code,setter,getter,checker)  _A(code,setter,getter,checker),_K(code)

+ (BOOL)itemClass:(GType)classType hasAttritube:(NSUInteger)code
{
    NSArray *baseAttrs = [NSArray arrayWithObjects:
                          _K(ZG_ATTR_POSITION),
                          _K(ZG_ATTR_SIZE),
                          NULL];
    
    NSArray *rectAttrs = [NSArray arrayWithObjects:
                          _K(SP_ATTR_RX),
                          _K(SP_ATTR_RY),
                          NULL];
    
    NSArray *ellipseAttrs = [NSArray arrayWithObjects:
                             _K(SP_ATTR_SODIPODI_START),
                             _K(SP_ATTR_SODIPODI_END),
                             _K(SP_ATTR_SODIPODI_OPEN),
                             NULL];
    
    NSArray *textAttrs = [NSArray arrayWithObjects:_K(ZG_ATTR_TEXT), NULL];
    NSArray *flowtextAttrs = [NSArray arrayWithObjects:_K(ZG_ATTR_TEXT), NULL];
    
    NSDictionary *classAttrs = [NSDictionary dictionaryWithObjectsAndKeys:
                                rectAttrs, _K(SP_TYPE_RECT),
                                ellipseAttrs, _K(SP_TYPE_ARC),
                                flowtextAttrs, _K(SP_TYPE_FLOWTEXT),
                                textAttrs, _K(SP_TYPE_TEXT),
                                NULL];
    
    BOOL isBaseAttr = [baseAttrs indexOfObject:_K(code)] != NSNotFound;
    
    NSArray *attrs = [classAttrs objectForKey:_K(classType)];
    BOOL isTypeAttr = attrs && ([attrs indexOfObject:_K(code)] != NSNotFound);

    return isBaseAttr || isTypeAttr;
}

+ (id)attributeWithCode:(NSUInteger)code
{
    static const ZGAttribute *attrsTbl[ZG_ATTR_COUNT];
    static BOOL initialized = NO;
    
    if (!initialized) {
        
        memset(attrsTbl, 0, sizeof(attrsTbl));
        
        attrsTbl[ZG_ATTR_POSITION] =[ZGAttribute attrWithCode:ZG_ATTR_POSITION
                                                       setter:@selector(setPosition:ofItem:)
                                                       getter:@selector(positionOfItem:)
                                                      checker:@selector(allwaysValid:)];
        
        attrsTbl[ZG_ATTR_SIZE] =  [ZGAttribute attrWithCode:ZG_ATTR_SIZE
                                                     setter:@selector(setSize:ofItem:)
                                                     getter:@selector(sizeOfItem:)
                                                    checker:@selector(isSizeValid:)];
        
        attrsTbl[SP_ATTR_RX] =  [ZGAttribute attrWithCode:SP_ATTR_RX
                                                   setter:@selector(setRectRx:ofItem:)
                                                   getter:@selector(rectRxOfItem:)
                                                  checker:@selector(allwaysValid:)];
        
        attrsTbl[SP_ATTR_RY] =  [ZGAttribute attrWithCode:SP_ATTR_RY
                                                   setter:@selector(setRectRy:ofItem:)
                                                   getter:@selector(rectRyOfItem:)
                                                  checker:@selector(allwaysValid:)];
        
        attrsTbl[SP_ATTR_SODIPODI_START] = [ZGAttribute attrWithCode:SP_ATTR_SODIPODI_START
                                                                   setter:@selector(setArcStart:ofItem:)
                                                                   getter:@selector(arcStartOfItem:)
                                                                  checker:@selector(allwaysValid:)];
        
        attrsTbl[SP_ATTR_SODIPODI_END] = [ZGAttribute attrWithCode:SP_ATTR_SODIPODI_END
                                                                 setter:@selector(setArcEnd:ofItem:)
                                                                 getter:@selector(arcEndOfItem:)
                                                                checker:@selector(allwaysValid:)];
        
        attrsTbl[SP_ATTR_SODIPODI_OPEN] = [ZGAttribute attrWithCode:SP_ATTR_SODIPODI_OPEN
                                                                   setter:@selector(setArcOpen:ofItem:)
                                                                   getter:@selector(arcOpenOfItem:)
                                                                  checker:@selector(allwaysValid:)];
        initialized = YES;
    }
    
    g_assert(code >= 0 && code < ZG_ATTR_COUNT);
    return attrsTbl[code];
}

- (id)valueOfItem:(SPItem *)item
{
    return [self performSelector:self.getter withObject:[ZGObject wrapSPObject:item]];
}

- (void)setValue:(id)value ofItem:(SPItem *)item
{
    [self performSelector:self.setter withObject:value withObject:[ZGObject wrapSPObject:item]];
}

- (BOOL)isValidValue:(id)value
{
    return [[self performSelector:self.checker withObject:value] boolValue];
}

- (NSNumber *)allwaysValid:(id)data
{
    BOOL ret = YES;
    
    if ([data isKindOfClass:[NSNumber class]]) {
        ret = ZGIntValids([data integerValue]) && ZGFloatValids([data floatValue]);
    }
    return [NSNumber numberWithBool:ret];
}

- (void)setPosition:(NSValue *)position ofItem:(ZGObject *)item
{
    CGFloat oldX = [[self positionOfItem:item] pointValue].x;
    CGFloat x = oldX;
    CGFloat oldY = [[self positionOfItem:item] pointValue].y;
    CGFloat y = oldY;
    
    if (ZGFloatValids(position.pointValue.x)) {
        x = position.pointValue.x;
    }
    
    if (ZGFloatValids(position.pointValue.y)) {
        y = position.pointValue.y;
    }
    
    [self moveItem:item toPoint:CGPointMake(x, y)];
}

- (NSValue *)positionOfItem:(ZGObject *)item
{
    Geom::OptRect bbox = [self bboxOfItem:item];
    CGPoint point;
    if (bbox) {
        point = CGPointMake(bbox->min()[Geom::X], bbox->min()[Geom::Y]);
    } else {
        point = CGPointMake(ZGFLOAT_NOT_SET, ZGFLOAT_NOT_SET);
    }
    return [NSValue valueWithPoint:point];
}

- (NSNumber *)isSizeValid:(NSValue *)size
{
    return [NSNumber numberWithBool:(size.sizeValue.width > 0 && size.sizeValue.height > 0)];
}

- (void)setSize:(NSValue *)size ofItem:(ZGObject *)item
{
    CGFloat oldWidth = [self bboxOfItem:item]->width();
    CGFloat width = oldWidth;
    CGFloat oldHeight = [self bboxOfItem:item]->height();
    CGFloat height = oldHeight;
    
    if (ZGFloatValids(size.sizeValue.width)) {
        width = size.sizeValue.width;
    }
    
    if (ZGFloatValids(size.sizeValue.height)) {
        height = size.sizeValue.height;
    }
    
    Geom::Scale sc(width / oldWidth, height / oldHeight);
    sp_item_scale_rel((SPItem *)item.spObj, sc);
}

- (NSValue *)sizeOfItem:(ZGObject *)item
{
    Geom::OptRect bbox = [self bboxOfItem:item];
    CGSize size;
    
    if (bbox) {
        size = CGSizeMake(bbox->max()[Geom::X] - bbox->min()[Geom::X],
                          bbox->max()[Geom::Y] - bbox->min()[Geom::Y]);
    } else {
        size = CGSizeMake(ZGFLOAT_NOT_SET, ZGFLOAT_NOT_SET);
    }
    
    return [NSValue valueWithSize:size];
}

- (void)moveItem:(ZGObject *)obj toPoint:(CGPoint)point
{
    Geom::Translate tr(point.x - [self xOfItem:obj].floatValue,
                       point.y - [self yOfItem:obj].floatValue);
    sp_item_move_rel((SPItem *)obj.spObj, tr);
}

- (NSNumber *)xOfItem:(ZGObject *)item
{
    CGFloat x = 0;
    Geom::OptRect r = [self bboxOfItem:item];
    if (r) {
        x = r->min()[Geom::X];
    }
    return [NSNumber numberWithFloat:x];
}

- (NSNumber *)yOfItem:(ZGObject *)item
{
    CGFloat y = 0;
    Geom::OptRect r = [self bboxOfItem:item];
    if (r) {
        y = r->min()[Geom::Y];
    }
    return [NSNumber numberWithFloat:y];
}

- (Geom::OptRect)bboxOfItem:(ZGObject *)item
{
    return sp_item_bbox_desktop((SPItem *)item.spObj, SPItem::GEOMETRIC_BBOX);
}

- (void)setRectRx:(NSNumber *)rx ofItem:(ZGObject *)item
{
    g_assert(SP_IS_RECT(item.spObj));
    if (ZGFloatValids(rx.floatValue)) {
        sp_rect_set_visible_rx(SP_RECT(item.spObj), rx.floatValue);
    }
}

- (NSNumber *)rectRxOfItem:(ZGObject *)item
{
    g_assert(SP_IS_RECT(item.spObj));
    return [NSNumber numberWithFloat:sp_rect_get_visible_rx(SP_RECT(item.spObj))];
}

- (void)setRectRy:(NSNumber *)ry ofItem:(ZGObject *)item
{
    g_assert(SP_IS_RECT(item.spObj));
    if (ZGFloatValids(ry.floatValue)) {
        sp_rect_set_visible_ry(SP_RECT(item.spObj), ry.floatValue);
    }
}

- (NSNumber *)rectRyOfItem:(ZGObject *)item
{
    g_assert(SP_IS_RECT(item.spObj));
    return [NSNumber numberWithFloat:sp_rect_get_visible_ry(SP_RECT(item.spObj))];
}

#define N2STR(n)    ([[NSString stringWithFormat:@"%@", (n)] UTF8String])

- (void)setArcStart:(NSNumber *)start ofItem:(ZGObject *)item
{
    g_assert(SP_IS_ARC(item.spObj));
    
    SPArc *arc = SP_ARC(item.spObj);
    
    if (ZGFloatValids([start floatValue])) {
        sp_object_set(arc, SP_ATTR_SODIPODI_START, N2STR(start));
    }
}

- (NSNumber *)arcStartOfItem:(ZGObject *)item
{
    g_assert(SP_IS_ARC(item.spObj));
    
    SPGenericEllipse *ge = (SPGenericEllipse *)item.spObj;
    return [NSNumber numberWithFloat:ge->start];
}

- (void)setArcEnd:(NSNumber *)end ofItem:(ZGObject *)item
{
    g_assert(SP_IS_ARC(item.spObj));
    SPArc *arc = SP_ARC(item.spObj);
    if (ZGFloatValids([end floatValue])) {
        sp_object_set(arc, SP_ATTR_SODIPODI_END, N2STR(end));
    }
}

- (NSNumber *)arcEndOfItem:(ZGObject *)item
{
    g_assert(SP_IS_ARC(item.spObj));
    
    SPGenericEllipse *ge = (SPGenericEllipse *)item.spObj;
    return [NSNumber numberWithFloat:ge->end];
}

- (void)setArcOpen:(NSNumber *)open ofItem:(ZGObject *)item
{
    g_assert(SP_IS_ARC(item.spObj));
    SPArc *arc = SP_ARC(item.spObj);
    if (ZGFloatValids([open boolValue])) {
        sp_object_set(arc, SP_ATTR_SODIPODI_OPEN, N2STR(open));
    }
}

- (NSNumber *)arcOpenOfItem:(ZGObject *)item
{
    g_assert(SP_IS_ARC(item.spObj));
    SPGenericEllipse *ge = (SPGenericEllipse *)item.spObj;
    return [NSNumber numberWithBool:!ge->closed];
}

@end


const NSString *attributeNameWithCode(NSUInteger attrCode)
{
    static const char *attrName[] = {
        "position",
        "size",
        "flowtext"
    };
    
    g_assert(attrCode < ZG_ATTR_COUNT);
    
    if (attrCode < ZG_ATTR_FIRST) {
        return [NSString stringWithUTF8String:(char*)sp_attribute_name((int)attrCode)];
    }
    
    return [NSString stringWithUTF8String:attrName[attrCode - ZG_ATTR_FIRST]];
}


