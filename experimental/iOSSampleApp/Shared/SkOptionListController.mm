#import "SkOptionListController.h"

@implementation SkOptionListController

@synthesize fOptions, fSelectedIndex, fSelectedCell, fParentCell;

#pragma mark -
#pragma mark Initialization

- (id)initWithStyle:(UITableViewStyle)style {
    self = [super initWithStyle:style];
    if (self) {
        self.fOptions = [[NSMutableArray alloc] init];
        self.fSelectedIndex = 0;
        self.fSelectedCell = nil;
    }
    return self;
}

- (void)addOption:(NSString*)option {
    [fOptions addObject:option];
}

- (NSString*)getSelectedOption {
    return (NSString*)[fOptions objectAtIndex:self.fSelectedIndex];
}

#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [fOptions count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
    cell.textLabel.text = [fOptions objectAtIndex:indexPath.row];
    if (indexPath.row == fSelectedIndex) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
        self.fSelectedCell = cell;
    }
    else
        cell.accessoryType = UITableViewCellAccessoryNone;
    
    return cell;
}

#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell* cell = [tableView cellForRowAtIndexPath:indexPath];
    self.fSelectedCell.accessoryType = UITableViewCellAccessoryNone;
    self.fSelectedCell = cell;
    cell.accessoryType = UITableViewCellAccessoryCheckmark;
    self.fParentCell.detailTextLabel.text = cell.textLabel.text;;
    self.fSelectedIndex = indexPath.row;
    [self.navigationController popViewControllerAnimated:YES];
}

- (void)dealloc {
    self.fOptions = nil;
    self.fSelectedCell = nil;
    [super dealloc];
}

@end
