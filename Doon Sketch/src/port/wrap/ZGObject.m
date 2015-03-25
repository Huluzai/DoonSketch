//
//  ZGWrapper.m
//  Inxcape
//
//  Created by 张 光建 on 14-10-12.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGObject.h"
#import "ZGItem.h"
#import "ZGObject-Private.h"

#import <sp-item.h>
#import <sp-object.h>
#import <xml/node.h>
#import <xml/node-event-vector.h>

NSString *kZGGObjectDidReleasedNotification = @"ZGGObjectDidReleasedNotification";

@interface ZGGObject () {
    GObject *_gObj;
    BOOL _copy;
}

@end

static void on_object_weak_removed(gpointer data, GObject *object)
{
    g_assert(data && object);
    
    ZGGObject *zObj = (__bridge ZGGObject *)data;
    [zObj didObjectReleased:zObj];
}

@implementation ZGGObject

- (id)initWithGObject:(GObject *)object takeCopy:(BOOL)copy
{
    if (self = [super init]) {
        _copy = copy;
        if (!copy) {
            /* A #GWeakNotify function can be added to an object as a callback that gets
            * triggered when the object is finalized. Since the object is already being
            * finalized when the #GWeakNotify is called, there's not much you could do
            * with the object, apart from e.g. using its address as hash-index or the like.
            */
            //g_message("add weak ref of %p to %p func:%p", object, self, &on_object_weak_removed);
            g_object_weak_ref(object, (GWeakNotify)&on_object_weak_removed, (__bridge gpointer)self);
            _gObj = object;
        } else {
            _gObj = G_OBJECT(g_object_ref(object));
        }
    }
    return self;
}

- (id)initWithGObject:(GObject *)object
{
    return [self initWithGObject:object takeCopy:FALSE];
}

+ (id)wrapGObject:(GObject *)spObj
{
    ZGGObject *obj = [[ZGGObject alloc] initWithGObject:spObj];
    return obj;
}

- (void)dealloc
{
    if (_gObj) {
        if (_copy) {
            g_object_unref(_gObj);
        } else {
            GObject *p = _gObj;
            //g_message("remove weak ref of %p to %p, func:%p", p, self, &on_object_weak_removed);
            g_object_weak_unref(p, (GWeakNotify)&on_object_weak_removed, (__bridge gpointer)self);
        }
        _gObj = NULL;
    }
}

- (GObject *)gObj
{
    return _gObj;
}

- (void *)object
{
    return self.gObj;
}

#pragma mark - delegate methods

- (void)didObjectReleased:(ZGGObject *)object
{
    /* !!! the object is already being finalized !!!*/
    _gObj = NULL;
    
    [defaultNfc postNotificationName:kZGGObjectDidReleasedNotification object:self];
    
    if ([self.delegate respondsToSelector:@selector(didObjectReleased:)]) {
        [self.delegate didObjectReleased:self];
    }
}

@end


struct ZGObjectProxy {
    
    __weak ZGObject *_owner;
    sigc::connection _conMdf;
    
    ZGObjectProxy(ZGObject *owner) {
        _owner = owner;
        _conMdf = _owner.spObj->connectModified(sigc::mem_fun(*this, &ZGObjectProxy::on_object_modified));
    }
    
    virtual ~ZGObjectProxy() {
        _conMdf.disconnect();
        _owner = nill;
    }
    
    void on_object_modified(SPObject *object, int flags) {
        [_owner didObjectModified:nill];
    }
};

static void on_repr_added_child(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void * data)
{}

static void on_repr_removed_child(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void * data)
{}


NSString *kZGObjectDidModifiedNotification = @"ZGObjectDidModifiedNotification";

@interface ZGObject () {
    Inkscape::XML::NodeEventVector _reprListener;
    ZGObjectProxy *_proxy;
}
- (void)objectAddedChild;

@end


@implementation ZGObject

+ (id)wrapSPObject:(SPObject *)spObj
{
    return [[ZGObject alloc] initWithSPObject:spObj];
}

// no retain
- (id)initWithSPObject:(SPObject *)object
{
    if (SP_IS_ITEM(object)) {
        return [[ZGItem alloc] initWithSPItem:SP_ITEM(object)];
    }
    
    return [self initWithSPObject:object takeCopy:FALSE];
}

//
- (id)initWithSPObject:(SPObject *)object takeCopy:(BOOL)copy
{
    if (self = [super initWithGObject:object takeCopy:copy]) {
        _reprListener.child_added = &on_repr_added_child;
        _reprListener.child_removed = &on_repr_removed_child;
        sp_repr_add_listener(SP_OBJECT_REPR(self.spObj), &_reprListener, (__bridge void *)self);
        _proxy = new ZGObjectProxy(self);
    }
    return self;
}

- (void)dealloc
{
    delete _proxy;
    _proxy = NULL;
    if (self.spObj) {
        sp_repr_remove_listener_by_data(SP_OBJECT_REPR(self.spObj), (__bridge void *)self);
    }
}

- (SPObject *)spObj
{
    return SP_OBJECT([super gObj]);
}

- (void)didObjectModified:(ZGObject *)sender
{
    if ([self.delegate respondsToSelector:@selector(didObjectModified:)]) {
        [self.delegate didObjectModified:self];
    }
    
    [defaultNfc postNotificationName:kZGObjectDidModifiedNotification object:self];
}

- (BOOL)isEqualTo:(id)object
{
    if (self == object || (self.spObj == [object spObj])) {
        return TRUE;
    }
 
    if (![object isKindOfClass:[self class]]) {
        return FALSE;
    }
    
    if (!self.spObj || ![object spObj]) {
        return FALSE;
    }
    
    if (self.spObj->getId() && [object spObj]->getId()) {
        return !strcmp(self.spObj->getId(), [object spObj]->getId());
    }
    
    return FALSE;
}

- (BOOL)isEqual:(id)object
{
    return [self isEqualTo:object];
}

@end

