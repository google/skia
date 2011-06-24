#import "SkUIRootViewController.h"
#import "SkUISplitViewController.h"
@implementation SkUIRootViewController

@synthesize popoverController, popoverButtonItem;


- (void)addItem:(NSString*)anItem {
    [fSamples addObject:anItem];
}

- (void)initList {
    fSamples = [[NSMutableArray alloc] init];
}

#pragma mark -
#pragma mark Rotation support

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}

#pragma mark -
#pragma mark View lifecycle


- (void)viewDidLoad {
    [super viewDidLoad];
    self.contentSizeForViewInPopover = CGSizeMake(200, self.view.bounds.size.height);
}

#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
    return [fSamples count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault 
                                       reuseIdentifier:CellIdentifier] autorelease];
    }
    
    cell.textLabel.text = [fSamples objectAtIndex:indexPath.row];
    return cell;
}

#pragma mark -
#pragma mark Memory management

- (void)viewDidUnload {
    [super viewDidUnload];
    self.popoverButtonItem = nil;
}

- (void)dealloc {
    [popoverController release];
    [popoverButtonItem release];
    [fSamples release];
    [super dealloc];
}
@end

