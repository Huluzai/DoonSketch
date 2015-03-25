//
//  ZGGradient.m
//  Inxcape
//
//  Created by 张 光建 on 14-10-14.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "ZGGradient.h"
#import <sp-gradient.h>
#import <sigc++/sigc++.h>
#import <display/nr-plain-stuff.h>
#import <sp-gradient-fns.h>

@interface ZGGradient ()
@end


@implementation ZGGradient

- (id)initWithSPGradient:(SPGradient *)gr
{
    return [super initWithGObject:gr takeCopy:FALSE];
}

- (SPGradient *)spGradient
{
    return SP_GRADIENT([super gObj]);
}

- (void)didObjectReleased:(ZGObject *)object
{
    if ([self.delegate respondsToSelector:@selector(didGradientReleased:)]) {
        [self.delegate didGradientReleased:self];
    }
    
    [super didObjectReleased:self];
}

+ (NSImage *)gradientImage:(SPGradient *)gradient
{
    int width = 256, height = 20;
    
    NSBitmapImageRep *bir = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                                    pixelsWide:width
                                                                    pixelsHigh:height
                                                                 bitsPerSample:8
                                                               samplesPerPixel:3
                                                                      hasAlpha:FALSE
                                                                      isPlanar:FALSE
                                                                colorSpaceName:NSCalibratedRGBColorSpace
                                                                   bytesPerRow:0
                                                                  bitsPerPixel:24];
    
    nr_render_checkerboard_rgb (bir.bitmapData, width, height, (int)bir.bytesPerRow, 0, 0);
    sp_gradient_render_vector_block_rgb (gradient,
                                         bir.bitmapData, width, height, (int)bir.bytesPerRow,
                                         0, width, TRUE);
    
    NSImage *image = [[NSImage alloc] initWithData:bir.TIFFRepresentation];
    
    return image;
}

@end
