#import "AppDelegate_iPhone.h"

@implementation AppDelegate_iPhone
@synthesize window, fRoot;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    [window addSubview:fRoot.view];
    [window makeKeyAndVisible];
    return YES;
}

- (void)dealloc {
    [window release];
    [fRoot release];
    [super dealloc];
}

@end
