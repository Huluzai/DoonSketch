//
//  ZGDocument.m
//  Inxcape
//
//  Created by 张 光建 on 14-10-12.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGDocument.h"
#import "ZGObject.h"

#import <document.h>
#import <document-private.h>
#import <sigc++/sigc++.h>
#import <undo-stack-observer.h>
#import <event.h>

NSString *kZGDocumentWillStartReconstructionNotification = @"ZGDocumentWillStartReconstructionNotification";
NSString *kZGDocumentDidFinishReconstructionNotification = @"ZGDocumentDidFinishReconstructionNotification";
NSString *kZGDocumentDefsModifiedNotification = @"ZGDocumentDefsModifiedNotification";
NSString *kZGDocumentModifiedNotification = @"ZGDocumentModifiedNotification";
NSString *kZGDocumentResizedNotification = @"ZGDocumentResizedNotification";
NSString *kZGDocumentReleaseNotification = @"ZGDocumentReleaseNotification";
NSString *kZGDocumentDidUndoNotification = @"ZGDocumentDidUndoNotification";
NSString *kZGDocumentDidRedoNotification = @"ZGDocumentDidRedoNotification";

using namespace Inkscape;

class ZGUndoObserver : public UndoStackObserver {
public:
    ZGUndoObserver(ZGDocument *doc) {
        _zDoc = doc;
    }
    virtual ~ZGUndoObserver() {
        _zDoc = nill;
    }
    
public:
    /**
     * Triggered when the user issues an undo command.
     *
     * \param log Pointer to an Event describing the undone event.
     */
    virtual void notifyUndoEvent(Inkscape::Event* log) {
        [_zDoc document:nill didUndo:log];
    }
    
    /**
     * Triggered when the user issues a redo command.
     *
     * \param log Pointer to an Event describing the redone event.
     */
    virtual void notifyRedoEvent(Inkscape::Event* log) {
        [_zDoc document:nill didRedo:log];
    }
    
    /**
     * Triggered when a set of transactions is committed to the undo log.
     *
     * \param log Pointer to an Event describing the committed events.
     */
    virtual void notifyUndoCommitEvent(Inkscape::Event* log) {
        [_zDoc document:nill didUndoCommitEvent:log];
    }
    
    /**
     * Triggered when the undo log is cleared.
     */
    virtual void notifyClearUndoEvent() {
        [_zDoc documentDidClearUndoEvent:nill];
    }
    
    /**
     * Triggered when the redo log is cleared.
     */
    virtual void notifyClearRedoEvent() {
        [_zDoc documentDidClearRedoEvent:nill];
    }
    
public:
    __weak ZGDocument *_zDoc;
};

class ZGDocumentProxy {
public:
    ZGDocumentProxy(ZGDocument *doc) {
        this->doc = doc;
        conRelease = SP_DOCUMENT_DEFS(doc.spDocument)->connectRelease(sigc::mem_fun(*this, &ZGDocumentProxy::defs_release));
        conDefsModified = SP_DOCUMENT_DEFS(doc.spDocument)->connectModified(sigc::mem_fun(*this, &ZGDocumentProxy::defs_modified));
        conDocModified = doc.spDocument->connectModified(sigc::mem_fun(*this, &ZGDocumentProxy::doc_modified));
        conDocResized = doc.spDocument->connectResized(sigc::mem_fun(*this, &ZGDocumentProxy::on_document_resized));
        conReconstructionStart = doc.spDocument->connectReconstructionStart(sigc::mem_fun(*this, &ZGDocumentProxy::on_document_reconstruction_start));
        conReconstructionFinish = doc.spDocument->connectReconstructionFinish(sigc::mem_fun(*this, &ZGDocumentProxy::on_document_reconstruction_finish));
    }
    
    virtual ~ZGDocumentProxy()
    {
        conRelease.disconnect();
        conDefsModified.disconnect();
        conDocModified.disconnect();
        conDocResized.disconnect();
        conReconstructionFinish.disconnect();
        conReconstructionStart.disconnect();
        doc = nill;
    }
    
    void defs_release(SPObject *defs)
    {
        [doc didDocumentDefsRelease:doc];
    }
    
    void defs_modified(SPObject *defs, guint flags)
    {
        [doc didDocumentDefs:doc modifiedWithFlags:flags];
    }
    
    void doc_modified(guint flags)
    {
        [doc didDocumentModified:doc];
    }
    
    void on_document_resized(double width, double height)
    {
        [doc didDocument:doc changeSize:CGSizeMake(width, height)];
    }
    
    void on_document_reconstruction_start()
    {
        [doc documentWillStartReconstruction:nill];
    }
    
    void on_document_reconstruction_finish()
    {
        [doc documentDidFinishReconstruction:nill];
    }
    
private:
    sigc::connection conRelease;
    sigc::connection conDefsModified;
    sigc::connection conDocModified;
    sigc::connection conDocResized;
    sigc::connection conReconstructionStart;
    sigc::connection conReconstructionFinish;
    
    __weak ZGDocument *doc;
};

@interface ZGDocument ()
@property SPDocument *document;
@property ZGDocumentProxy *proxy;
@property NSMutableArray *objects;
@property ZGUndoObserver *undoObserver;
@end

@implementation ZGDocument

