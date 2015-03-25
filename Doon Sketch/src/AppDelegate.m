//
//  AppDelegate.m
//  Inxcape
//
//  Created by 张 光建 on 14/10/24.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "AppDelegate.h"
#import "IDDocumentController.h"

#include <libnrtype/FontFactory.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/font-lister.h>
#import <inkscape.h>

// User defaults keys
NSString *const ZGFirstRunKey = @"ZGFirstRunKey";

static void thread_test()
{
    g_debug("5 processes num : %d", g_get_num_processors());
}


static void a_shared_func()
{
    static bool busy = false;
    g_assert(!busy);
    g_debug("enter shared func ..");
    busy = true;
    sleep(10);
    busy = false;
    g_debug("leave shared func ..");
}

static gboolean idle_handler(gpointer data)
{
    a_shared_func();
    return TRUE;
}

static gboolean idle_handler2(gpointer data)
{
    a_shared_func();
    return TRUE;
}

static gboolean idle_handler3(gpointer data)
{
    a_shared_func();
    return TRUE;
}

static void idle_test()
{
    g_idle_add(idle_handler, 0);
    g_idle_add(idle_handler2, 0);
    g_idle_add(idle_handler3, 0);
    for (int i = 0; i < 0; i++) {
        a_shared_func();
    }
}

static Boolean idle_func(gpointer data)
{
    //    g_message("\n[%d]idle_func running ... ", (*(guint32*)data)++);
    g_print("-");
    return TRUE;
}

static Boolean timer_func(gpointer data)
{
    g_message("[%d]timer called ... ", (*(guint32*)data)++);
    return TRUE;
}

static void gidle_test1()
{
    g_message("gidle test1 start .\n");
    guint32 counter = 0;
    g_idle_add((GSourceFunc)idle_func, &counter);
    g_timeout_add(3, (GSourceFunc)timer_func, &counter);
    
    while (1) {
        sleep(1);
    }
}

static void gidle_test4()
{
    g_message("gidle test4 start .\n");
    guint32 counter = 0;
    g_idle_add((GSourceFunc)idle_func, &counter);
    g_timeout_add(3, (GSourceFunc)timer_func, &counter);
}

@implementation AppDelegate
#if 0
+ (void)initialize
{
    Inkscape::FontLister::get_instance()->get_font_list();
    
    [IDDocumentController sharedDocumentController];
    
    NSDictionary *defs = [NSDictionary dictionaryWithObjectsAndKeys:
                          @TRUE, ZGFirstRunKey,
                          NULL];
    
    [standardDefaults registerDefaults:defs];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    if ([standardDefaults boolForKey:ZGFirstRunKey]) {
        [self prepareRescourcesFiles];
        [standardDefaults setBool:FALSE forKey:ZGFirstRunKey];
    }
//    gidle_test4();
//    thread_test();
//    idle_test();
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void)prepareRescourcesFiles
{
/*    NSURL *pref = [[NSBundle mainBundle] URLForResource:@"preferences" withExtension:@"xml"];
    NSURL *profile = [NSURL URLWithString:[NSString stringWithUTF8String:profile_path("preferences.xml")]];
    NSError *error = nill;
    [[NSFileManager defaultManager] copyItemAtURL:pref toURL:profile error:&error];
    if (error) {
        g_warning("Copy preferences.xml failed : %s", [[error localizedDescription] UTF8String]);
    }*/
}
#endif
@end


@implementation MyApplication
- (void)sendEvent:(NSEvent *)anEvent
{
    CGEventGetType([anEvent CGEvent]);
    [super sendEvent:anEvent];
}
@end

