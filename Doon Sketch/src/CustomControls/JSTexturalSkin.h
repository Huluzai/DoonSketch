//
//  JSTexturalSkin.h
//  PaintForMac
//
//  Created by Zhang Guangjian on 4/2/13.
//
//

#import <Foundation/Foundation.h>


@protocol JSMultiStateTexture <NSObject>

- (void)changeState:(NSUInteger)state;

- (void)drawWithFrame:(NSRect)frame inView:(NSView *)view;


@end


#import "JSImage.h"

enum
{
	JSSkinStateNormal = 0x0001,
	JSSkinStateSelected = 0x0002,
	JSSkinStateHighlighted = 0x0004,
	JSSkinStatePressed = 0x0008,
	JSSkinStateDisabled = 0x00010,
};

typedef NSUInteger JSSkinState;

@interface JSTexturalSkin: NSObject<JSMultiStateTexture> {
	JSImage *_currentImage;
	JSEdgeInsets _capInsets;
	NSDictionary *_stateImages;
	JSSkinState _oldState;
}

@property (retain) JSImage *currentImage;
@property (retain) NSDictionary *stateImages;
@property (assign, readonly) JSEdgeInsets capInsets;

- (id)initWithIdentifier:(NSString *)identifier;

- (id)initWithIdentifier:(NSString *)identifier
			   capInsets:(JSEdgeInsets)capInsets;

@end

