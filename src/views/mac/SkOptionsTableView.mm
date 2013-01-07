
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import "SkOptionsTableView.h"
#import "SkTextFieldCell.h"
@implementation SkOptionItem
@synthesize fCell, fItem;
- (void)dealloc {
    [fCell release];
    [super dealloc];
}
@end

@implementation SkOptionsTableView
@synthesize fItems;

- (id)initWithCoder:(NSCoder*)coder {
    if ((self = [super initWithCoder:coder])) {
        self.dataSource = self;
        self.delegate = self;
        fMenus = NULL;
        fShowKeys = YES;
        [self setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
        self.fItems = [NSMutableArray array];
    }
    return self;
}

- (void)dealloc {
    self.fItems = nil;
    [super dealloc];
}

- (void) view:(SkNSView*)view didAddMenu:(const SkOSMenu*)menu {}
- (void) view:(SkNSView*)view didUpdateMenu:(const SkOSMenu*)menu {
    [self updateMenu:menu];
}

- (IBAction)toggleKeyEquivalents:(id)sender {
    fShowKeys = !fShowKeys;
    NSMenuItem* item = (NSMenuItem*)sender;
    [item setState:fShowKeys];
    [self reloadData];
}

- (void)registerMenus:(const SkTDArray<SkOSMenu*>*)menus {
    fMenus = menus;
    for (int i = 0; i < fMenus->count(); ++i) {
        [self loadMenu:(*fMenus)[i]];
    }
}

- (void)updateMenu:(const SkOSMenu*)menu {
    // the first menu is always assumed to be the static, the second is 
    // repopulated every time over and over again 

    // seems pretty weird that we have to get rid of the const'ness here,
    // but trying to propagate the const'ness through all the way to the fMenus
    // vector was a non-starter.

    int menuIndex = fMenus->find(const_cast<SkOSMenu *>(menu));
    if (menuIndex >= 0 && menuIndex < fMenus->count()) {
        NSUInteger first = 0;
        for (NSInteger i = 0; i < menuIndex; ++i) {
            first += (*fMenus)[i]->getCount();
        }
        [fItems removeObjectsInRange:NSMakeRange(first, [fItems count] - first)];
        [self loadMenu:menu];
    }
    [self reloadData];
}

- (NSCellStateValue)triStateToNSState:(SkOSMenu::TriState)state {
    if (SkOSMenu::kOnState == state)
        return NSOnState;
    else if (SkOSMenu::kOffState == state)
        return NSOffState;
    else
        return NSMixedState;
}

- (void)loadMenu:(const SkOSMenu*)menu {
    const SkOSMenu::Item* menuitems[menu->getCount()];
    menu->getItems(menuitems);
    for (int i = 0; i < menu->getCount(); ++i) {
        const SkOSMenu::Item* item = menuitems[i];
        SkOptionItem* option = [[SkOptionItem alloc] init];
        option.fItem = item;
        
        if (SkOSMenu::kList_Type == item->getType()) {
            int index = 0, count = 0;
            SkOSMenu::FindListItemCount(*item->getEvent(), &count);
            NSMutableArray* optionstrs = [[NSMutableArray alloc] initWithCapacity:count];
            SkAutoTDeleteArray<SkString> ada(new SkString[count]);
            SkString* options = ada.get();
            SkOSMenu::FindListItems(*item->getEvent(), options);
            for (int i = 0; i < count; ++i)
                [optionstrs addObject:[NSString stringWithUTF8String:options[i].c_str()]];
            SkOSMenu::FindListIndex(*item->getEvent(), item->getSlotName(), &index);
            option.fCell = [self createList:optionstrs current:index];
            [optionstrs release];
        }
        else {
            bool state = false;
            SkString str;
            SkOSMenu::TriState tristate;
            switch (item->getType()) {
                case SkOSMenu::kAction_Type:
                    option.fCell = [self createAction];
                    break;
                case SkOSMenu::kSlider_Type:
                    SkScalar min, max, value;
                    SkOSMenu::FindSliderValue(*item->getEvent(), item->getSlotName(), &value);
                    SkOSMenu::FindSliderMin(*item->getEvent(), &min);
                    SkOSMenu::FindSliderMax(*item->getEvent(), &max);
                    option.fCell = [self createSlider:value 
                                                  min:min 
                                                  max:max];
                    break;                    
                case SkOSMenu::kSwitch_Type:
                    SkOSMenu::FindSwitchState(*item->getEvent(), item->getSlotName(), &state);
                    option.fCell = [self createSwitch:(BOOL)state];
                    break;
                case SkOSMenu::kTriState_Type:
                    SkOSMenu::FindTriState(*item->getEvent(), item->getSlotName(), &tristate);
                    option.fCell = [self createTriState:[self triStateToNSState:tristate]];
                    break;
                case SkOSMenu::kTextField_Type:
                    SkOSMenu::FindText(*item->getEvent(),item->getSlotName(), &str);
                    option.fCell = [self createTextField:[NSString stringWithUTF8String:str.c_str()]];
                    break;
                default:
                    break;
            }
        }
        [fItems addObject:option];
        [option release];
    }
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [self.fItems count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    int columnIndex = [tableView columnWithIdentifier:[tableColumn identifier]];
    if (columnIndex == 0) {
        const SkOSMenu::Item* item = ((SkOptionItem*)[fItems objectAtIndex:row]).fItem;
        NSString* label = [NSString stringWithUTF8String:item->getLabel()];
        if (fShowKeys) 
            return [NSString stringWithFormat:@"%@ (%c)", label, item->getKeyEquivalent()];
        else 
            return label;
    }
    else
        return nil;
}

- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    if (tableColumn) {
        int columnIndex = [tableView columnWithIdentifier:[tableColumn identifier]];
        if (columnIndex == 1) 
            return [((SkOptionItem*)[fItems objectAtIndex:row]).fCell copy];
        else
            return [[[SkTextFieldCell alloc] init] autorelease];
    }
    return nil;
}

- (void)tableView:(NSTableView *)tableView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    int columnIndex = [tableView columnWithIdentifier:[tableColumn identifier]];
    if (columnIndex == 1) {
        SkOptionItem* option = (SkOptionItem*)[self.fItems objectAtIndex:row];
        NSCell* storedCell = option.fCell;
        const SkOSMenu::Item* item = option.fItem;
        switch (item->getType()) {
            case SkOSMenu::kAction_Type:
                break;                
            case SkOSMenu::kList_Type:
                [cell selectItemAtIndex:[(NSPopUpButtonCell*)storedCell indexOfSelectedItem]];
                break;
            case SkOSMenu::kSlider_Type:
                [cell setFloatValue:[storedCell floatValue]];
                break;
            case SkOSMenu::kSwitch_Type:
                [cell setState:[(NSButtonCell*)storedCell state]];
                break;
            case SkOSMenu::kTextField_Type:
                if ([[storedCell stringValue] length] > 0)
                    [cell setStringValue:[storedCell stringValue]];
                break;
            case SkOSMenu::kTriState_Type:
                [cell setState:[(NSButtonCell*)storedCell state]];
                break;
            default:
                break;
        }
    }
    else {
        [(SkTextFieldCell*)cell setEditable:NO];
    }
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    int columnIndex = [tableView columnWithIdentifier:[tableColumn identifier]];
    if (columnIndex == 1) {
        SkOptionItem* option = (SkOptionItem*)[self.fItems objectAtIndex:row];
        NSCell* cell = option.fCell;
        const SkOSMenu::Item* item = option.fItem;
        switch (item->getType()) {
            case SkOSMenu::kAction_Type:
                item->postEvent();
                break;
            case SkOSMenu::kList_Type:
                [(NSPopUpButtonCell*)cell selectItemAtIndex:[anObject intValue]];
                item->setInt([anObject intValue]);
                break;
            case SkOSMenu::kSlider_Type:
                [cell setFloatValue:[anObject floatValue]];
                item->setScalar([anObject floatValue]);
                break;
            case SkOSMenu::kSwitch_Type:
                [cell setState:[anObject boolValue]];
                item->setBool([anObject boolValue]);
                break;
            case SkOSMenu::kTextField_Type:
                if ([anObject length] > 0) {
                    [cell setStringValue:anObject];
                    item->setString([anObject UTF8String]);
                }
                break;
            case SkOSMenu::kTriState_Type:
                [cell setState:[anObject intValue]];
                item->setTriState((SkOSMenu::TriState)[anObject intValue]);
                break;
            default:
                break;
        }
        item->postEvent();
    }
}

