//
//  PageSettings.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/25.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "PageSettings.h"

@interface PageSettings ()

@end

@implementation PageSettings

- (void)windowDidLoad {
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (IBAction)didClickConfirm:(id)sender
{
    [self.delegate didPageSettings:self finishWithCode:NSModalResponseOK];
}

- (IBAction)didClickCancel:(id)sender
{
    [self.delegate didPageSettings:self finishWithCode:NSModalResponseCancel];
}

@end
