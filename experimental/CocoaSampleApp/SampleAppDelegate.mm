#import "SampleAppDelegate.h"
@implementation SampleAppDelegate
@synthesize fWindow, fView, fOptions;

-(void) applicationDidFinishLaunching:(NSNotification *)aNotification {
    //Load specified skia views after launching
    fView.fOptionsDelegate = fOptions;
    [fOptions registerMenus:fView.fWind->getMenus()];
}

static float deltaw = 120;
static float deltah = 80;

- (IBAction)decreaseWindowSize:(id)sender {
    NSRect frame = [fWindow frame];
    frame.origin.y += deltah;
    frame.size.width -=deltaw;
    frame.size.height -= deltah;
    
    [fWindow setFrame:frame display:YES animate:YES];
}

- (IBAction)increaseWindowSize:(id)sender {
    NSRect frame = [fWindow frame];
    frame.origin.y -= deltah;
    frame.size.width += deltaw;
    frame.size.height += deltah;
    
    [fWindow setFrame:frame display:YES animate:YES];
}
@end
