//
//  ZGDesktop.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/5.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGDesktop.h"
#import "ZGDesktopStyle.h"
#import "ZGDocument.h"
#import "ZGSelection.h"

#include <selection.h>
#include <desktop.h>
#include <desktop-handles.h>

NSString *kZGDesktopSubselectionChanged = @"ZGDesktopSubselectionChanged";
NSString *kZGDesktopEventContextDidChangeNotification = @"ZGDesktopEventContextDidChangeNotification";

class ZGDesktopProxy {
public:
    
    ZGDesktopProxy(ZGDesktop *dt) {
        _dt = dt;
        _conSubselectionChanged = _dt.spDesktop->connectToolSubselectionChanged(sigc::mem_fun(*this, &ZGDesktopProxy::on_subselection_changed));
        _conToolChanged = _dt.spDesktop->connectEventContextChanged(sigc::mem_fun(*this, &ZGDesktopProxy::on_tool_changed));
    }
    
    virtual ~ZGDesktopProxy() {
        _dt = nill;
        _conSubselectionChanged.disconnect();
    }
    
    void on_subselection_changed(gpointer data) {
        [_dt desktop:_dt didChangedSubselection:data];
    }
    
    void on_tool_changed(SPDesktop *desktop, SPEventContext *e) {
        [_dt didChangedEventContext:e];
    }
private:
    __weak ZGDesktop *_dt;
    sigc::connection _conToolChanged;
    sigc::connection _conSubselectionChanged;
};


@interface ZGDesktop () {
    SPDesktop *_desktop;
}
@property NSArrayController *objectsController;
@property SPDesktop *desktop;
@property ZGDocument *zdoc;
@property ZGDesktopProxy *proxy;
@property ZGSelection *zSel;
@end

@implementation ZGDesktop

- (id)initWithSPDesktop:(SPDesktop *)desktop
{
    if (self = [super init]) {
        _desktop = Inkscape::GC::anchor(desktop);
        self.zdoc = [[ZGDocument alloc] initWithSPDocument:sp_desktop_document(desktop)];
        self.zdoc.delegate = self;
        self.zSel = [[ZGSelection alloc] initWithSPDesktop:desktop];
        self.zSel.delegate = self;
        self.proxy = new ZGDesktopProxy(self);
        self.objectsController = [[NSArrayController alloc] initWithContent:self.zdoc.drawingObjects];
        [self.objectsController setContent:self.zdoc.drawingObjects];
    }
    return self;
}

- (void)dealloc
{
    self.zSel = nill;
    delete self.proxy;
    self.proxy = NULL;
    self.objectsController = nill;
    self.zdoc = nill;
    Inkscape::GC::release(_desktop);
    _desktop = NULL;
}

- (void)didChangedEventContext:(SPEventContext *)eventCtx
{
    if ([self.delegate respondsToSelector:@selector(desktop:didChangedEventContext:)]) {
        [self.delegate desktop:self didChangedEventContext:eventCtx];
    }
    [defaultNfc postNotificationName:kZGDesktopEventContextDidChangeNotification object:self];
}

- (void)didSelectionChanged:(ZGSelection *)selection
{
//    BOOL r = [self.objectsController setSelectedObjects:self.zSel.selectedItems];
//    g_assert(r);
//    NSArray *objs = [self.objectsController arrangedObjects];
//    g_debug("obj count = %d, selected = %d", [objs count], self.zSel.selectedItems.count);
}

- (void)desktop:(ZGDesktop *)desktop didChangedSubselection:(gpointer)pointer
{
    if ([self.delegate respondsToSelector:@selector(desktop:didChangedSubselection:)]) {
        [self.delegate desktop:self didChangedSubselection:pointer];
    }
    NSDictionary *usrInfo = [NSDictionary dictionaryWithObject:[NSValue valueWithPointer:pointer] forKey:@"data"];
    [defaultNfc postNotificationName:kZGDesktopSubselectionChanged object:self userInfo:usrInfo];
}

- (SPDesktop *)spDesktop
{
    return _desktop;
}

- (id)selection
{
    return self.objectsController.selection;
}

@end
