//
//  ZGSelection.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/25.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGSelection.h"

#import <selection.h>
#import <desktop-handles.h>
#import <desktop.h>

NSString *kZGSelectionDidModifiedNotification = @"ZGSelectionDidModifiedNotification";
NSString *kZGSelectionDidChangedNotification = @"ZGSelectionDidChangedNotification";

class ZGSelectionProxy {
public:
    
    ZGSelectionProxy(ZGSelection *zSel) {
        _sel = zSel;
        _conModified = _sel.selection->connectModified(sigc::mem_fun(*this, &ZGSelectionProxy::on_selection_modified));
        _conChanged = _sel.selection->connectChanged(sigc::mem_fun(*this, &ZGSelectionProxy::on_selection_changed));
    }
    
    virtual ~ZGSelectionProxy() {
        _sel = NULL;
        _conModified.disconnect();
        _conChanged.disconnect();
    }
    
    void on_selection_changed(Inkscape::Selection *aSelection) {
        [_sel didSelectionChanged:_sel];
    }
    
    void on_selection_modified(Inkscape::Selection *aSelection, guint flags) {
        [_sel didSelectionModified:_sel flags:flags];
    }
    
private:
    __weak ZGSelection *_sel;
    sigc::connection _conModified;
    sigc::connection _conChanged;
};


@interface ZGSelection ()
@property ZGSelectionProxy *proxy;
@property SPDesktop *dt;
@end

@implementation ZGSelection

- (id)initWithSPDesktop:(SPDesktop *)aDesktop
{
    if (self = [super init]) {
        self.dt = Inkscape::GC::anchor(aDesktop);
        self.proxy = new ZGSelectionProxy(self);
    }
    return self;
}

- (void)dealloc
{
    delete self.proxy;
    self.proxy = NULL;
    Inkscape::GC::release(self.dt);
    self.dt = NULL;
}

- (NSArray *)selectedItems
{
    NSMutableArray *arr = [NSMutableArray array];
    const GSList *items = self.selection->itemList();
    while (items) {
        [arr addObject:[[ZGObject alloc] initWithSPObject:SP_OBJECT(items->data)]];
        items = g_slist_next(items);
    }
    return [NSArray arrayWithArray:arr];
}

- (Inkscape::Selection *)selection
{
    return sp_desktop_selection(self.dt);
}

- (void)didSelectionModified:(ZGSelection *)selection flags:(guint)flags
{
    if ([self.delegate respondsToSelector:@selector(didSelectionModified:flags:)]) {
        [self.delegate didSelectionModified:self flags:flags];
    }
    
    NSDictionary *usrinfo = [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedInt:flags] forKey:@"flags"];
    [defaultNfc postNotificationName:kZGSelectionDidModifiedNotification object:self userInfo:usrinfo];
}

- (void)didSelectionChanged:(ZGSelection *)selection
{
    if ([self.delegate respondsToSelector:@selector(didSelectionChanged:)]) {
        [self.delegate didSelectionChanged:self];
    }
    
    [defaultNfc postNotificationName:kZGSelectionDidChangedNotification object:self];
}

@end
