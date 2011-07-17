#import <UIKit/UIKit.h>
#import "SkUIRootViewController.h"
#import "SkUIView.h"

class SampleWindow;
class SkData;
@interface SkUIDetailViewController : UIViewController {
    UINavigationBar* fNavigationBar;
    UIPopoverController* fPopOverController;
    UIBarButtonItem* fPrintButton;
    UIBarButtonItem* fCycleButton;
    SkData* fData;
    SkUIView* fSkUIView;
    SampleWindow* fWind;
}
@property (nonatomic, retain) IBOutlet UINavigationBar *fNavigationBar;
@property (nonatomic, retain) UIBarButtonItem* fPrintButton;
@property (nonatomic, retain) UIBarButtonItem* fCycleButton;
@property (nonatomic, assign) UIPopoverController* fPopOverController;

//Instance methods
- (void)redraw;
- (void)populateRoot:(SkUIRootViewController*)root;
- (void)goToItem:(NSUInteger)index;
- (void)createButtons;
//UI actions
- (IBAction)usePipe:(id)sender;
- (IBAction)enterServerIP:(id)sender;
- (void)printContent;
- (void)cycleDeviceType;

//SplitView popover management
- (void)showRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem;
- (void)invalidateRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem;

@end
