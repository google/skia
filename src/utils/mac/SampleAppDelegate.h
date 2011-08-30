#import <Cocoa/Cocoa.h>
#import "SkSampleNSView.h"
#import "SkOptionsTableView.h"
@interface SampleAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow* fWindow;
    SkSampleNSView* fView;
    SkOptionsTableView* fOptions;
}

@property (assign) IBOutlet NSWindow* fWindow;
@property (assign) IBOutlet SkSampleNSView* fView;
@property (assign) IBOutlet SkOptionsTableView* fOptions;

- (IBAction)toiPadSize:(id)sender;
@end
