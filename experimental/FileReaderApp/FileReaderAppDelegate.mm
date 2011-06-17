#import "FileReaderAppDelegate.h"

@implementation FileReaderAppDelegate
@synthesize window;

-(void) applicationDidFinishLaunching:(NSNotification *)aNotification {
    //Load specified skia views after launching
    [window installSkViews];
}
@end
