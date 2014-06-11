#import "AppDelegate_iPhone.h"

@implementation AppDelegate_iPhone
@synthesize window, fRoot;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    [window addSubview:fRoot.view];
    [window makeKeyAndVisible];

    self.window.rootViewController = fRoot;

    return YES;
}

- (void)dealloc {
    [window release];
    [fRoot release];
    [super dealloc];
}

@end
