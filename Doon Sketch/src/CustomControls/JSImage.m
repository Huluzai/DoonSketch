//
//  JSImage.m
//  PaintForMac
//
//  Created by Zhang Guangjian on 4/3/13.
//
//

#import "JSImage.h"


const JSEdgeInsets JSEdgeInsetsZero = {0, 0, 0, 0};
const JSEdgeInsets JSEdgeInsetsDefault = {-1, -1, -1, -1};

JSEdgeInsets JSEdgeInsetsMake(CGFloat top,
							  CGFloat left,
							  CGFloat bottom,
							  CGFloat right)
{
	JSEdgeInsets e;
	e.top = floorf(top);
	e.left = floorf(left);
	e.bottom = floorf(bottom);
	e.right = floorf(right);
	return e;
}

BOOL JSEqualInsets(JSEdgeInsets ins1, JSEdgeInsets ins2)
{
	return (ins1.left == ins2.left) &&
	(ins1.right == ins2.right) &&
	(ins1.top == ins2.top) &&
	(ins1.bottom == ins2.bottom);
}


@interface JSImage ()
- (void)setPartImages:(NSArray *)parts;
- (void)setCapInsets:(JSEdgeInsets)capInsets;
- (void)setResingMode:(JSImageResizingMode)resizingMode;
@end


@implementation JSImage

@synthesize resizingMode = _resizingMode;
@synthesize capInsets = _capInsets;
@synthesize stockName = _stockName;

+ (id)imageNamed:(NSString *)name
{
    return [JSImage imageNamed:name capInsets:JSEdgeInsetsZero];
}

+ (id)imageNamed:(NSString *)name capInsets:(JSEdgeInsets)insets
{
    static NSMutableArray *histImages = nil;
    
    if (histImages == nil) {
        histImages = [NSMutableArray array];
    }
    
    JSImage *newImage = nil;
    
    if (IS_STR_NOT_NIL(name))
        for (JSImage *image in histImages) {
            if ([image.stockName isEqualTo:name] &&
                JSEqualInsets(image.capInsets, insets)) {
                return image;
            }
            
            if ([image.stockName isEqualTo:name]) {
                newImage = image;
            }
        }
    
    if (newImage == nil) {
        NSURL *url = [[NSBundle mainBundle] URLForImageResource:name];
        newImage = url ? [[JSImage alloc] initWithContentsOfURL:url] : nil;
        [newImage setStockName:name];
        if (newImage && IS_STR_NOT_NIL(name)) {
            [histImages addObject:newImage];
        }
    }
    
    if (!JSEqualInsets(insets, JSEdgeInsetsZero)) {
        newImage = [newImage imageWithCapInsets:insets];
        
        if (newImage && IS_STR_NOT_NIL(name)) {
            [histImages addObject:newImage];
        }
    }
    
    return newImage;
}

- (BOOL)isEqualTo:(id)object
{
    if ([object isKindOfClass:self.class])
        return NO;
    
    JSImage *image = (JSImage *)object;
    
    if ([self.stockName isEqual:[image stockName]] &&
        JSEqualInsets(self.capInsets, image.capInsets))
        return YES;
    
    return NO;
}

- (void)dealloc
{
    _partImages = nil;
}

- (JSImage *)imageWithCapInsets:(JSEdgeInsets)capInsets
{
    if (JSEqualInsets(capInsets, JSEdgeInsetsZero))
        return self;
    
	NSSize totalSize = [self size];
    
	CGFloat width = capInsets.right - capInsets.left;
	CGFloat height = capInsets.bottom - capInsets.top;
	
	CGFloat w0 = MAX(1, capInsets.left), w1 = MAX(1, width), w2 = MAX(1, totalSize.width - capInsets.right);
	CGFloat h0 = MAX(1, capInsets.top), h1 = MAX(1, height), h2 = MAX(1, totalSize.height - capInsets.bottom);
    
	CGFloat x0 = 0;
	CGFloat x1 = MIN(totalSize.width - 1, capInsets.left);
	CGFloat x2 = MIN(totalSize.width - 1, capInsets.right);
	
	CGFloat y0 = 0;
	CGFloat y1 = MIN(totalSize.height - 1, capInsets.top);
	CGFloat y2 = MIN(totalSize.height - 1, capInsets.bottom);
	
	NSRect partRects[9];
	
	partRects[0] = NSMakeRect(x0, y2, w0, h2);
	partRects[1] = NSMakeRect(x1, y2, w1, h2);
	partRects[2] = NSMakeRect(x2, y2, w2, h2);
	
	partRects[3] = NSMakeRect(x0, y1, w0, h1);
	partRects[4] = NSMakeRect(x1, y1, w1, h1);
	partRects[5] = NSMakeRect(x2, y1, w2, h1);
	
	partRects[6] = NSMakeRect(x0, y0, w0, h0);
	partRects[7] = NSMakeRect(x1, y0, w1, h0);
	partRects[8] = NSMakeRect(x2, y0, w2, h0);
	
	NSArray *parts = [NSArray array];
	
	for (int i = 0; i < 9; i++) {
		if (partRects[i].size.width <= 0 || partRects[i].size.height <= 0) {
			parts = [parts arrayByAddingObject:[[NSImage alloc] init]];
			continue;
		}
		
		NSImage *img = [[NSImage alloc] initWithSize:partRects[i].size];
		[img lockFocusFlipped:NO];
		[self drawAtPoint:NSZeroPoint
				 fromRect:partRects[i]
				operation:NSCompositeCopy
				 fraction:1.0];
		[img unlockFocus];
		parts = [parts arrayByAddingObject:img];
	}
	
	JSImage *image = [self copy];
    
	[image setPartImages:parts];
	[image setCapInsets:capInsets];
    
	return image;
}

- (JSImage *)resizableImageWithCapInsets:(JSEdgeInsets)capInsets
{
    if (JSEqualInsets(JSEdgeInsetsDefault, capInsets)) {
        capInsets = JSEdgeInsetsMake(self.size.height / 2,
                                     self.size.width / 2,
                                     self.size.height / 2 + 1,
                                     self.size.width / 2 + 1);
    }
    
    if (!self.stockName || [self.stockName isEqualTo:@""]) {
        return [self imageWithCapInsets:capInsets];
    }
    return [JSImage imageNamed:self.stockName capInsets:capInsets];
}

- (void)drawInRect:(NSRect)dstRect
		 operation:(NSCompositingOperation)op
		  fraction:(CGFloat)delta
		 isFlipped:(BOOL)flipped
{
	if (JSEqualInsets(self.capInsets, JSEdgeInsetsZero)) {
		[super drawInRect:dstRect
				 fromRect:NSMakeRect(0, 0, self.size.width, self.size.height)
				operation:op
				 fraction:delta];
	} else {
		NSDrawNinePartImage(dstRect,
							_partImages[0],
							_partImages[1],
							_partImages[2],
							_partImages[3],
							_partImages[4],
							_partImages[5],
							_partImages[6],
							_partImages[7],
							_partImages[8],
							op, delta, flipped);
	}
}

#pragma mark private access

- (void)setPartImages:(NSArray *)parts
{
    _partImages = parts;
}

- (void)setCapInsets:(JSEdgeInsets)capInsets
{
	_capInsets = capInsets;
}

- (void)setResingMode:(JSImageResizingMode)resizingMode
{
	_resizingMode = resizingMode;
}

@end
