#import "SkNSWindow.h"
#import "ReaderView.h"
@interface FileReaderWindow : SkNSWindow {
    IBOutlet SkNSView* fView;
    ReaderView* fReaderView;
}
@end

