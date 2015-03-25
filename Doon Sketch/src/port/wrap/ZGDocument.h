//
//  ZGDocument.h
//  Inxcape
//
//  Created by 张 光建 on 14-10-12.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGWrapper.h"

#import <Foundation/Foundation.h>
#import <glib.h>

extern NSString *kZGDocumentWillStartReconstructionNotification;
extern NSString *kZGDocumentDidFinishReconstructionNotification;
extern NSString *kZGDocumentDefsModifiedNotification;
extern NSString *kZGDocumentModifiedNotification;
extern NSString *kZGDocumentResizedNotification;
extern NSString *kZGDocumentReleaseNotification;
extern NSString *kZGDocumentDidUndoNotification;
extern NSString *kZGDocumentDidRedoNotification;

struct SPDocument;
namespace Inkscape {
    struct Event;
}

@interface ZGDocument : ZGWrapper

// the SPDocument is retained
- (id)initWithSPDocument:(SPDocument *)document;

- (SPDocument *)spDocument;

- (NSArray *)drawingObjects;

//Delegate methods

- (void)documentWillStartReconstruction:(ZGDocument *)doc;
- (void)documentDidFinishReconstruction:(ZGDocument *)doc;
- (void)didDocumentDefsRelease:(ZGDocument *)doc;
- (void)didDocumentDefs:(ZGDocument *)doc modifiedWithFlags:(guint)flags;
- (void)didDocumentModified:(ZGDocument *)doc;
- (void)didDocument:(ZGDocument *)doc changeSize:(CGSize)newSize;

//undo
- (void)document:(ZGDocument *)doc didUndo:(Inkscape::Event *)e;
- (void)document:(ZGDocument *)doc didRedo:(Inkscape::Event *)e;
- (void)document:(ZGDocument *)doc didUndoCommitEvent:(Inkscape::Event *)e;
- (void)documentDidClearUndoEvent:(ZGDocument *)doc;
- (void)documentDidClearRedoEvent:(ZGDocument *)doc;

@end
