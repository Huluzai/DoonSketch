//
//  JSImage.h
//  PaintForMac
//
//  Created by Zhang Guangjian on 4/3/13.
//
//

#import <Foundation/Foundation.h>

typedef enum {
	JSImageResizingModeTile,
	JSImageResizingModeStretch,
} JSImageResizingMode;

typedef struct {
	CGFloat top, left, bottom, right;
} JSEdgeInsets;

extern const JSEdgeInsets JSEdgeInsetsZero;
extern const JSEdgeInsets JSEdgeInsetsDefault;

extern JSEdgeInsets JSEdgeInsetsMake(CGFloat top,
									 CGFloat left,
									 CGFloat bottom,
									 CGFloat right);

extern BOOL JSEqualInsets(JSEdgeInsets ins1, JSEdgeInsets ins2);


@interface JSImage : NSImage {
	NSArray *_partImages;
	JSImageResizingMode _resizingMode;
	JSEdgeInsets _capInsets;
    NSString *_stockName;
}

@property(nonatomic, readonly) JSImageResizingMode resizingMode;
@property(nonatomic, readonly) JSEdgeInsets capInsets;
@property(retain, readwrite) NSString *stockName;

- (JSImage *)resizableImageWithCapInsets:(JSEdgeInsets)capInsets;

- (void)drawInRect:(NSRect)dstRect
		 operation:(NSCompositingOperation)op
		  fraction:(CGFloat)delta
		 isFlipped:(BOOL)flipped;

@end
