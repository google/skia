#import "SkOptionsTableViewController.h"
#include "SkEvent.h"
#include "SkTArray.h"

@implementation SkOptionItem
@synthesize fCell, fItem;
- (void)dealloc {
    [fCell release];
    [super dealloc];
}
@end

@implementation SkOptionListItem
@synthesize fOptions;
- (void)dealloc {
    [fOptions release];
    [super dealloc];
}
@end

@implementation SkOptionsTableViewController

@synthesize fItems, fCurrentList;

- (id)initWithStyle:(UITableViewStyle)style {
    self = [super initWithStyle:style];
    if (self) {
        self.fItems = [NSMutableArray array];
    }
    return self;
}

//SkUIViewOptionsDelegate
- (void) view:(SkUIView*)view didAddMenu:(const SkOSMenu*)menu {}
- (void) view:(SkUIView*)view didUpdateMenu:(SkOSMenu*)menu {
    [self updateMenu:menu];
}

- (NSUInteger)convertPathToIndex:(NSIndexPath*)path {
    NSUInteger index = 0;
    for (NSInteger i = 0; i < path.section; ++i) {
        index += (*fMenus)[i]->getCount();
    }
    return index + path.row;
}

- (void)registerMenus:(const SkTDArray<SkOSMenu*>*)menus {
    fMenus = menus;
    for (NSUInteger i = 0; i < fMenus->count(); ++i) {
        [self loadMenu:(*fMenus)[i]];
    }
}

- (void)updateMenu:(SkOSMenu*)menu {
    // the first menu is always assumed to be the static, the second is 
    // repopulated every time over and over again 
    int menuIndex = fMenus->find(menu);
    if (menuIndex >= 0 && menuIndex < fMenus->count()) {
        NSUInteger first = 0;
        for (NSInteger i = 0; i < menuIndex; ++i) {
            first += (*fMenus)[i]->getCount();
        }
        [fItems removeObjectsInRange:NSMakeRange(first, [fItems count] - first)];
        [self loadMenu:menu];
    }
    [self.tableView reloadData];
}

- (void)loadMenu:(SkOSMenu*)menu {
    const SkOSMenu::Item* menuitems[menu->getCount()];
    menu->getItems(menuitems);
    for (int i = 0; i < menu->getCount(); ++i) {
        const SkOSMenu::Item* item = menuitems[i];
        NSString* title = [NSString stringWithUTF8String:item->getLabel()];
        
        if (SkOSMenu::kList_Type == item->getType()) {
            int value = 0;
            SkOptionListItem* List = [[SkOptionListItem alloc] init];

            List.fItem = item;
            List.fOptions = [[SkOptionListController alloc] initWithStyle:UITableViewStyleGrouped];
            
            int count = 0;
            SkOSMenu::FindListItemCount(*item->getEvent(), &count);
            SkTArray<SkString> options;
            options.resize_back(count);
            SkOSMenu::FindListItems(*item->getEvent(), &options.front());
            for (int i = 0; i < count; ++i)
                [List.fOptions addOption:[NSString stringWithUTF8String:options[i].c_str()]];
            SkOSMenu::FindListIndex(*item->getEvent(), item->getSlotName(), &value);
            
            List.fOptions.fSelectedIndex = value;
            List.fCell = [self createList:title
                                      default:[List.fOptions getSelectedOption]];
            List.fOptions.fParentCell = List.fCell;
            [fItems addObject:List];
            [List release];
        }
        else {
            SkOptionItem* option = [[SkOptionItem alloc] init];
            option.fItem = item;
 
            bool state = false;
            SkString str;
            SkOSMenu::TriState tristate;
            switch (item->getType()) {
                case SkOSMenu::kAction_Type:
                    option.fCell = [self createAction:title];
                    break;
                case SkOSMenu::kSwitch_Type:
                    SkOSMenu::FindSwitchState(*item->getEvent(), item->getSlotName(), &state);
                    option.fCell = [self createSwitch:title default:(BOOL)state];
                    break;
                case SkOSMenu::kSlider_Type:
                    SkScalar min, max, value;
                    SkOSMenu::FindSliderValue(*item->getEvent(), item->getSlotName(), &value);
                    SkOSMenu::FindSliderMin(*item->getEvent(), &min);
                    SkOSMenu::FindSliderMax(*item->getEvent(), &max);
                    option.fCell = [self createSlider:title 
                                                  min:min 
                                                  max:max
                                              default:value];
                    break;                    
                case SkOSMenu::kTriState_Type:
                    SkOSMenu::FindTriState(*item->getEvent(), item->getSlotName(), &tristate);
                    option.fCell = [self createTriState:title default:(int)tristate];
                    break;
                case SkOSMenu::kTextField_Type:
                    SkOSMenu::FindText(*item->getEvent(), item->getSlotName(), &str);
                    option.fCell = [self createTextField:title 
                                                 default:[NSString stringWithUTF8String:str.c_str()]];
                    break;
                default:
                    break;
            }
            [fItems addObject:option];
            [option release];
        }
    }
}

