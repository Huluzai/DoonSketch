//
//  PageSettings.h
//  Inxcape
//
//  Created by 张 光建 on 14/12/25.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface PageSettings : NSWindowController

@property CGFloat width;
@property CGFloat height;

@property(weak) id delegate;

- (IBAction)didClickConfirm:(id)sender;
- (IBAction)didClickCancel:(id)sender;

@end


@interface PageSettings (Delegate)
- (void)didPageSettings:(PageSettings *)settings finishWithCode:(NSInteger)code;
@end
