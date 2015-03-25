//
//  JSTexturalView.h
//  PaintForMac
//
//  Created by Zhang Guangjian on 4/5/13.
//
//

#import <Cocoa/Cocoa.h>

@protocol JSMultiStateTexture;

@interface JSTexturalView : NSView {
	id<JSMultiStateTexture> _texture;
	NSColor *_backgroundColor;
}

@property (retain) NSColor *backgroundColor;

- (id)initWithIdentifier:(NSString *)identifier;

- (id<JSMultiStateTexture>)texturalSkin;

@end
