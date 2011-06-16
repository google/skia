#import "SkNSWindow.h"
#import "SkDebuggerViews.h"
@interface SkDebugger : SkNSWindow {
    IBOutlet SkNSView* fCommandView;
    IBOutlet SkNSView* fContentView;
    IBOutlet SkNSView* fInfoView;
    
    SkCommandListView* fCommand;
    SkContentView* fContent;
    SkInfoPanelView* fInfo;
}

- (void)loadFile:(NSString *)filename;
@end