- (NSCell*)createAction{
    NSButtonCell* cell = [[[NSButtonCell alloc] init] autorelease];
    [cell setTitle:@""];
    [cell setButtonType:NSMomentaryPushInButton];
    [cell setBezelStyle:NSSmallSquareBezelStyle];
    return cell;
}

- (NSCell*)createList:(NSArray*)items current:(int)index {
    NSPopUpButtonCell* cell = [[[NSPopUpButtonCell alloc] init] autorelease];
    [cell addItemsWithTitles:items];
    [cell selectItemAtIndex:index];
    [cell setArrowPosition:NSPopUpArrowAtBottom];
    [cell setBezelStyle:NSSmallSquareBezelStyle];
    return cell; 
}

- (NSCell*)createSlider:(float)value min:(float)min max:(float)max {
    NSSliderCell* cell = [[[NSSliderCell alloc] init] autorelease];
    [cell setFloatValue:value];
    [cell setMinValue:min];
    [cell setMaxValue:max];
    return cell;
}

- (NSCell*)createSwitch:(BOOL)state {
    NSButtonCell* cell = [[[NSButtonCell alloc] init] autorelease];
    [cell setState:state];
    [cell setTitle:@""];
    [cell setButtonType:NSSwitchButton];
    return cell;
}

- (NSCell*)createTextField:(NSString*)placeHolder; {
    SkTextFieldCell* cell = [[[SkTextFieldCell alloc] init] autorelease];
    [cell setEditable:YES];
    [cell setStringValue:@""];
    [cell setPlaceholderString:placeHolder];
    return cell;
}

- (NSCell*)createTriState:(NSCellStateValue)state {
    NSButtonCell* cell = [[[NSButtonCell alloc] init] autorelease];
    [cell setAllowsMixedState:TRUE];
    [cell setTitle:@""];
    [cell setState:(NSInteger)state];
    [cell setButtonType:NSSwitchButton];
    return cell;
}
@end
