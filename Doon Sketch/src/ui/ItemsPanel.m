//
//  ItemsPanel.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-15.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ItemsPanel.h"
#import "PXListView.h"
#import <document.h>
#import <sp-object.h>
#import <selection.h>
#import <gc.h>
#import <desktop-handles.h>
#import <desktop.h>
#import <glib.h>
#import <verbs.h>
#import <helper/action.h>
#import <glibmm.h>

#import "ObjectsView.h"

using namespace Inkscape;

@interface ItemsPanel (Private)
- (void)didDesktopDocumentReplaced:(SPDocument *)doc;
- (void)didDocumentModified:(guint)flags;
- (void)didSelectionChanged:(Selection *)sel;
- (void)didDeleteObject:(SPObject *)obj;
@end

#pragma mark -
#pragma mark SignalAgent

typedef struct {
    __weak id owner;
    void didDocumentModified(guint flags) {
        [owner didDocumentModified:flags];
    }
    void didDesktopDocumentReplaced(SPDesktop *dt, SPDocument *doc) {
        [owner didDesktopDocumentReplaced:doc];
    }
    void didSelectionChanged(Selection *sel) {
        [owner didSelectionChanged:sel];
    }
    void didDeleteObject(SPObject *obj) {
        [owner didDeleteObject:obj];
    }
} SignalAgent;

SignalAgent *newSignalAgent(id owner)
{
    SignalAgent *agent = new SignalAgent;
    agent->owner = owner;
    return agent;
}


#pragma mark -
#pragma mark OutlineView item

@interface DrawingItem : NSObject

@property SPObject *spObj;
@property SignalAgent *sa;
@property sigc::connection delObjectConnection;
@property id owner;

- (id)initWithSPObject:(SPObject *)object owner:(id)owner;
+ (id)itemWithSPObject:(SPObject *)obj owner:(id)owner;
@end


@implementation DrawingItem

- (id)initWithSPObject:(SPObject *)object owner:(id)owner
{
    if (self = [super init]) {
        self.owner = owner;
        self.spObj = sp_object_ref(object);
        self.sa = newSignalAgent(self.owner);
        self.delObjectConnection = self.spObj->connectDelete(sigc::mem_fun(self.sa, &SignalAgent::didDeleteObject));
    }
    return self;
}

- (void)dealloc
{
    self.owner = NULL;
    self.delObjectConnection.disconnect();
    sp_object_unref(self.spObj);
    delete self.sa;
}

+ (id)itemWithSPObject:(SPObject *)obj owner:(id)owner
{
    return [[DrawingItem alloc] initWithSPObject:obj owner:owner];
}

@end


#pragma mark -
#pragma mark ItemsPanel implementation

@interface ItemsPanel () {
    SPDesktop *_desktop;
}

@property BOOL codeSelecting;
@property SPDocument *document;
@property Selection *selection;

@property NSMutableDictionary *tableItems;
@property SignalAgent *sigAgent;

@property sigc::connection docModifiedConnection;
@property sigc::connection docReplacedConnection;
@property sigc::connection selectionChangedConnection;

@end


#pragma mark -
#pragma ItemsPanel implementation

@implementation ItemsPanel

- (id)init
{
    return [self initWithOutlineView:nill];
}

- (id)initWithOutlineView:(NSOutlineView *)anOutlineView;
{
    if (self = [super init]) {
        self.sigAgent = newSignalAgent(self);
        self.document = NULL;
        self.desktop = NULL;
        self.outlineView = anOutlineView;
        self.tableItems = [NSMutableDictionary dictionary];
    }
    return self;
}

- (void)dealloc
{
    sp_document_unref(self.document);
    GC::release(self.desktop);
    GC::release(self.selection);
    self.tableItems = nill;
    delete self.sigAgent;
}

- (void)setDesktop:(SPDesktop *)dt
{
    self.docReplacedConnection.disconnect();
    
    if (_desktop) {
        GC::release(_desktop);
    }
    
    _desktop = dt;
    
    if (self.selection) {
        GC::release(self.selection);
        self.selection = NULL;
    }
    
    if (_desktop) {
        GC::anchor(_desktop);
        
        self.docReplacedConnection = _desktop->connectDocumentReplaced(sigc::mem_fun(self.sigAgent, &SignalAgent::didDesktopDocumentReplaced));
        [self didDesktopDocumentReplaced:sp_desktop_document(_desktop)];
        
        self.selectionChangedConnection.disconnect();
        self.selection = sp_desktop_selection(_desktop);
        g_assert(self.selection);
        GC::anchor(self.selection);
        [self didSelectionChanged:self.selection];
        self.selectionChangedConnection = self.selection->connectChanged(sigc::mem_fun(self.sigAgent, &SignalAgent::didSelectionChanged));
    }
}

- (SPDesktop *)desktop
{
    return _desktop;
}

- (DrawingItem *)findItemForObject:(SPObject *)obj
{
    return [self.tableItems objectForKey:[self objectKey:obj]];
}

