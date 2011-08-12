#import <Cocoa/Cocoa.h>
#import "SkNSView.h"
#import "SkOptionsTableView.h"
@interface SampleAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow* fWindow;
    SkNSView* fView;
    SkOptionsTableView* fOptions;
}

@property (assign) IBOutlet NSWindow* fWindow;
@property (assign) IBOutlet SkNSView* fView;
@property (assign) IBOutlet SkOptionsTableView* fOptions;

- (IBAction)toiPadSize:(id)sender;
@end
