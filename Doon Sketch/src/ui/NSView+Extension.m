//
//  NSView+Extends.m
//  Inxcape
//
//  Created by 张 光建 on 14/12/7.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "NSView+Extension.h"

@implementation NSView(Extension)

- (void)setSensitive:(BOOL)sensitive
{
    for (NSView *sub in self.subviews) {
        if ([sub isKindOfClass:[NSControl class]]) {
            [(NSControl *)sub setEnabled:sensitive];
        } else if ([sub respondsToSelector:@selector(setSensitive:)]) {
            [sub setSensitive:sensitive];
        }
    }
}

@end
