//
//  BlockListView.m
//  HDRist
//
//  Created by 张光建 on 14-4-16.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "BlockListViewController.h"
#import "JSTexturalSkin.h"

@implementation FloatBlockView

- (id<JSMultiStateTexture>)texturalSkin
{
    return [[JSTexturalSkin alloc] initWithIdentifier:@"block" capInsets:JSEdgeInsetsDefault];
}

- (BOOL)isFlipped
{
    return NO;
}

@end


@interface BlockListViewController ()

@property NSMutableArray *blocks;

@end


@implementation FloatBlock
@end


@implementation BlockListViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        self.blocks = [NSMutableArray array];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    if (self = [super initWithCoder:aDecoder]) {
        self.blocks = [NSMutableArray array];
    }
    return self;
}

- (void)dealloc
{
    self.blocks = nil;
}

- (void)updateLayout
{
    //recalc space needs
    CGFloat heightNeeds = (self.blocks.count - 1) * self.inset;
    
    for (FloatBlock *b in self.blocks) {
        heightNeeds += b.view.frame.size.height;
    }
    
    [self.view setFrameSize:CGSizeMake(self.view.frame.size.width, heightNeeds)];
//    [self.view setFrameOrigin:CGPointMake(self.view.frame.origin.x, self.view.superview.bounds.size.height - heightNeeds)];
    
    int y = self.view.isFlipped ? 0 : self.view.frame.size.height;
    int dyScale = self.view.isFlipped ? 1 : -1;
    
    for (FloatBlock *b in self.blocks) {
        CGRect rect = b.view.frame;
        rect.origin.y = self.view.isFlipped ? y : y - b.view.frame.size.height;
        
        if (!b.view.superview) {
            rect.origin.x = 0;
            b.view.frame = rect;
            [self.view addSubview:b.view];
        } else {
            [[b.view animator] setFrame:rect];
        }
        
        y += dyScale * rect.size.height + self.inset;
    }
}


- (void)removeBlock:(FloatBlock *)block
{
    if ([self.blocks indexOfObject:block] != NSNotFound) {
        [block.view removeFromSuperview];
        [self.blocks removeObject:block];
        [self updateLayout];
    }
}

- (void)removeAllBlocks
{
    [self.view setSubviews:[NSArray array]];
    [self.blocks removeAllObjects];
    [self updateLayout];
}

- (void)addBlock:(FloatBlock *)block
{
    if ([self.blocks indexOfObject:block] != NSNotFound) {
        return;
    }
    
    for (NSUInteger i = 0; i < self.blocks.count; i++) {
        FloatBlock *b = [self.blocks objectAtIndex:i];
        
        if (block.priority >= b.priority) {
            [self.blocks insertObject:block atIndex:i];
            block = nil;
            break;
        }
    }
    
    if (block) {
        [self.blocks addObject:block];
    }
    
    [self updateLayout];
}

- (NSArray *)allBlocks
{
    return self.blocks;
}

@end
