#import "HelloWorldDelegate.h"

#include "SkApplication.h"

@implementation HelloWorldDelegate
@synthesize fWindow, fView, fOptions;

// for iOS
-(void) applicationDidFinishLaunching:(NSNotification *)aNotification {
    //Load specified skia views after launching
    fView.fOptionsDelegate = fOptions;
    [fWindow setAcceptsMouseMovedEvents:YES];
 // [fOptions registerMenus:fView.fWind->getMenus()];
}

- (IBAction)toiPadSize:(id)sender {
    NSRect frame = NSMakeRect(fWindow.frame.origin.x, fWindow.frame.origin.y, 768, 1024);
    [fWindow setFrame:frame display:YES animate:YES];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    [fView freeNativeWind];
    application_term();
    return NSTerminateNow;
}

@end
