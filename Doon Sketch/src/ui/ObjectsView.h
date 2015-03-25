//
//  ObjectsListView.h
//  Inxcape
//
//  Created by 张 光建 on 15/1/4.
//  Copyright (c) 2015年 Doonsoft. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface ObjectsView : NSOutlineView

@end

@interface ObjectsView (Delegate)

- (void)objectsView:(ObjectsView*)anObjectsView didClickTableRow:(NSInteger)row;

@end