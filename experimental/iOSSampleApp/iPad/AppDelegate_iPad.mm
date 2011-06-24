#import "AppDelegate_iPad.h"

@implementation AppDelegate_iPad

@synthesize window, splitViewController;

#pragma mark -
#pragma mark Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
    
    // Override point for customization after application launch.
    
    [window addSubview:[splitViewController view]];
    [window makeKeyAndVisible];
    [splitViewController loadData];
    return YES;
}

- (void)dealloc {
    [window release];
    [splitViewController release];
    [super dealloc];
}

@end
