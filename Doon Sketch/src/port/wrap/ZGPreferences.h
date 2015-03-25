//
//  ZGPreferences.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/3.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGWrapper.h"
#import <preferences.h>

extern NSString *kZGPreferencesDidChangedValueNotification;

@interface ZGPreferencesNode : ZGWrapper

- (id)initWithNodePath:(NSString *)path;
- (Inkscape::Preferences *)preferences;

@end


@interface ZGPreferencesNode (Delegate)
- (void)preferences:(ZGPreferencesNode *)pref didChangedValue:(Inkscape::Preferences::Entry)entry;
@end
