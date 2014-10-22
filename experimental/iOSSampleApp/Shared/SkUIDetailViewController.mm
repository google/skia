#import "SkUIDetailViewController.h"
#include "SampleApp.h"
#include "SkCGUtils.h"
#include "SkData.h"
#include "SkOSMenu.h"
@implementation SkUIDetailViewController
@synthesize fPrintButton, fOptionsButton, fPopOverController, fOptionsController;

//Overwritten from UIViewController
- (void)viewDidLoad {
    [super viewDidLoad];

    fSkUIView = (SkUIView*)self.view;
    
    fWind = (SampleWindow*)fSkUIView.fWind;
    fSkUIView.fTitleItem = self.navigationItem;
    
    [self createButtons];
    
    UISwipeGestureRecognizer* swipe = [[UISwipeGestureRecognizer alloc]
                                       initWithTarget:self 
                                       action:@selector(handleSwipe:)];
    [self.navigationController.navigationBar addGestureRecognizer:swipe];
    [swipe release];
    swipe = [[UISwipeGestureRecognizer alloc]
             initWithTarget:self 
             action:@selector(handleSwipe:)];
    swipe.direction = UISwipeGestureRecognizerDirectionLeft;
    [self.navigationController.navigationBar addGestureRecognizer:swipe];
    [swipe release];
    
    fOptionsController = [[SkOptionsTableViewController alloc] 
                          initWithStyle:UITableViewStyleGrouped];
    fSkUIView.fOptionsDelegate = fOptionsController;
    [fOptionsController registerMenus:fWind->getMenus()];
    
}

- (void)createButtons {
    UIToolbar* toolbar = [[UIToolbar alloc]
                          initWithFrame:CGRectMake(0, 0, 125, 45)];
    [toolbar setBarStyle: UIBarStyleBlackOpaque];
    
    UIBarButtonItem* flexibleSpace = [[UIBarButtonItem alloc]
                                       initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                       target:nil
                                       action:nil];
    
    fOptionsButton = [[UIBarButtonItem alloc]
                    initWithTitle:@"Options" 
                    style:UIBarButtonItemStylePlain
                    target:self
                    action:@selector(presentOptions)];
    UIBarButtonItem* fixedSpace = [[UIBarButtonItem alloc]
                                    initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace
                                    target:nil
                                    action:nil];
    fixedSpace.width = 10;
    
    fPrintButton = [[UIBarButtonItem alloc]
                    initWithBarButtonSystemItem:UIBarButtonSystemItemAction
                    target:self
                    action:@selector(printContent)];
    fPrintButton.style = UIBarButtonItemStylePlain;

    [toolbar setItems:[NSArray arrayWithObjects:flexibleSpace, fOptionsButton, fixedSpace, fPrintButton, nil]
             animated:NO];
    
    self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc]
                                              initWithCustomView:toolbar];
    [flexibleSpace release];
    [fixedSpace release];
    [toolbar release];
}

- (void)handleSwipe:(UISwipeGestureRecognizer *)sender {
    if (UISwipeGestureRecognizerDirectionRight == sender.direction)
        fWind->previousSample();
    else
        fWind->nextSample();
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES; // Overriden to allow auto rotation for any direction
}

- (void)dealloc {
    [fPrintButton release];
    [fOptionsButton release];
    [fPopOverController release];
    [fOptionsController release];
    [super dealloc];
}

//Instance Methods
- (void)populateRoot:(SkUIRootViewController*)rootVC {
    for (int i = 0; i < fWind->sampleCount(); ++i) {
        [rootVC addItem:[NSString stringWithUTF8String:fWind->getSampleTitle(i).c_str()]];
    }
}

- (void)goToItem:(NSUInteger)index {
    fWind->goToSample(index);
}

- (void)printContent {
    /* comment out until we rev. this to use SkDocument

    UIPrintInteractionController *controller = [UIPrintInteractionController sharedPrintController];
    UIPrintInfo *printInfo = [UIPrintInfo printInfo];
    printInfo.jobName = @"Skia iOS SampleApp";
    printInfo.duplex = UIPrintInfoDuplexLongEdge;
    printInfo.outputType = UIPrintInfoOutputGeneral;
    fWind->saveToPdf();
    [fSkUIView forceRedraw];
    fData = fWind->getPDFData();
    NSData* data = [NSData dataWithBytesNoCopy:(void*)fData->data() length:fData->size()];
    controller.printInfo = printInfo;
    controller.printingItem = data;
    //Add ref because data pointer retains a pointer to data
    fData->ref();

    void (^SkCompletionHandler)(UIPrintInteractionController *, BOOL, NSError *) =
    ^(UIPrintInteractionController *pic, BOOL completed, NSError *error) {
        fData->unref();
        if (!completed && error)
            NSLog(@"FAILED! due to error in domain %@ with error code %u",
                  error.domain, error.code);
    };

    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
        [controller presentFromBarButtonItem:fPrintButton animated:YES
                        completionHandler:SkCompletionHandler];
    } else {
        [controller presentAnimated:YES completionHandler:SkCompletionHandler];
    }
     */
}

- (void)presentOptions {
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
        if (nil == fPopOverController) {
            UINavigationController* navigation = [[UINavigationController alloc] 
                                                  initWithRootViewController:fOptionsController];
            navigation.navigationBar.topItem.title = @"Options";
            fPopOverController = [[UIPopoverController alloc] initWithContentViewController:navigation];
            [navigation release];
        }
        
        if (fPopOverController.isPopoverVisible)
            [fPopOverController dismissPopoverAnimated:YES];
        else
            [fPopOverController presentPopoverFromBarButtonItem:fOptionsButton 
                                       permittedArrowDirections:UIPopoverArrowDirectionAny 
                                                       animated:YES];
        
    } else {
        UIBarButtonItem* backButton = [[UIBarButtonItem alloc] initWithTitle:@"Back"
                                                                       style:UIBarButtonItemStyleBordered
                                                                      target:nil
                                                                      action:nil];
        self.navigationItem.backBarButtonItem = backButton;
        [backButton release];
        [self.navigationController pushViewController:fOptionsController animated:YES];
        self.navigationController.navigationBar.topItem.title = @"Options";
    }
}
 
//Popover Management
- (void)showRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem {
    [self.navigationItem setLeftBarButtonItem:barButtonItem animated:NO];
}

- (void)invalidateRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem {
    [self.navigationItem setLeftBarButtonItem:nil animated:NO];
}

@end