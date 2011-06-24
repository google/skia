#import "SkUISplitViewController.h"


@implementation SkUISplitViewController
@synthesize fRoot, fDetail;

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES; //Auto Rotation for all orientations
}

- (void)loadData {
    [fRoot initList];
    [fDetail populateRoot:fRoot];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.delegate = self;
}

- (void)dealloc {
    [fRoot release];
    [fDetail release];
    [super dealloc];
}

#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [fDetail goToItem:indexPath.row];
    if (fRoot.popoverController != nil) {
        [fRoot.popoverController dismissPopoverAnimated:YES];
    }
}

#pragma mark -
#pragma mark  Split view controller delegate
- (void)splitViewController:(UISplitViewController*)svc 
     willHideViewController:(UIViewController *)aViewController 
          withBarButtonItem:(UIBarButtonItem*)barButtonItem 
       forPopoverController:(UIPopoverController*)pc {
    
    barButtonItem.title = @"Samples";
    fRoot.popoverController = pc;
    fRoot.popoverButtonItem = barButtonItem;
    [fDetail showRootPopoverButtonItem:fRoot.popoverButtonItem];
}


- (void)splitViewController:(UISplitViewController*)svc 
     willShowViewController:(UIViewController *)aViewController 
  invalidatingBarButtonItem:(UIBarButtonItem *)barButtonItem {
    [fDetail invalidateRootPopoverButtonItem:fRoot.popoverButtonItem];
    fRoot.popoverController = nil;
    fRoot.popoverButtonItem = nil;
}

@end