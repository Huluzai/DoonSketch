//
//  AttributesViewController.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-19.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <desktop.h>
#import <document.h>
#import <inkscape.h>
#import <selection.h>
#import <verbs.h>
#import <selection.h>
#import <desktop-handles.h>
#import <xml/repr.h>
#import <attributes.h>
#import <sp-item-transform.h>

#import "AttributesPage.h"
#import "ZGBox.h"
#import "ZGAdjustment.h"
#import "ZGAttribute.h"
#import "ZGAdjustment+Attribute.h"

typedef struct {
    int intValue;
    float floatValue;
    double doubleValue;
    char *stringValue;
} AttrValue;

using namespace Inkscape;

@interface AttributesPage (PreDefine)
- (void)didDesktopDocumentReplaced:(SPDocument *)doc;
- (void)didSelectionChanged:(Selection *)selection;
@end


class AttrSignalProxy {
public:
    AttrSignalProxy(id anObject) {
        owner = anObject;
    }
    
    virtual ~AttrSignalProxy() {
        owner = nill;
    }
    
    void didDocumentModified(guint flags) {
        [owner updateValues];
    }
    void didDesktopDocumentReplaced(SPDesktop *dt, SPDocument *doc) {
        [owner didDesktopDocumentReplaced:doc];
    }
    void didSelectionChanged(Selection *sel) {
        [owner didSelectionChanged:sel];
    }
    
    __weak id owner;
    sigc::connection docReplacedConnection;
    sigc::connection docModifiedConnection;
    sigc::connection selChangeConnection;
};

static AttrSignalProxy *newProxy(id owner)
{
    AttrSignalProxy *agent = new AttrSignalProxy(owner);
    return agent;
}

static void destroyProxy(AttrSignalProxy *proxy)
{
    proxy->docReplacedConnection.disconnect();
    proxy->docModifiedConnection.disconnect();
    proxy->selChangeConnection.disconnect();
    delete proxy;
}

static void connectProxyWithDesktop(AttrSignalProxy *proxy, SPDesktop *dt)
{
    proxy->docModifiedConnection = dt->connectDocumentReplaced(sigc::mem_fun(proxy, &AttrSignalProxy::didDesktopDocumentReplaced));
    sp_desktop_selection(dt)->connectChanged(sigc::mem_fun(proxy, &AttrSignalProxy::didSelectionChanged));
}

static void disconnectProxyWithDesktop(AttrSignalProxy *proxy)
{
    proxy->docReplacedConnection.disconnect();
    proxy->selChangeConnection.disconnect();
}

static void connectProxyWithDocument(AttrSignalProxy *proxy, SPDocument *doc)
{
    doc->connectModified(sigc::mem_fun(proxy, &AttrSignalProxy::didDocumentModified));
}

static void disconnectProxyWithDocument(AttrSignalProxy *proxy)
{
    proxy->docModifiedConnection.disconnect();
}


@interface AttrInfo : NSObject
@property NSArray *itemTypes; // Which type tiems to setting
@property NSString *attrName;
@property ZGAdjustmentType valueType;
@property NSUInteger verbId;
@property ZGAdjustment *adjustment;

+ (id)attrInfoWithItemTypes:(NSArray *)iTypes
                   attrName:(NSString*)name
                  valueType:(ZGAdjustmentType)vType
                     verbId:(NSUInteger)verb;
@end

@implementation AttrInfo
+ (id)attrInfoWithItemTypes:(NSArray *)iTypes
                   attrName:(NSString*)name
                  valueType:(ZGAdjustmentType)vType
                     verbId:(NSUInteger)verb
{
    AttrInfo *info = [[AttrInfo alloc] init];
    info.itemTypes = iTypes;
    info.attrName = name;
    info.valueType = vType;
    info.verbId = verb;
    return info;
}
@end


@interface AttributesPage () {
    SPDesktop *_desktop;
}

@property NSMutableDictionary *attrDict;
@property NSView *priView;
@property AttrSignalProxy *signalProxy;
@property ZGVBox *vbox;
@property NSMutableArray *adjustments;

@end


@implementation AttributesPage

- (id)initWithDesktop:(SPDesktop *)desktop
{
    if (self = [super init]) {
        self.priView = [[NSView alloc] init];
        self.vbox = [[ZGVBox alloc] initWithView:self.view];
        self.attrDict = [NSMutableDictionary dictionary];
        self.signalProxy = newProxy(self);
        self.desktop = desktop;
        self.adjustments = [NSMutableArray array];
    }
    
    return self;
}

- (id)init
{
    return [self initWithDesktop:NULL];
}

- (void)dealloc
{
    if (self.signalProxy) {
        destroyProxy(self.signalProxy);
    }
    self.vbox = nill;
    self.priView = nill;
    self.attrDict = nill;
    self.adjustments = nill;
}

