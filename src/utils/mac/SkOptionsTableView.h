
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
    BOOL fShowKeys;
}
@property (nonatomic, retain) NSMutableArray* fItems;

- (void)registerMenus:(const SkTDArray<SkOSMenu*>*)menus;
- (void)updateMenu:(const SkOSMenu*)menu;
- (void)loadMenu:(const SkOSMenu*)menu;
- (IBAction)toggleKeyEquivalents:(id)sender;

- (NSCell*)createAction;
- (NSCell*)createList:(NSArray*)items current:(int)index;
- (NSCell*)createSlider:(float)value min:(float)min max:(float)max;
- (NSCell*)createSwitch:(BOOL)state;
- (NSCell*)createTextField:(NSString*)placeHolder;
- (NSCell*)createTriState:(NSCellStateValue)state;

@end
