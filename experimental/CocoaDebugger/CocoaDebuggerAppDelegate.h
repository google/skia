#import <Cocoa/Cocoa.h>
#import "SkNSWindow.h"
@interface CocoaDebuggerAppDelegate : NSObject <NSApplicationDelegate> {
    SkNSWindow *window;
}

@property (assign) IBOutlet SkNSWindow *window;
@end
