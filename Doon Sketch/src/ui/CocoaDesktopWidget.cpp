//
//  CocoaDesktopManager.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "CocoaDesktopWidget.h"
#import <desktop.h>
#import <sigc++/sigc++.h>
#import <ui/view/edit-widget-interface.h>
#import <gtk/gtk.h>
#import <Cocoa/Cocoa.h>
#import <sp-namedview.h>
#import "sp-canvas-cocoa.h"
#import "CocoaCanvasView.h"
#import <document.h>
#import "application/editor.h"
#import <verbs.h>
#import <helper/action.h>
#import <inkscape-private.h>
//#import "IPDocument.h"

CocoaEditWidget::CocoaEditWidget()
: _delegate(NULL)
{
}

CocoaEditWidget::CocoaEditWidget(id<CocoaEditWidgetDelegate> delegate)
{
    setDelegate(delegate);
}

CocoaEditWidget::~CocoaEditWidget()
{
    _delegate = NULL;
}

void CocoaEditWidget::getGeometry (gint &x, gint &y, gint &w, gint &h)
{
    CGRect frame = [_delegate.window frame];
    x = frame.origin.x;
    y = frame.origin.y;
    w = frame.size.width;
    h = frame.size.height;
}

void CocoaEditWidget::setSize (gint w, gint h)
{
    CGRect frame = [_delegate.window frame];
    frame.size = CGSizeMake(w, h);
    [_delegate.window setFrame:frame display:YES animate:YES];
}

void CocoaEditWidget::setPosition (Geom::Point p)
{
    CGRect frame = [_delegate.window frame];
    frame.origin = CGPointMake(p[Geom::X], p[Geom::Y]);
    [_delegate.window setFrame:frame display:YES animate:YES];
}

void CocoaEditWidget::setCoordinateStatus (Geom::Point p)
{
    [_delegate updateCoordinateStatus:CGPointMake(p[Geom::X], p[Geom::Y])];
}

void CocoaEditWidget::toggleRulers()
{
    [_delegate toggleRulers];
}

void CocoaEditWidget::updateRulers()
{
    [_delegate updateRulers];
}

Geom::Point CocoaEditWidget::getPointer()
{
    CGPoint pos = [_delegate.canvasView convertPoint:[_delegate.window mouseLocationOutsideOfEventStream]
                                            fromView:nill];
    return Geom::Point(pos.x, pos.y);
}

void CocoaEditWidget::requestCanvasUpdate()
{
    [_delegate requestCanvasUpdate];
}

void CocoaEditWidget::requestCanvasUpdateAndWait()
{
    requestCanvasUpdate();
}

bool CocoaEditWidget::warnDialog (gchar* text)
{
    [NSAlert alertWithMessageText:[NSString stringWithUTF8String:text]
                    defaultButton:@"OK"
                  alternateButton:NULL
                      otherButton:NULL
        informativeTextWithFormat:NULL];
    return TRUE;
}

void CocoaEditWidget::setMessage (Inkscape::MessageType type, gchar const* msg)
{
    if (msg) {
//        g_message(msg);
        [_delegate showMessage:[NSString stringWithUTF8String:msg]];
    }
}

void CocoaEditWidget::setCursor(SPCursorType type) {
    NSCursor *cursor = NULL;
    switch (type) {
        case SP_TEXT_CURSOR:
        case SP_TEXT_INSERT_CURSOR:
            cursor = [NSCursor IBeamCursor];
            break;
        
        default:
            cursor = [NSCursor arrowCursor];
            break;
    }
    
    [cursor set];
}
