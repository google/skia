#import "SkMenuController.h"

@implementation SkMenuController
-(IBAction) openFile:(id) sender {
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    NSInteger response = [panel runModal];
    
    [panel setFloatingPanel:YES];
    [panel setCanChooseDirectories:NO];
    [panel setCanChooseFiles:YES];
    
    if(response == NSOKButton){
        [fWindow loadFile:[panel filename]];
    } 
}
@end
