#import "SimpleCocoaAppDelegate.h"

@implementation SimpleCocoaAppDelegate
@synthesize window;

-(void) applicationDidFinishLaunching:(NSNotification *)aNotification {
    //Load specified skia views after launching
    [window installSkViews];
}
@end
