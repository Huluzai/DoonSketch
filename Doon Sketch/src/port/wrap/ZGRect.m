//
//  ZGRect.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/26.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGRect.h"
#import "ZGObject-Private.h"

#import <sp-rect.h>
#import <attributes.h>
#import <2geom/geom.h>

#define X()    ((self.spRect)->x.computed)
#define Y()    ((self.spRect)->y.computed)
#define W()    ((self.spRect)->width.computed)
#define H()    ((self.spRect)->height.computed)


@implementation ZGRect

DEFINE_ATTRIBUTE_METHODS

+ (void)initialize
{
    [self registerAttribute:SP_ATTR_X];
    [self registerAttribute:SP_ATTR_Y];
    [self registerAttribute:SP_ATTR_RX];
    [self registerAttribute:SP_ATTR_RY];
    [self registerAttribute:SP_ATTR_WIDTH];
    [self registerAttribute:SP_ATTR_HEIGHT];
    g_debug("ZGRect attributes:\n%s", self.allAttributes.description.UTF8String);
}

- (id)initWithSPRect:(SPRect *)rect
{
    if (self = [super initWithSPObject:SP_OBJECT(rect) takeCopy:FALSE]) {
        [self addObserver:self forKeyPath:@"x" options:NSKeyValueObservingOptionNew context:nill];
        [self addObserver:self forKeyPath:@"width" options:NSKeyValueObservingOptionNew context:nill];
        [self addObserver:self forKeyPath:@"height" options:NSKeyValueObservingOptionNew context:nill];
        [self addObserver:self forKeyPath:@"y" options:NSKeyValueObservingOptionNew context:nill];
        [self addObserver:self forKeyPath:@"rx" options:NSKeyValueObservingOptionNew context:nill];
        [self addObserver:self forKeyPath:@"ry" options:NSKeyValueObservingOptionNew context:nill];
    }
    return self;
}

- (void)dealloc
{
    [self removeObserver:self forKeyPath:@"x"];
    [self removeObserver:self forKeyPath:@"y"];
    [self removeObserver:self forKeyPath:@"width"];
    [self removeObserver:self forKeyPath:@"height"];
    [self removeObserver:self forKeyPath:@"rx"];
    [self removeObserver:self forKeyPath:@"ry"];
}

- (SPRect *)spRect
{
    return SP_RECT([super spObj]);
}

- (void)didObjectModified:(ZGObject *)sender
{
    [super didObjectModified:sender];
    
    if (self.x != self.spRect->x.computed) self.x = self.spRect->x.computed;
    if (self.y != self.spRect->y.computed) self.y = self.spRect->y.computed;
    if (self.width != self.spRect->width.computed) self.width = self.spRect->width.computed;
    if (self.height != self.spRect->height.computed) self.height = self.spRect->height.computed;
    if (self.rx != self.spRect->rx.computed) self.rx = self.spRect->rx.computed;
    if (self.ry != self.spRect->ry.computed) self.ry = self.spRect->ry.computed;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqual:@"x"]) {
        sp_rect_position_set(self.spRect, self.x, Y(), W(), H());
    } else if ([keyPath isEqual:@"y"]) {
        sp_rect_position_set(self.spRect, X(), self.y, W(), H());
    } else if ([keyPath isEqual:@"width"]) {
        sp_rect_position_set(self.spRect, X(), Y(), self.width, H());
    } else if ([keyPath isEqual:@"height"]) {
        sp_rect_position_set(self.spRect, X(), Y(), W(), self.height);
    } else if ([keyPath isEqual:@"rx"]) {
        sp_rect_set_rx(self.spRect, true, self.rx);
    } else if ([keyPath isEqual:@"ry"]) {
        sp_rect_set_ry(self.spRect, true, self.ry);
    }
}

@end
