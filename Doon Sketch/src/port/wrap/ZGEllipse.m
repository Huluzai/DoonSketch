//
//  ZGEllipse.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/26.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGEllipse.h"
#import "ZGObject-Private.h"

#import <sp-ellipse.h>
#import <attributes.h>

@implementation ZGEllipse

DEFINE_ATTRIBUTE_METHODS

+ (void)initialize
{
    [self registerAttribute:SP_ATTR_CX];
    [self registerAttribute:SP_ATTR_CY];
    [self registerAttribute:SP_ATTR_RX];
    [self registerAttribute:SP_ATTR_RY];
    g_debug("ZGEllipse attributes:\n%s", self.allAttributes.description.UTF8String);
}

- (id)initWithSPEllipse:(SPGenericEllipse *)ellipse
{
    if (self = [super initWithSPObject:SP_OBJECT(ellipse) takeCopy:FALSE]) {
        [self addObserver:self forKeyPath:@"cx" options:NSKeyValueObservingOptionNew context:nill];
        [self addObserver:self forKeyPath:@"cy" options:NSKeyValueObservingOptionNew context:nill];
        [self addObserver:self forKeyPath:@"rx" options:NSKeyValueObservingOptionNew context:nill];
        [self addObserver:self forKeyPath:@"ry" options:NSKeyValueObservingOptionNew context:nill];
    }
    return self;
}

- (void)dealloc
{
    [self removeObserver:self forKeyPath:@"cx"];
    [self removeObserver:self forKeyPath:@"cy"];
    [self removeObserver:self forKeyPath:@"rx"];
    [self removeObserver:self forKeyPath:@"ry"];
}

- (SPGenericEllipse *)spEllipse
{
    return SP_GENERICELLIPSE([super spObj]);
}

- (void)didObjectModified:(ZGObject *)sender
{
    [super didObjectModified:sender];
    
    if (self.cx != self.spEllipse->cx.computed) self.cx = self.spEllipse->cx.computed;
    if (self.cy != self.spEllipse->cy.computed) self.cy = self.spEllipse->cy.computed;
    if (self.rx != self.spEllipse->rx.computed) self.rx = self.spEllipse->rx.computed;
    if (self.ry != self.spEllipse->ry.computed) self.ry = self.spEllipse->ry.computed;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
#define CX()    ((self.spEllipse)->cx.computed)
#define CY()    ((self.spEllipse)->cy.computed)
#define RX()    ((self.spEllipse)->rx.computed)
#define RY()    ((self.spEllipse)->ry.computed)
    
    if ([keyPath isEqual:@"cx"]) {
        if (SP_IS_ELLIPSE(self.spEllipse)) {
            sp_ellipse_position_set(SP_ELLIPSE(self.spEllipse), self.cx, CY(), RX(), RY());
        } else if (SP_IS_ARC(self.spEllipse)) {
            sp_arc_position_set(SP_ARC(self.spEllipse), self.cx, CY(), RX(), RY());
        }
    } else if ([keyPath isEqual:@"cy"]) {
        if (SP_IS_ELLIPSE(self.spEllipse)) {
            sp_ellipse_position_set(SP_ELLIPSE(self.spEllipse), CX(), self.cy, RX(), RY());
        } else if (SP_IS_ARC(self.spEllipse)) {
            sp_arc_position_set(SP_ARC(self.spEllipse), CX(), self.cy, RX(), RY());
        }
    } else if ([keyPath isEqual:@"rx"]) {
        if (SP_IS_ELLIPSE(self.spEllipse)) {
            sp_ellipse_position_set(SP_ELLIPSE(self.spEllipse), CX(), CY(), self.rx, RY());
        } else if (SP_IS_ARC(self.spEllipse)) {
            sp_arc_position_set(SP_ARC(self.spEllipse), CX(), CY(), self.rx, RY());
        }
    } else if ([keyPath isEqual:@"ry"]) {
        if (SP_IS_ELLIPSE(self.spEllipse)) {
            sp_ellipse_position_set(SP_ELLIPSE(self.spEllipse), CX(), CY(), RX(), self.ry);
        } else if (SP_IS_ARC(self.spEllipse)) {
            sp_arc_position_set(SP_ARC(self.spEllipse), CX(), CY(), RX(), self.ry);
        }
    }
}

@end
