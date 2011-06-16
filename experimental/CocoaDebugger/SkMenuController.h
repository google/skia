#import <Cocoa/Cocoa.h>
#import "SkDebugger.h"
@interface SkMenuController : NSObject {
    IBOutlet SkDebugger *fWindow;
}
-(IBAction) openFile:(id) sender;
@end