- (int)itemChildCount:(SPObject *)obj
{
    int cnt = 0;
    for (SPObject *child = obj->firstChild(); child; child = child->getNext()) {
        if (SP_IS_ITEM(child)) {
            cnt++;
        }
    }
    return cnt;
}

- (SPObject *)itemChildOfObject:(SPObject *)obj atIndex:(int)index
{
    SPObject *child = NULL;
    index += 1;
    
    for (child = obj->firstChild(); child; child = child->getNext()) {
        if (SP_IS_ITEM(child)) {
            if (--index <= 0) {
                break;
            }
        }
    }
    
    return index == 0 ? child : NULL;
}

- (NSString *)objectKey:(SPObject *)obj
{
    return [NSString stringWithUTF8String:obj->getId()];
}

- (void)didDeleteObject:(SPObject *)obj
{
    NSString *key = [NSString stringWithUTF8String:obj->getId()];
    [self.tableItems removeObjectForKey:key];
}

- (void)didDesktopDocumentReplaced:(SPDocument *)doc
{
    self.codeSelecting = YES;
    
    self.docModifiedConnection.disconnect();
    sp_document_unref(self.document);
    self.tableItems = [NSMutableDictionary dictionary];
    
    self.document = sp_document_ref(sp_desktop_document(self.desktop));
    [self didDocumentModified:0];
    
    if (self.document) {
        sp_document_ref(self.document);
        self.docModifiedConnection = self.document->connectModified(sigc::mem_fun(self.sigAgent, &SignalAgent::didDocumentModified));
        [self rebuildDrawingItems:SP_DOCUMENT_ROOT(self.document) intoDictionary:self.tableItems];
    }
    
    self.codeSelecting = NO;
}

- (void)didDocumentModified:(guint)flags
{
    self.codeSelecting = YES;
    [self.outlineView reloadData];
    self.codeSelecting = NO;
}

- (void)rebuildDrawingItems:(SPObject *)obj intoDictionary:(NSMutableDictionary *)dic
{
    for (int i = 0; i < [self itemChildCount:obj]; i++) {
        [dic setObject:[DrawingItem itemWithSPObject:obj owner:self] forKey:[self objectKey:obj]];
        [self rebuildDrawingItems:[self itemChildOfObject:obj atIndex:i] intoDictionary:dic];
    }
}

- (void)expandToItem:(DrawingItem *)item
{
    SPObject *parent = SP_OBJECT_PARENT(item.spObj);
    DrawingItem *pi = NULL;
    if (parent && (pi = [self findItemForObject:parent])) {
        [self expandToItem:pi];
        [self.outlineView expandItem:pi];
    }
}

- (void)didSelectionChanged:(Selection *)sel
{
    self.codeSelecting = YES;
    
    g_assert((sel == self.selection) && sel);
    GSList *items = (GSList *)sel->list();
    GSList *selected = [self createSelectedList];
    
    if (items && ![self list:items equalToList:selected]) {
        for (int i = 0; i < g_slist_length(items); i++) {
            SPObject *obj = (SPObject *)g_slist_nth_data(items, i);
            DrawingItem *di = [self findItemForObject:obj];
            if (!di) {
                di = [DrawingItem itemWithSPObject:obj owner:self];
                [self.tableItems setObject:di forKey:[self objectKey:obj]];
                [self.outlineView reloadData];
            }
            
            [self expandToItem:di];
        }
        
        NSMutableIndexSet *idxes = [NSMutableIndexSet indexSet];
        
        for (int i = 0; i < g_slist_length(items); i++) {
            SPObject *obj = (SPObject *)g_slist_nth_data(items, i);
            DrawingItem *di = [self findItemForObject:obj];
            NSUInteger row = [self.outlineView rowForItem:di];
            [idxes addIndex:row];
        }
        
        [self.outlineView selectRowIndexes:idxes byExtendingSelection:NO];
        [self.outlineView scrollRowToVisible:idxes.firstIndex];
    } else if (!items) {
        [self.outlineView deselectAll:nill];
    }
    
    g_slist_free(selected);
    self.self.codeSelecting = NO;
}


#pragma mark -
#pragma mark List View Delegate Methods


- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
    SPObject *parent = NULL;
    
    if (!item && self.document) {
        parent = SP_DOCUMENT_ROOT(self.document);
    } else if (item) {
        parent = [item spObj];
    }
    
    if (parent) {
        g_assert([self itemChildCount:parent] > index);
        
        SPObject *child = [self itemChildOfObject:parent atIndex:index];
        
        if (!child || !child->getId()) {
            g_error("Child %p has not an Id", child);
        }
        
        NSString *key = [NSString stringWithUTF8String:child->getId()];
        DrawingItem *item = [self.tableItems objectForKey:key];
        if (item) {
            return item;
        } else {
            item = [DrawingItem itemWithSPObject:child owner:self];
            [self.tableItems setObject:item forKey:key];
        }
    }
    
    return nill;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    return [self itemChildCount:[item spObj]];
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    SPObject *parent = NULL;
    
    if (!item && self.document) {
        parent = SP_DOCUMENT_ROOT(self.document);
    } else if (item) {
        parent = [item spObj];
    }
    
    if (parent && parent->hasChildren()) {
        return [self itemChildCount:parent];
    }
    return 0;
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    SPObject *spObj = [item spObj];
    
    char const* title = spObj ? spObj->title() : NULL;
    title = title ? title : SP_OBJECT_ID(spObj);
