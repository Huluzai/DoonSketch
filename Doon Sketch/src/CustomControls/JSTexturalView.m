//
//  JSTexturalView.m
//  PaintForMac
//
//  Created by Zhang Guangjian on 4/5/13.
//
//

#import "JSTexturalView.h"
#import "JSTexturalSkin.h"

@implementation JSTexturalView

@synthesize backgroundColor = _backgroundColor;

- (id)init
{
	self = [super init];
    if (self) {
        // Initialization code here.
		[self customInit];
    }
    
    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
		[self customInit];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
	self = [super initWithCoder:aDecoder];
    if (self) {
        // Initialization code here.
		[self customInit];
    }
    
    return self;
}

- (id)initWithIdentifier:(NSString *)identifier
{
	self = [super init];
	if (self) {
		self.identifier = identifier;
		[self customInit];
	}
	
	return self;
}

- (BOOL)customInit
{
	self.backgroundColor = nil;
	_texture = [self texturalSkin];
    
	return YES;
}

- (id<JSMultiStateTexture>)texturalSkin
{
	if (IS_STR_NIL([self identifier]))
		return nil;
	
	return [[JSTexturalSkin alloc] initWithIdentifier:[self identifier] capInsets:JSEdgeInsetsDefault];
}

- (void)dealloc
{
	self.backgroundColor = nil;
	
    if (_texture) {
        _texture = nil;
    }
}

- (BOOL)isFlipped
{
	return YES;
}

- (void)drawRect:(NSRect)dirtyRect
{
    // Drawing code here.
	
	[super drawRect:dirtyRect];
	
	// Fill background
	if (self.backgroundColor) {
		[self.backgroundColor setFill];
		NSRectFill([self bounds]);
	}
	
	// Draw texture
    if (_texture) {
        [_texture drawWithFrame:[self bounds] inView:self];
    }
}

@end
