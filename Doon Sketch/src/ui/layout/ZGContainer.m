//
//  ZGContainer.m
//  Inxcape
//
//  Created by 张 光建 on 14-9-20.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGContainer.h"

@interface ZGContainer ()
@property (weak) NSView *view;
@end

@implementation ZGContainer

- (id)initWithView:(NSView *)anView
{
    if (self = [super init]) {
        self.view = anView;
        [self.view setPostsBoundsChangedNotifications:YES];
        [self.view setPostsFrameChangedNotifications:YES];
        
        [defaultNfc addObserver:self
                       selector:@selector(didBoundsChanged:)
                           name:NSViewBoundsDidChangeNotification
                         object:self.view];
        
        [defaultNfc addObserver:self
                       selector:@selector(didBoundsChanged:)
                           name:NSViewFrameDidChangeNotification
                         object:self.view];
    }
    return self;
}

- (void)dealloc
{
    [defaultNfc removeObserver:self];
    self.view = nill;
}

- (void)add:(NSView *)anView
{
    [self.view addSubview:anView];
}

- (void)remove:(NSView *)anView
{
    [anView removeFromSuperview];
}

- (void)didBoundsChanged:(NSNotification*)nf
{
    [self relayout];
}

- (NSView *)targetView
{
    return self.view;
}

@end
