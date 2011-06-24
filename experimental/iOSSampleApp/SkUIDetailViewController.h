#import <UIKit/UIKit.h>
#import "SkUIRootViewController.h"
#import "SkUIView_shell.h"
#import "SampleApp.h"
#import "SkData.h"
@interface SkUIDetailViewController : UIViewController {
    UINavigationBar* fNavigationBar;
    UIBarButtonItem* fPrintButton;
    @private
    SkData* fData;
    SkUIView_shell* fSkUIView;
    SampleWindow* fWind;
    CGPoint fInitialOffset, fInitialCenter;
    CGFloat fInitialScale, fInitialRotation;
    
}
@property (nonatomic, retain) IBOutlet UINavigationBar *fNavigationBar;
@property (nonatomic, retain) IBOutlet UIBarButtonItem* fPrintButton;

- (IBAction)printContent:(id)sender;

- (void)redraw;
- (void)populateRoot:(SkUIRootViewController*)root;
- (void)goToItem:(NSUInteger)index;

- (void)showRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem;
- (void)invalidateRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem;

- (void)initGestureRecognizers;
- (void)handleDoubleTapGesture:(UIGestureRecognizer *)sender;
- (void)handlePanGesture:(UIPanGestureRecognizer *)sender;
- (void)handlePinchGesture:(UIPinchGestureRecognizer *)sender;
- (void)handleSwipeGesture:(UISwipeGestureRecognizer *)sender;
- (void)handleRotationGesture:(UIRotationGestureRecognizer *)sender;

@end
