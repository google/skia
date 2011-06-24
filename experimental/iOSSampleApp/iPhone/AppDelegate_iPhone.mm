#import "AppDelegate_iPhone.h"

@implementation AppDelegate_iPhone

@synthesize window, fRoot, fDetail;

#pragma mark -
#pragma mark Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
    [window addSubview:fRoot.view];
    [window addSubview:fDetail.view];
    [fRoot loadData];
    fDetail.view.hidden = YES;
    [window makeKeyAndVisible];
    
    return YES;
}

#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [fDetail goToItem:indexPath.row];
    [UIView transitionWithView:window
                      duration:0.5
                       options:UIViewAnimationOptionTransitionFlipFromRight
                    animations:^{ 
                        fRoot.view.hidden = YES;
                        fDetail.view.hidden = NO;
                    }
                    completion:NULL];
}

- (IBAction)displaySampleList:(id)sender
{
    [UIView transitionWithView:window
                      duration:0.5
                       options:UIViewAnimationOptionTransitionFlipFromLeft
                    animations:^{ 
                        fRoot.view.hidden = NO;
                        fDetail.view.hidden = YES;
                    }
                    completion:NULL];
}

- (void)dealloc {
    [window release];
    [fRoot release];
    [fDetail release];
    [super dealloc];
}


@end
