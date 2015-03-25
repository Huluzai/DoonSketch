//
//  JSImagesSkin.m
//  PaintForMac
//
//  Created by Zhang Guangjian on 4/2/13.
//
//

#import "JSTexturalSkin.h"
#import "JSImage.h"

#define kJSSkinStateNormal      (@"_n")
#define kJSSkinStateHighlighted	(@"_h")
#define kJSSkinStateSelected	(@"_s")
#define kJSSkinStateDisabled	(@"_d")
#define kJSSkinStatePressed		(@"_p")

@implementation JSTexturalSkin

@synthesize stateImages = _stateImages;
@synthesize currentImage = _currentImage;
@synthesize capInsets = _capInsets;

- (id)initWithIdentifier:(NSString *)identifier
{
	self = [super init];
	if (self) {
		_capInsets = JSEdgeInsetsZero;
		self.stateImages = [self loadImages:identifier];
		[self changeState:JSSkinStateNormal];
	}
	return self;
}

- (id)initWithIdentifier:(NSString *)identifier
			   capInsets:(JSEdgeInsets)capInsets
{
	self = [super init];
	if (self) {
		_capInsets = capInsets;
		self.stateImages = [self loadImages:identifier];
		[self changeState:JSSkinStateNormal];
	}
	return self;
}

- (void)dealloc
{
	self.currentImage = nil;
	self.stateImages = nil;
}

- (void)changeState:(NSUInteger)state
{
	if (_oldState == state && self.currentImage)
		return;
#if 0
	JSImage *normalImage = [self.stateImages objectForKey:kJSSkinStateNormal];
	
	if (normalImage) {
		NSRect srcRect = NSMakeRect(0, 0, normalImage.size.width, normalImage.size.height);
		
		self.currentImage = [[[JSImage alloc] initWithSize:[normalImage size]] autorelease];
		[self.currentImage lockFocus];
		
		[[NSColor clearColor] setFill];
		[[NSBezierPath bezierPathWithRect:srcRect] fill];
		
		// Below the sequence and the composite method is important
		
		if (state & JSSkinStateNormal) {
			[normalImage drawAtPoint:NSZeroPoint
							fromRect:srcRect
						   operation:NSCompositeSourceOver
							fraction:1.0];
		}
        
		if (state & JSSkinStateSelected) {
			JSImage *selectedImage = [self.stateImages objectForKey:kJSSkinStateSelected];
			[selectedImage drawAtPoint:NSZeroPoint
							  fromRect:srcRect
							 operation:NSCompositeSourceOver
							  fraction:1.0];
		}
		
		if (state & JSSkinStateHighlighted) {
			JSImage *highlightedImage = [self.stateImages objectForKey:kJSSkinStateHighlighted];
			[highlightedImage drawAtPoint:NSZeroPoint
								 fromRect:srcRect
								operation:NSCompositeSourceOver
								 fraction:1.0];
		}
		
		if (state & JSSkinStatePressed) {
			JSImage *pressedImage = [self.stateImages objectForKey:kJSSkinStatePressed];
			[pressedImage drawAtPoint:NSZeroPoint
                             fromRect:srcRect
                            operation:NSCompositeSourceOver
                             fraction:1.0];
		}
		
		if (state & JSSkinStateDisabled) {
			JSImage *disabledImage = [self.stateImages objectForKey:kJSSkinStateDisabled];
			[disabledImage drawAtPoint:NSZeroPoint
							  fromRect:srcRect
							 operation:NSCompositeSourceOver
							  fraction:1.0];
		}
		
		[self.currentImage unlockFocus];
		
		if (!JSEqualInsets(_capInsets, JSEdgeInsetsZero)) {
			JSEdgeInsets cap = _capInsets;
			if (JSEqualInsets(JSEdgeInsetsDefault, cap)) {
				cap = JSEdgeInsetsMake(self.currentImage.size.height / 2,
									   self.currentImage.size.width / 2,
									   self.currentImage.size.height / 2 + 1,
									   self.currentImage.size.width / 2 + 1);
			}
			self.currentImage = [self.currentImage resizableImageWithCapInsets:cap];
		}
	}
#endif
	_oldState = state;
}