//    title = title ? title : spObj->label();
//    title = title ? title : spObj->defaultLabel();
//    title = title ? title : spObj->desc();
    return [NSString stringWithFormat:@"%s", title ? title : "[noname]"];
}

- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    [item spObj]->setTitle([object UTF8String]);
    g_debug("Set title as [%s] result : [%s]", [object UTF8String], [item spObj]->title());
}

#pragma mark -
#pragma mark NSOutlineViewDelegate implementation

- (BOOL)list:(GSList *)li1 equalToList:(GSList *)li2
{
    if ((!li1 || !li1) && (li1 != li2)) {
        return NO;
    }
    
    if (g_slist_length(li1) != g_slist_length(li2)) {
        return NO;
    }
    while (li1) {
        if (!g_slist_find(li2, li1)) {
            return NO;
        }
        li1 = g_slist_next(li1);
    }
    return YES;
}

- (GSList *)createSelectedList
{
    NSIndexSet *idxes = [self.outlineView selectedRowIndexes];
    GSList *list = NULL;
    
    if (idxes) {
        NSUInteger idx = [idxes firstIndex];
        while (idx != NSNotFound) {
            DrawingItem *item = [self.outlineView itemAtRow:idx];
            list = g_slist_append(list, item.spObj);
            idx = [idxes indexGreaterThanIndex:idx];
        }
    }
    return list;
}
/*
- (void)outlineViewSelectionDidChange:(NSNotification *)notification
{
    if (self.codeSelecting) {
        return;
    }
    
    GSList *selected = [self createSelectedList];
    if (selected && ![self list:selected equalToList:(GSList *)self.selection->itemList()]) {
        self.selection->setList(selected);
    } else if (!selected) {
        self.selection->clear();
    }
    g_slist_free(selected);
}
*/
- (void)outlineView:(NSOutlineView *)outlineView didClickTableColumn:(NSTableColumn *)tableColumn
{
    
}

- (void)objectsView:(ObjectsView*)anObjectsView didClickTableRow:(NSInteger)row
{

    if (row >= 0 && ![self.outlineView isRowSelected:row]) {
        DrawingItem *item = [self.outlineView itemAtRow:row];
        self.desktop->setCurrentLayer(SP_OBJECT_PARENT(item.spObj));
        self.selection->toggle(item.spObj);
    }
}

- (BOOL)hasChildrenSelected:(DrawingItem *)item
{
    for (int i = 0; i < [self itemChildCount:[item spObj]]; i++) {
        SPObject *im = [self itemChildOfObject:item.spObj atIndex:i];
        DrawingItem *ci = [self findItemForObject:im];
        g_assert(ci);
        NSUInteger row = [self.outlineView rowForItem:ci];
        if ([self.outlineView isRowSelected:row]) {
            return YES;
        }
        if ([self hasChildrenSelected:ci]) {
            return YES;
        }
    }
    return NO;
}

- (void)deselectChildren:(DrawingItem *)item
{
    for (int i = 0; i < [self itemChildCount:[item spObj]]; i++) {
        SPObject *im = [self itemChildOfObject:item.spObj atIndex:i];
        DrawingItem *ci = [self findItemForObject:im];
        g_assert(ci);
        NSUInteger row = [self.outlineView rowForItem:ci];
        [self.outlineView deselectRow:row];
        [self deselectChildren:ci];
    }
}
/*
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
    [self deselectChildren:item];
    
    // Deselect parents
    SPObject *obj = [item spObj];
    
    while ((obj = SP_OBJECT_PARENT(obj))) {
        DrawingItem *it = [self findItemForObject:obj];
        NSUInteger row = [self.outlineView rowForItem:it];
        [self.outlineView deselectRow:row];
    }
    return TRUE;
}
*/

#pragma mark -
#pragma mark Actions

- (IBAction)deleteItem:(id)sender
{
    NSUInteger row = [self.outlineView clickedRow];
    NSIndexSet *sels = [self.outlineView selectedRowIndexes];
    
    if (row == -1 || [sels containsIndex:row]) {
        // Delete selected objects
        Verb *dv = Verb::get(SP_VERB_EDIT_DELETE);
        SPAction *act = dv->get_action(self.desktop);
        sp_action_perform(act, NULL);
    } else {
        // Delete clicked object
        DrawingItem *item = [self.outlineView itemAtRow:row];
        item.spObj->deleteObject(true, true);
        sp_document_done(self.document, SP_VERB_EDIT_DELETE, "Delete");
    }
}

@end
