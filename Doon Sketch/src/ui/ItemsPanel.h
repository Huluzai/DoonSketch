//
//  ItemsPanel.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-15.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <Foundation/Foundation.h>

struct SPDesktop;

@interface ItemsPanel : NSObject <NSOutlineViewDataSource, NSOutlineViewDelegate>

@property SPDesktop *desktop;

@property IBOutlet NSOutlineView *outlineView;

- (IBAction)deleteItem:(id)sender;

- (id)initWithOutlineView:(NSOutlineView *)anOutlineView;

@end
