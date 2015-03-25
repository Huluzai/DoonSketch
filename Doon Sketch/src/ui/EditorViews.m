//
//  EditorViews.m
//  PencilSketch
//
//  Created by 张光建 on 14-4-13.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "EditorViews.h"
#import "JSTexturalSkin.h"

@implementation ImageToolbarView

- (id<JSMultiStateTexture>)texturalSkin
{
	return [[JSTexturalSkin alloc] initWithIdentifier:@"WWChatViewHeaderBg" capInsets:JSEdgeInsetsDefault];
}

@end


@implementation ImageFootbarView

- (id<JSMultiStateTexture>)texturalSkin
{
	return [[JSTexturalSkin alloc] initWithIdentifier:@"WWChatViewInputAreaBg" capInsets:JSEdgeInsetsDefault];
}

@end


@implementation OptionAreaBackground

- (NSColor *)backgroundColor
{
    return [NSColor colorWithCalibratedRed:160. / 255 green:200. / 255. blue:220. / 255 alpha:1];
}

@end


@implementation BlockView

- (id<JSMultiStateTexture>)texturalSkin
{
	return [[JSTexturalSkin alloc] initWithIdentifier:@"page_backgound" capInsets:JSEdgeInsetsDefault];
}

- (BOOL)isFlipped
{
    return NO;
}

@end

@implementation BlockHeaderView

- (id<JSMultiStateTexture>)texturalSkin
{
	return [[JSTexturalSkin alloc] initWithIdentifier:@"header" capInsets:JSEdgeInsetsDefault];
}

@end

@implementation BackgroundView

- (id<JSMultiStateTexture>)texturalSkin
{
	return [[JSTexturalSkin alloc] initWithIdentifier:@"background" capInsets:JSEdgeInsetsDefault];
}

@end

@implementation ToolbarView

- (id<JSMultiStateTexture>)texturalSkin
{
	return [[JSTexturalSkin alloc] initWithIdentifier:@"toolbar" capInsets:JSEdgeInsetsDefault];
}

@end

@implementation ParametersView

- (id<JSMultiStateTexture>)texturalSkin
{
	return [[JSTexturalSkin alloc] initWithIdentifier:@"parameters" capInsets:JSEdgeInsetsDefault];
}

@end

@implementation GreenTileView

- (id<JSMultiStateTexture>)texturalSkin
{
	return [[JSTexturalSkin alloc] initWithIdentifier:@"buttons" capInsets:JSEdgeInsetsDefault];
}

@end

@implementation DarkTileView

- (id<JSMultiStateTexture>)texturalSkin
{
	return [[JSTexturalSkin alloc] initWithIdentifier:@"darktile" capInsets:JSEdgeInsetsDefault];
}

@end
