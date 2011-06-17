#import <Cocoa/Cocoa.h>
#import "SkNSWindow.h"
@interface FileReaderAppDelegate : NSObject <NSApplicationDelegate> {
    SkNSWindow *window;
}

@property (assign) IBOutlet SkNSWindow *window;
@end