- (void)valueChanged:(id)sender {
    UITableViewCell* cell = (UITableViewCell*)(((UIView*)sender).superview);
    NSUInteger index = [self convertPathToIndex:[self.tableView indexPathForCell:cell]];
    SkOptionItem* item = (SkOptionItem*)[fItems objectAtIndex:index];
    if ([sender isKindOfClass:[UISlider class]]) {//Slider
        UISlider* slider = (UISlider *)sender;
        cell.detailTextLabel.text = [NSString stringWithFormat:@"%1.1f", slider.value];
        item.fItem->setScalar(slider.value);
    }
    else if ([sender isKindOfClass:[UISwitch class]]) {//Switch
        UISwitch* switch_ = (UISwitch *)sender;
        item.fItem->setBool(switch_.on);
    }
    else if ([sender isKindOfClass:[UITextField class]]) { //TextField
        UITextField* textField = (UITextField *)sender;
        [textField resignFirstResponder];
        item.fItem->setString([textField.text UTF8String]);
    }
    else if ([sender isKindOfClass:[UISegmentedControl class]]) { //Action
        UISegmentedControl* segmented = (UISegmentedControl *)sender;
        SkOSMenu::TriState state;
        if (2 == segmented.selectedSegmentIndex) {
            state = SkOSMenu::kMixedState;
        } else {
            state = (SkOSMenu::TriState)segmented.selectedSegmentIndex;
        }
        item.fItem->setTriState(state);
    }
    else{
        NSLog(@"unknown");
    }
    item.fItem->postEvent();
}

- (UITableViewCell*)createAction:(NSString*)title {
    UITableViewCell* cell = [[[UITableViewCell alloc]
                              initWithStyle:UITableViewCellStyleValue1 
                              reuseIdentifier:nil] autorelease];
    cell.textLabel.text = title;
    return cell;
}

- (UITableViewCell*)createSwitch:(NSString*)title default:(BOOL)state {
    UITableViewCell* cell = [[[UITableViewCell alloc] 
                              initWithStyle:UITableViewCellStyleValue1 
                              reuseIdentifier:nil] autorelease];
    cell.textLabel.text = title;
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
    UISwitch* switchView = [[UISwitch alloc] initWithFrame:CGRectZero];
    [switchView setOn:state animated:NO];
    [switchView addTarget:self 
                   action:@selector(valueChanged:) 
         forControlEvents:UIControlEventValueChanged];
    cell.accessoryView = switchView;
    [switchView release];
    return cell;
}

- (UITableViewCell*)createSlider:(NSString*)title 
                             min:(float)min 
                             max:(float)max 
                         default:(float)value {
    UITableViewCell* cell = [[[UITableViewCell alloc] 
                             initWithStyle:UITableViewCellStyleValue1 
                             reuseIdentifier:nil] autorelease];
    cell.textLabel.text = title;
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
    UISlider* sliderView = [[UISlider alloc] init];
    sliderView.value = value;
    sliderView.minimumValue = min;
    sliderView.maximumValue = max;
    [sliderView addTarget:self 
                   action:@selector(valueChanged:) 
         forControlEvents:UIControlEventValueChanged];
    cell.detailTextLabel.text = [NSString stringWithFormat:@"%1.1f", value];
    cell.accessoryView = sliderView; 
    [sliderView release];
    return cell;
}

