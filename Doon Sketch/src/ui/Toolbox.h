//
//  Toolbox.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/2.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <preferences.h>
#import <glib.h>
#import <gobject/gobject.h>

#define CAN_ENTER_ONCE_CODE (!_freeze)
#define ENTER_ONCE_CODE {if (_freeze) {return;} _freeze = TRUE;}
#define LEAVE_ONCE_CODE {_freeze = FALSE;}
#define GlbPrefs    (Inkscape::Preferences::get())

struct SPDesktop;

@interface Toolbox : NSViewController {
    BOOL _freeze;
}
- (id)initWithSPDesktop:(SPDesktop *)desktop;
- (SPDesktop *)desktop;
- (gpointer)gpointer;
+ (id)toolboxFromGPointer:(gpointer)pointer;
- (void)didFreehandModeChanged:(int)mode;
@end

@interface Toolbox (Override)
- (NSString *)toolboxNibName;
- (void)didSelectionChanged:(NSNotification *)aNotification;
- (void)didSelectionModified:(NSNotification *)aNotificaiton;
- (void)didSubselectionChanged:(NSNotification *)aNotification;
@end


void purge_repr_listener( GObject* /*obj*/, gpointer tbl );

