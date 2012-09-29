#import "SkUINavigationController.h"

@implementation SkUINavigationController
@synthesize fRoot, fDetail;

- (void)viewDidLoad {
    [super viewDidLoad];
    [fDetail populateRoot:fRoot];
    [self pushViewController:fDetail animated:NO];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES; //Allow auto rotation for all orientations
}

- (void)dealloc {
    [fRoot release];
    [fDetail release];
    [super dealloc];
}

//Table View Delegate Methods
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [fDetail goToItem:indexPath.row];
    [self pushViewController:fDetail animated:YES];
}

@end