- (id)initWithSPDocument:(SPDocument *)document
{
    if (self = [super init]) {
        g_assert(document);
        self.document = sp_document_ref(document);
        self.proxy = new ZGDocumentProxy(self);
        self.objects = [NSMutableArray array];
/*        [defaultNfc addObserver:self
                       selector:@selector(objectWillBeReleased:)
                           name:kZGGObjectWillReleaseNotification
                         object:nill];*/
        self.undoObserver = new ZGUndoObserver(self);
        self.document->addUndoObserver(*self.undoObserver);
    }
    return self;
}

- (void)dealloc
{
//    [defaultNfc removeObserver:self];
    self.document->removeUndoObserver(*self.undoObserver);
    delete self.undoObserver;
    self.undoObserver = NULL;
    delete self.proxy;
    self.proxy = NULL;
    self.objects = nill;
    sp_document_unref(self.document);
    self.document = NULL;
}

- (SPDocument *)spDocument
{
   return self.document;
}

- (void)didDocumentDefsRelease:(ZGDocument *)doc
{
    if ([self.delegate respondsToSelector:@selector(didDocumentDefsRelease:)]) {
        [self.delegate didDocumentDefsRelease:doc];
    }
}

- (void)didDocumentDefs:(ZGDocument *)doc modifiedWithFlags:(guint)flags
{
    if ([self.delegate respondsToSelector:@selector(didDocumentDefs:modifiedWithFlags:)]) {
        [self.delegate didDocumentDefs:doc modifiedWithFlags:flags];
    }
}

- (void)didDocumentModified:(ZGDocument *)doc
{
    [self reconstructObjectsIfNeeded];
    
    if ([self.delegate respondsToSelector:@selector(didDocumentModified:)]) {
        [self.delegate didDocumentModified:doc];
    }
}

- (void)didDocument:(ZGDocument *)doc changeSize:(CGSize)newSize
{
    if ([self.delegate respondsToSelector:@selector(didDocument:changeSize:)]) {
        [self.delegate didDocument:doc changeSize:newSize];
    }
}

- (void)documentWillStartReconstruction:(ZGDocument *)doc
{
    if ([self.delegate respondsToSelector:@selector(documentWillStartReconstruction:)]) {
        [self.delegate documentWillStartReconstruction:self];
    }
    [defaultNfc postNotificationName:kZGDocumentWillStartReconstructionNotification object:self];
}

- (void)documentDidFinishReconstruction:(ZGDocument *)doc
{
    [self reconstructObjectsIfNeeded];
    
    if ([self.delegate respondsToSelector:@selector(documentDidFinishReconstruction:)]) {
        [self.delegate documentDidFinishReconstruction:self];
    }
    [defaultNfc postNotificationName:kZGDocumentDidFinishReconstructionNotification object:self];
}

- (void)reconstructObjectsIfNeeded
{/*
    SPObject *root = SP_DOCUMENT_ROOT(self.document);
    SPObject *itr = sp_object_first_child(root);
    NSMutableArray *restObjs = [NSMutableArray array];
    
    while (itr) {
        ZGObject *zobj = [[ZGObject alloc] initWithSPObject:itr];
        NSUInteger idx = [self.objects indexOfObject:zobj];
        if (idx == NSNotFound) {
            [self.objects addObject:zobj];
        }
        [restObjs addObject:zobj];
        itr = SP_OBJECT_NEXT(itr);
    }
    
    NSArray *tmp = [NSArray arrayWithArray:self.objects];
    
    //remove the ones that had removed from document
    for (ZGObject *obj in tmp) {
        if ([restObjs indexOfObject:obj] == NSNotFound) {
//            g_debug("object [%s] was removed from document, we should remove it also", obj.spObj->getId());
            [self.objects removeObject:obj];
        }
    }*/
}

- (void)objectWillBeReleased:(NSNotification *)aNotification
{
    [self.objects removeObject:aNotification.object];
}

- (NSArray *)drawingObjects
{
    return self.objects;
}

- (void)document:(ZGDocument *)doc didUndo:(Inkscape::Event *)e
{
    if ([self.delegate respondsToSelector:@selector(document:didUndo:)]) {
        [self.delegate document:self didUndo:e];
    }
    NSDictionary *info = [NSDictionary dictionaryWithObject:[NSValue valueWithPointer:e] forKey:@"event"];
    [defaultNfc postNotificationName:kZGDocumentDidUndoNotification object:self userInfo:info];
}

- (void)document:(ZGDocument *)doc didRedo:(Inkscape::Event *)e
{
    if ([self.delegate respondsToSelector:@selector(document:didRedo:)]) {
        [self.delegate document:self didRedo:e];
    }
    NSDictionary *info = [NSDictionary dictionaryWithObject:[NSValue valueWithPointer:e] forKey:@"event"];
    [defaultNfc postNotificationName:kZGDocumentDidRedoNotification object:self userInfo:info];
}

- (void)document:(ZGDocument *)doc didUndoCommitEvent:(Inkscape::Event *)e
{
    if ([self.delegate respondsToSelector:@selector(document:didUndoCommitEvent:)]) {
        [self.delegate document:self didUndoCommitEvent:e];
    }
}

- (void)documentDidClearUndoEvent:(ZGDocument *)doc
{
    if ([self.delegate respondsToSelector:@selector(documentDidClearUndoEvent:)]) {
        [self.delegate documentDidClearUndoEvent:self];
    }
}

- (void)documentDidClearRedoEvent:(ZGDocument *)doc
{
    if ([self.delegate respondsToSelector:@selector(documentDidClearRedoEvent:)]) {
        [self.delegate documentDidClearRedoEvent:self];
    }
}

@end