- (UITableViewCell*)createTriState:(NSString*)title default:(int)index {
    UITableViewCell* cell = [[[UITableViewCell alloc] 
                              initWithStyle:UITableViewCellStyleValue1 
                              reuseIdentifier:nil] autorelease];
    cell.textLabel.text = title;
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
    NSArray* items = [NSArray arrayWithObjects:@"Off", @"On", @"Mixed", nil];
    UISegmentedControl* segmented = [[UISegmentedControl alloc] initWithItems:items];
    segmented.selectedSegmentIndex = (index == -1) ? 2 : index;
    segmented.segmentedControlStyle = UISegmentedControlStyleBar;
    [segmented addTarget:self 
                  action:@selector(valueChanged:) 
        forControlEvents:UIControlEventValueChanged];
    cell.accessoryView = segmented;
    [segmented release];
    return cell; 
}

- (UITableViewCell*)createTextField:(NSString*)title 
                            default:(NSString*)value {
    UITableViewCell* cell = [[[UITableViewCell alloc] 
                              initWithStyle:UITableViewCellStyleValue1 
                              reuseIdentifier:nil] autorelease];
    cell.textLabel.text = title;
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
    UITextField* textField = [[UITextField alloc] 
                              initWithFrame:CGRectMake(0, 10, 150, 25)];
    textField.adjustsFontSizeToFitWidth = YES;
    textField.textAlignment = NSTextAlignmentRight;
    textField.textColor = cell.detailTextLabel.textColor;
    textField.placeholder = value;
    textField.returnKeyType = UIReturnKeyDone;
    [textField addTarget:self 
                  action:@selector(valueChanged:) 
        forControlEvents:UIControlEventEditingDidEndOnExit];
    cell.accessoryView = textField; 
    [textField release];
    return cell;
}

- (UITableViewCell*)createList:(NSString*)title default:(NSString*)value{
    UITableViewCell* cell = [[[UITableViewCell alloc] 
                              initWithStyle:UITableViewCellStyleValue1 
                              reuseIdentifier:nil] autorelease];
    cell.textLabel.text = title;
    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
    cell.detailTextLabel.text = value;
    return cell; 
}

#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return fMenus->count();
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
    return [NSString stringWithUTF8String:(*fMenus)[section]->getTitle()];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return (*fMenus)[section]->getCount();
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    return ((SkOptionItem*)[fItems objectAtIndex:[self convertPathToIndex:indexPath]]).fCell;
}

#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell* cell = [tableView cellForRowAtIndexPath:indexPath];
    id item = [fItems objectAtIndex:[self convertPathToIndex:indexPath]];
    
    if ([item isKindOfClass:[SkOptionListItem class]]) {
        SkOptionListItem* list = (SkOptionListItem*)item;
        self.fCurrentList = list;
        self.navigationController.delegate = self;
        [self.navigationController pushViewController:list.fOptions animated:YES];
    }
    else if ([item isKindOfClass:[SkOptionItem class]]) {
        if (UITableViewCellSelectionStyleNone != cell.selectionStyle) { //Actions
            SkOptionItem* action = (SkOptionItem*)item;
            action.fItem->postEvent();
        }
    } 
    else{
        NSLog(@"unknown");
    }

    [self.tableView deselectRowAtIndexPath:indexPath animated:YES];
}

#pragma mark -
#pragma mark Navigation controller delegate

- (void)navigationController:(UINavigationController *)navigationController 
      willShowViewController:(UIViewController *)viewController 
                    animated:(BOOL)animated {
    if (self == viewController) { //when a List option is popped, trigger event
        NSString* selectedOption = [fCurrentList.fOptions getSelectedOption];
        fCurrentList.fCell.detailTextLabel.text = selectedOption;
        fCurrentList.fItem->setInt(fCurrentList.fOptions.fSelectedIndex);
        fCurrentList.fItem->postEvent();
    }
}

#pragma mark -
#pragma mark Memory management

- (void)dealloc {
    self.fItems = nil;
    [super dealloc];
}

@end