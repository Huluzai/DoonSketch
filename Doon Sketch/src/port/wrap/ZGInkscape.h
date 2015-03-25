//
//  ZGInkscape.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/2.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGObject.h"

#import <inkscape.h>
#import <selection.h>

@interface ZGInkscape : ZGObject

// no ref
- (id)initWithInkscape:(Inkscape::Application *)inkscape;
- (Inkscape::Application *)inkscape;

@end


@interface ZGInkscape (Delegate)

- (void) didInkscape:(ZGInkscape *)inkscape modifySelection:(Inkscape::Selection *)selection withFlags:(guint)flags;
- (void) didInkscape:(ZGInkscape *)inkscape changeSelection:(Inkscape::Selection *)selection;
- (void) didInkscape:(ZGInkscape *)inkscape setSelection:(Inkscape::Selection *)selection;
- (void) didInkscape:(ZGInkscape *)inkscape changeSubselection:(Inkscape::Selection *)selection;

- (void) didInkscape:(ZGInkscape *)inkscape activeDesktop:(SPDesktop *)desktop;
- (void) didInkscape:(ZGInkscape *)inkscape deactiveDesktop:(SPDesktop *)desktop;

@end