- (void)setDesktop:(SPDesktop *)dt
{
    disconnectProxyWithDesktop(self.signalProxy);
    disconnectProxyWithDocument(self.signalProxy);

    SPDesktop *p = dt ? GC::anchor(dt) : NULL;
    if (_desktop) {
        GC::release(_desktop);
    }
    _desktop = p;
    
    if (_desktop) {
        connectProxyWithDesktop(self.signalProxy, _desktop);
        if (sp_desktop_document(_desktop)) {
            connectProxyWithDocument(self.signalProxy, sp_desktop_document(_desktop));
        }
    }
    
    [self updateValues];
}

- (SPDesktop *)desktop
{
    return _desktop;
}

- (NSView *)view
{
    return self.priView;
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)sizeToFit
{
    [self.vbox sizeToFit];
}

- (void)updateValues
{
    static BOOL isUpdating = NO;
    
    if (isUpdating || !self.desktop) {
        return;
    }
    isUpdating = YES;
    
    Selection *selection = sp_desktop_selection(self.desktop);
    
    for (ZGAdjustment *adj in self.adjustments) {
        
        NSUInteger attrId = [self attrIdWithAdjustment:adj];
        ZGAttribute *attr = [ZGAttribute attributeWithCode:attrId];
        
        [adj clearValue];
        
        for (GSList const *items = selection->itemList(); items != NULL; items = items->next) {
            
            SPItem *item = SP_ITEM(items->data);
            
            if (![ZGAttribute itemClass:G_TYPE_FROM_INSTANCE(item) hasAttritube:attrId]) {
                continue;
            }

            adj.value = [attr valueOfItem:item];
        }
    }
    
    isUpdating = NO;
}

- (void)didAdjustmentValueChanged:(ZGAdjustment *)adj
{
    SPDocument *doc = sp_desktop_document(self.desktop);
    Selection *selection = sp_desktop_selection(self.desktop);
    GSList const *items = selection->itemList();
    BOOL modmade = FALSE;
    NSUInteger attrId = [self attrIdWithAdjustment:adj];
    ZGAttribute *attr = [ZGAttribute attributeWithCode:attrId];
    
    for (; items != NULL; items = items->next) {
        
        SPItem *item = SP_ITEM(items->data);
        
        if (![ZGAttribute itemClass:G_TYPE_FROM_INSTANCE(item) hasAttritube:attrId]) {
            continue;
        }
        
        if (![attr isValidValue:adj.value]) {
            g_warning("Invalid value");
        } if (attr) {
            [attr setValue:adj.value ofItem:item];
            SP_OBJECT(items->data)->updateRepr();
            modmade = TRUE;
        }
    }
    
    if (modmade) {
        NSString *actionName = [NSString stringWithFormat:@"Modify attribute %@", attributeNameWithCode(attrId)];
        sp_document_done(doc, SP_VERB_CONTEXT_SELECT, [actionName UTF8String]);      // TODO: How to do this ?
    }
}

- (BOOL)hasAttribute:(NSUInteger)attr inSelection:(Selection *)selection
{
    GSList const *items = selection->itemList();
    
    for (; items != NULL; items = items->next) {
        BOOL a = [ZGAttribute itemClass:G_TYPE_FROM_INSTANCE(items->data) hasAttritube:attr];
        if (a) {
            return YES;
        }
    }
    
    return NO;
}

- (void)didDesktopDocumentReplaced:(SPDocument *)doc
{
    Selection *selection = sp_desktop_selection(self.desktop);
    [self didSelectionChanged:selection];
}

- (void)didSelectionChanged:(Selection *)selection
{
    for (ZGAdjustment *adj in self.adjustments) {
        NSUInteger attr = [self attrIdWithAdjustment:adj];
        BOOL hasAttr = [self hasAttribute:attr inSelection:selection];
        g_debug("Attr %s(%d) %s in selection", [attributeNameWithCode(attr) UTF8String], attr, hasAttr ? "is" : "is not");
        [adj.view setHidden:!hasAttr];
    }
    
    [self sizeToFit];
    [self updateValues];
}

- (NSUInteger )attrIdWithAdjustment:(ZGAdjustment *)adj
{
    return [adj.object unsignedIntegerValue];
}

- (void)addAdjustmentWithAttribute:(NSUInteger)attrCode
{
    ZGAdjustment *adj = [ZGAdjustment adjustmentWithAttribute:attrCode];
    g_assert(adj);
    
    adj.target = self;
    adj.selector = @selector(didAdjustmentValueChanged:);
    adj.object = [NSNumber numberWithUnsignedInteger:attrCode];
    
    [self.adjustments addObject:adj];
    [self.vbox packStart:adj.view];
}

@end
