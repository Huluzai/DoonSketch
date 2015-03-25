//
//  ZGContainer.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-20.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface ZGContainer : NSObject

@property id layout;

- (id)initWithView:(NSView *)anView;

- (void)add:(NSView *)anView;

- (void)remove:(NSView *)anView;

- (NSView *)targetView;

@end


@interface ZGContainer (Virtual)

- (void)relayout;

@end