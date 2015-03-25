//
//  GeomPropertiesPage.m
//  Inxcape
//
//  Created by 张 光建 on 14/11/26.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import "GeomPropertiesPage.h"
#import "AttributeColumn.h"
#import "ZGBox.h"
#import "ZGDesktop.h"

@interface GeomPropertiesPage () {
    ZGDesktop *_desktop;
}
@property NSMutableArray *columns;
@property ZGVBox *vbox;
@end

@implementation GeomPropertiesPage

- (void)dealloc
{
    for (AttributeColumn *c in self.columns) {
        [c.view removeObserver:self
                    forKeyPath:@"hidden"];
    }
 
    self.desktop = nill;
    self.vbox = nill;
    self.columns = nill;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    
    self.vbox = [[ZGVBox alloc] initWithView:self.view];
    self.columns = [NSMutableArray array];
    
    [self addColumn:self.positionColumn];
    [self addColumn:self.roundedColumn];
    [self addColumn:self.starColumn];
    [self addColumn:self.ellipseColumn];
    
    for (AttributeColumn* c in self.columns) {
        [c.view addObserver:self
                 forKeyPath:@"hidden"
                    options:NSKeyValueObservingOptionNew
                    context:NULL];
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    if ([keyPath isEqualToString:@"hidden"]) {
        [self.vbox relayout];
    }
}

- (void)setDesktop:(ZGDesktop *)desktop
{
    _desktop = desktop;
    
    for (AttributeColumn *c in self.columns) {
        c.desktop = desktop;
    }
}

- (ZGDesktop *)desktop
{
    return _desktop;
}

- (NSArray *)allColumns
{
    return [NSArray arrayWithArray:self.columns];
}

- (void)addColumn:(AttributeColumn *)column
{
    if (column) {
        column.desktop = self.desktop;
        [self.vbox packStart:column.view];
        [self.columns addObject:column];
    }
}

- (void)removeColumn:(AttributeColumn *)column atIndex:(NSUInteger)index
{
}

- (void)insertColumn:(AttributeColumn *)column atIndex:(NSUInteger)index
{
}

@end
