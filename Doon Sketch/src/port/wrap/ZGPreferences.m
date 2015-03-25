//
//  ZGPreferences.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/3.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGPreferences.h"

NSString *kZGPreferencesDidChangedValueNotification = @"ZGPreferencesDidChangedValueNotification";

/**
 * A simple mediator class that keeps UI controls matched to the preference values they set.
 */
class ZGPreferencesProxy : public Inkscape::Preferences::Observer
{
public:
    /**
     * Constructor for a boolean value that syncs to the supplied path.
     * Initializes the widget to the current preference stored state and registers callbacks
     * for widget changes and preference changes.
     *
     * @param act the widget to synchronize preference with.
     * @param path the path to the preference the widget is synchronized with.
     * @param callback function to invoke when changes are pushed.
     * @param cbData data to be passed on to the callback function.
     */
    ZGPreferencesProxy(ZGPreferencesNode *owner, Glib::ustring const &path) : Observer(path) {
        _zPref = owner;
        Inkscape::Preferences::get()->addObserver(*this);
    }
    
    /**
     * Destructor that unregisters the preference callback.
     */
    virtual ~ZGPreferencesProxy() {
        Inkscape::Preferences::get()->removeObserver(*this);
        _zPref = nill;
    }
    
    /**
     * Callback method invoked when the preference setting changes.
     */
    virtual void notify(Inkscape::Preferences::Entry const &new_val) {
        [_zPref preferences:nill didChangedValue:(Inkscape::Preferences::Entry)new_val];
    }
    
private:
    __weak ZGPreferencesNode *_zPref;
};

@interface ZGPreferencesNode ()
@property ZGPreferencesProxy *proxy;
@end

@implementation ZGPreferencesNode

- (id)initWithNodePath:(NSString *)path
{
    if (self = [super init]) {
        self.proxy = new ZGPreferencesProxy(self, [path UTF8String]);
    }
    return self;
}

- (void)dealloc
{
    delete self.proxy;
}

- (void)preferences:(ZGPreferencesNode *)pref didChangedValue:(Inkscape::Preferences::Entry)entry
{
    if ([self.delegate respondsToSelector:@selector(preferences:didChangedValue:)]) {
        [self.delegate preferences:self didChangedValue:entry];
    }
    NSDictionary *usrInfo = [NSDictionary dictionaryWithObject:[NSString stringWithUTF8String:entry.getPath().c_str()] forKey:@"path"];
    [defaultNfc postNotificationName:kZGPreferencesDidChangedValueNotification object:self userInfo:usrInfo];
}

- (Inkscape::Preferences *)preferences
{
    return Inkscape::Preferences::get();
}

@end
