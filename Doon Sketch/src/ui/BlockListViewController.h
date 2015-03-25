//
//  BlockListView.h
//  HDRist
//
//  Created by 张光建 on 14-4-16.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "JSTexturalView.h"


@interface FloatBlockView : JSTexturalView
@end


@interface FloatBlock : NSObject

@property IBOutlet NSView *view;
@property int priority;
@property NSString *name;

@end


@interface BlockListViewController : NSViewController

@property CGFloat inset;

- (void)removeBlock:(FloatBlock *)block;

- (void)removeAllBlocks;

- (void)addBlock:(FloatBlock *)block;

- (NSArray *)allBlocks;

- (void)updateLayout;

@end
