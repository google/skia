#import "SkUINavigationController.h"

@implementation SkUINavigationController
@synthesize fRoot, fDetail;

- (void)loadData {
    [fRoot initList];
    [fDetail populateRoot:fRoot];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES; //Allow auto rotation for all orientations
}

- (void)dealloc {
    [fRoot release];
    [fDetail release];
    [super dealloc];
}

@end
