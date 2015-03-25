//
//  EditorViews.h
//  PencilSketch
//
//  Created by 张光建 on 14-4-13.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "JSTexturalView.h"

@interface ImageToolbarView : JSTexturalView
@end

@interface ImageFootbarView : JSTexturalView
@end

@interface OptionAreaBackground : JSTexturalView
@end

@interface BlockView : JSTexturalView
@end

@interface BlockHeaderView : BlockView
@end

@interface BackgroundView : BlockView
@end

@interface ToolbarView : BlockView
@end

@interface ParametersView : BlockView
@end

@interface GreenTileView : BlockView
@end

@interface DarkTileView : BlockView
@end
