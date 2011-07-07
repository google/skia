#import "SkAlertPrompt.h"
#import "SkUIDetailViewController.h"
#include "SampleApp.h"
#include "SkApplication.h"
#include "SkCGUtils.h"
#include "SkData.h"
#include "SkWindow.h"

@implementation SkUIDetailViewController
@synthesize fNavigationBar, fPrintButton;

//Overwritten from UIViewController
- (void)viewDidLoad {
    [super viewDidLoad];

    fSkUIView = (SkUIView_shell*)self.view;
    fSkUIView.fTitle = fNavigationBar.topItem;

    application_init();
    fWind = (SampleWindow*)create_sk_window(self.view, NULL, NULL);
    CGSize s = self.view.bounds.size;
    fWind->resize(s.width, s.height);
    [fSkUIView setSkWindow:(SkOSWindow*)fWind];

    [NSTimer scheduledTimerWithTimeInterval:0.001 target:self
                                   selector:@selector(redraw) userInfo:nil
                                    repeats:YES];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES; // Overriden to allow auto rotation for any direction
}

- (void)dealloc {
    [fNavigationBar release];
    [fPrintButton release];
    application_term();
    delete fWind;
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

- (IBAction)printContent:(id)sender {
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
    // Add the popover button to the left navigation item.
    [fNavigationBar.topItem setLeftBarButtonItem:barButtonItem animated:NO];
}


- (void)invalidateRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem {
    // Remove the popover button.
    [fNavigationBar.topItem setLeftBarButtonItem:nil animated:NO];
}
@end
