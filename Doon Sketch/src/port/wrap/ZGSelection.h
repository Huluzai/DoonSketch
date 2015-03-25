//
//  ZGSelection.h
//  Inxcape
//
//  Created by 张 光建 on 14/11/25.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGObject.h"

extern NSString *kZGSelectionDidModifiedNotification;
extern NSString *kZGSelectionDidChangedNotification;

namespace Inkscape {
    class Selection;
};

class SPDesktop;

@interface ZGSelection : ZGObject

- (id)initWithSPDesktop:(SPDesktop *)aDesktop;
- (Inkscape::Selection *)selection;
- (NSArray *)selectedItems;

@end


@interface ZGSelection (Delegate)

- (void)didSelectionModified:(ZGSelection *)selection flags:(guint)flags;
- (void)didSelectionChanged:(ZGSelection *)selection;
- (void)didSubselectionChanged:(ZGSelection *)selection;

@end
