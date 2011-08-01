#import <Cocoa/Cocoa.h>
#import "SkNSView.h"
#import "SkOSMenu.h"
#import "SkEvent.h"
@interface SkOptionItem : NSObject {
    NSCell* fCell;
    const SkOSMenu::Item* fItem;
}
@property (nonatomic, assign) const SkOSMenu::Item* fItem;
@property (nonatomic, retain) NSCell* fCell;
@end

@interface SkOptionsTableView : NSTableView <SkNSViewOptionsDelegate, NSTableViewDelegate, NSTableViewDataSource> {
    NSMutableArray* fItems;
    const SkTDArray<SkOSMenu*>* fMenus;
}
@property (nonatomic, retain) NSMutableArray* fItems;

- (void)registerMenus:(const SkTDArray<SkOSMenu*>*)menus;
- (void)updateMenu:(const SkOSMenu*)menu;
- (void)loadMenu:(const SkOSMenu*)menu;

- (NSCell*)createAction;
- (NSCell*)createList:(NSArray*)items current:(int)index;
- (NSCell*)createSegmented:(NSArray*)items current:(int)index;
- (NSCell*)createSlider:(float)value min:(float)min max:(float)max;
- (NSCell*)createSwitch:(BOOL)state;
- (NSCell*)createTextField:(NSString*)placeHolder;
- (NSCell*)createTriState:(NSCellStateValue)state;

@end
