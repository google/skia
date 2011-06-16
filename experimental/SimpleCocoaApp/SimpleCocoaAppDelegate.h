#import <Cocoa/Cocoa.h>
#import "SkNSWindow.h"
@interface SimpleCocoaAppDelegate : NSObject <NSApplicationDelegate> {
    SkNSWindow *window;
}

@property (assign) IBOutlet SkNSWindow *window;
@end
