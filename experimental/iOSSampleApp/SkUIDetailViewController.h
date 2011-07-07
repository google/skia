#import <UIKit/UIKit.h>
#import "SkUIRootViewController.h"
#import "SkUIView_shell.h"

class SampleWindow;
class SkData;
@interface SkUIDetailViewController : UIViewController {
@private
    UINavigationBar* fNavigationBar;
    UIBarButtonItem* fPrintButton;
    SkData* fData;
    SkUIView_shell* fSkUIView;
    SampleWindow* fWind;
}
@property (nonatomic, retain) IBOutlet UINavigationBar *fNavigationBar;
@property (nonatomic, retain) IBOutlet UIBarButtonItem* fPrintButton;

//Instance methods
- (void)redraw;
- (void)populateRoot:(SkUIRootViewController*)root;
- (void)goToItem:(NSUInteger)index;

//UI actions
- (IBAction)printContent:(id)sender;
- (IBAction)usePipe:(id)sender;
- (IBAction)enterServerIP:(id)sender;

//SplitView popover management
- (void)showRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem;
- (void)invalidateRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem;

@end
