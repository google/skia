#import "SkUIRootViewController.h"
#import "SkUISplitViewController.h"
@implementation SkUIRootViewController
@synthesize popoverController, popoverButtonItem;

//Overwritten from UIViewController
- (void)viewDidLoad {
    [super viewDidLoad];
    self.contentSizeForViewInPopover = CGSizeMake(200, self.view.bounds.size.height);
    fSamples = [[NSMutableArray alloc] init];
}

- (void)viewDidUnload {
    [super viewDidUnload];
    self.popoverButtonItem = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}

- (void)dealloc {
    [popoverController release];
    [popoverButtonItem release];
    [fSamples release];
    [super dealloc];
}


//Table View Delegate Methods
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

//Instance methods
- (void)addItem:(NSString*)anItem {
    [fSamples addObject:anItem];
}

@end