- (void)drawWithFrame:(NSRect)frame inView:(NSView *)view
{/*
  [self.currentImage drawInRect:frame
  operation:NSCompositeSourceOver
  fraction:1.0
  isFlipped:[view isFlipped]];*/
    
    NSUInteger state = _oldState;
    JSImage *normalImage = [self.stateImages objectForKey:kJSSkinStateNormal];
	
	if (normalImage) {
		NSRect srcRect = NSMakeRect(0, 0, normalImage.size.width, normalImage.size.height);
		
		[[NSColor clearColor] setFill];
		[[NSBezierPath bezierPathWithRect:srcRect] fill];
		
		// Below the sequence and the composite method is important
		
		if (state & JSSkinStateNormal) {
            [normalImage drawInRect:frame
                          operation:NSCompositeSourceOver
                           fraction:1.0
                          isFlipped:[view isFlipped]];
		}
        
		if (state & JSSkinStateSelected) {
			JSImage *selectedImage = [self.stateImages objectForKey:kJSSkinStateSelected];
            [selectedImage drawInRect:frame
                            operation:NSCompositeSourceOver
                             fraction:1.0
                            isFlipped:[view isFlipped]];
		}
		
		if (state & JSSkinStateHighlighted) {
			JSImage *highlightedImage = [self.stateImages objectForKey:kJSSkinStateHighlighted];
            [highlightedImage drawInRect:frame
                               operation:NSCompositeSourceOver
                                fraction:1.0
                               isFlipped:[view isFlipped]];
		}
		
		if (state & JSSkinStatePressed) {
			JSImage *pressedImage = [self.stateImages objectForKey:kJSSkinStatePressed];
            [pressedImage drawInRect:frame
                           operation:NSCompositeSourceOver
                            fraction:1.0
                           isFlipped:[view isFlipped]];
		}
		
		if (state & JSSkinStateDisabled) {
			JSImage *disabledImage = [self.stateImages objectForKey:kJSSkinStateDisabled];
            [disabledImage drawInRect:frame
                            operation:NSCompositeSourceOver
                             fraction:1.0
                            isFlipped:[view isFlipped]];
		}
	}
}

- (NSDictionary *)loadImages:(NSString *)identifier
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    
    if (IS_STR_NIL(identifier)) {
        NSLog(@"*** Error, identifier is nil");
        return dict;
    }
    
	const NSArray *ext = [self skinStateArray];
	
    NSArray *allExtentions = [ext arrayByAddingObject:@""];
    
	for (int i = 0; i < [allExtentions count]; i++) {
		NSString *title = nil;
		JSImage *image = nil;
		NSString *ident = [NSString stringWithString:identifier];
        NSString *e = [allExtentions objectAtIndex:i];
        
		// ie. identifier = test_button_shape,
		// first search for images named test_button_shape_*,
		// then button_shape_*,
		// last search for shape_*
		while (ident && !image) {
			title = [ident stringByAppendingFormat:@"%@", e];
			image = [JSImage imageNamed:title];
            image = [image resizableImageWithCapInsets:self.capInsets];
			ident = [self nextLevelTitle:ident];
		}

		if (image) {
            [dict setObject:image forKey:[e isEqualToString:@""] ? kJSSkinStateNormal : e];
		}
	}

    self.currentImage = [dict objectForKey:kJSSkinStateNormal];

	return dict;
}

- (NSString *)nextLevelTitle:(NSString *)ident
{
	NSRange r = [ident rangeOfCharacterFromSet:[NSCharacterSet characterSetWithCharactersInString:@"_"]];
	if (r.location == NSNotFound) {
		return nil;
	}
	return [ident substringFromIndex:r.location + 1];
}

- (NSArray *)skinStateArray
{
	return [NSArray arrayWithObjects:
			kJSSkinStateNormal,
			kJSSkinStateHighlighted,
			kJSSkinStateSelected,
			kJSSkinStatePressed,
			kJSSkinStateDisabled,
			nil];
}

@end
