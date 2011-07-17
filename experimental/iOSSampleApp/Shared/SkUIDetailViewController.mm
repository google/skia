#import "SkAlertPrompt.h"
#import "SkUIDetailViewController.h"
#include "SampleApp.h"
#include "SkApplication.h"
#include "SkCGUtils.h"
#include "SkData.h"
@implementation SkUIDetailViewController
@synthesize fNavigationBar, fPrintButton, fCycleButton, fPopOverController;

//Overwritten from UIViewController
- (void)viewDidLoad {
    [super viewDidLoad];

    fSkUIView = (SkUIView*)self.view;
    fWind = (SampleWindow*)fSkUIView.fWind;
    fSkUIView.fTitleItem = fNavigationBar.topItem;

    [NSTimer scheduledTimerWithTimeInterval:0.001 target:self
                                   selector:@selector(redraw) userInfo:nil
                                    repeats:YES];
    [self createButtons];
}

- (void)createButtons {
    UIToolbar* toolbar = [[UIToolbar alloc]
                          initWithFrame:CGRectMake(0, 0, 150, 45)];
    [toolbar setBarStyle: UIBarStyleBlackOpaque];
    
    UIBarButtonItem* flexibleSpace = [[UIBarButtonItem alloc]
                                       initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                       target:nil
                                       action:nil];
    
    fCycleButton = [[UIBarButtonItem alloc]
                    initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh
                    target:self
                    action:@selector(cycleDeviceType)];
    fCycleButton.style = UIBarButtonItemStylePlain;
    
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

    [toolbar setItems:[NSArray arrayWithObjects:flexibleSpace, fCycleButton, fixedSpace, fPrintButton, nil]
             animated:NO];
    
    self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc]
                                              initWithCustomView:toolbar];
    [flexibleSpace release];
    [fixedSpace release];
    [toolbar release];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES; // Overriden to allow auto rotation for any direction
}

- (void)dealloc {
    [fNavigationBar release];
    [fPrintButton release];
    [fCycleButton release];
    [fPopOverController release];
    application_term();
    [super dealloc];
}

//Instance Methods
- (void)redraw {
    [self.view setNeedsDisplay];
}

- (void)populateRoot:(SkUIRootViewController*)rootVC {
    for (int i = 0; i < fWind->sampleCount(); ++i) {
        [rootVC addItem:[NSString stringWithUTF8String:fWind->getSampleTitle(i).c_str()]];
    }
}

- (void)goToItem:(NSUInteger)index {
    fWind->goToSample(index);
}

//UI actions
- (IBAction)usePipe:(id)sender {
    //fWind->togglePipe();
}

- (void)printContent {
    UIPrintInteractionController *controller = [UIPrintInteractionController sharedPrintController];
    UIPrintInfo *printInfo = [UIPrintInfo printInfo];
    printInfo.jobName = @"Skia iOS SampleApp";
    printInfo.duplex = UIPrintInfoDuplexLongEdge;
    printInfo.outputType = UIPrintInfoOutputGeneral;
    fWind->saveToPdf();
    [self.view drawRect:self.view.bounds];
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
}

- (IBAction)enterServerIP:(id)sender {
    SkAlertPrompt *prompt = [[SkAlertPrompt alloc] initWithTitle:@"Enter Server IP:"
                                                         message:@"\n"
                                                        delegate:self
                                               cancelButtonTitle:@"Cancel"
                                               otherButtonTitles:@"Enter", nil];
    // show the dialog box
    [prompt show];
    [prompt release];
}

- (void)cycleDeviceType {
    fWind->toggleRendering();
}

/*
- (void)presentActions {
    if (!fPopOverController) {
        SkOptionsTableViewController* controller = [[SkOptionsTableViewController alloc] 
                                                    initWithStyle:UITableViewStyleGrouped];
        fPopOverController = [[UIPopoverController alloc] initWithContentViewController:controller];
        fPopOverController.popoverContentSize = CGSizeMake(500, 400);
        [controller release];
    }
    
    if (fPopOverController.isPopoverVisible)
        [fPopOverController dismissPopoverAnimated:YES];
    else
        [fPopOverController presentPopoverFromBarButtonItem:fPrintButton 
                                   permittedArrowDirections:UIPopoverArrowDirectionAny 
                                                   animated:YES];

}
 */

// manage popup
- (void)alertView:(UIAlertView *)alertView willDismissWithButtonIndex:(NSInteger)buttonIndex
{
    if (buttonIndex != [alertView cancelButtonIndex])
    {
        NSString *entered = [(SkAlertPrompt*)alertView enteredText];
        //fWind->setServerIP([entered UTF8String]);
    }
}
//Popover Management
- (void)showRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem {
    [fNavigationBar.topItem setLeftBarButtonItem:barButtonItem animated:NO];
}

- (void)invalidateRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem {
    [fNavigationBar.topItem setLeftBarButtonItem:nil animated:NO];
}

@end