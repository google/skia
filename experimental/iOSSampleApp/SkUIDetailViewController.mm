#import "SkUIDetailViewController.h"
#import "SkApplication.h"
#import "SkWindow.h"
#import "SkCGUtils.h"

@implementation SkUIDetailViewController
@synthesize fNavigationBar, fPrintButton;

- (void)viewDidLoad {
    [super viewDidLoad];
    
    fSkUIView = (SkUIView_shell*)self.view;
    fSkUIView.fTitle = fNavigationBar.topItem;
    
    application_init();
    fWind = (SampleWindow*)create_sk_window(self.view);
    CGSize s = self.view.bounds.size;
    fWind->resize(s.width, s.height);
    [fSkUIView setSkWindow:(SkOSWindow*)fWind];
    
    [self initGestureRecognizers];
    [NSTimer scheduledTimerWithTimeInterval:0.001 target:self 
                                   selector:@selector(redraw) userInfo:nil 
                                    repeats:YES];
}

- (void)dealloc {
    [fNavigationBar release];
    [fPrintButton release];
    application_term();
    delete fWind;
    [super dealloc];
}

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
    controller.showsPageRange = YES;
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

#pragma mark -
#pragma mark Rotation support

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES; // Overriden to allow auto rotation for any direction
}

#pragma mark -
#pragma mark Managing the popover

- (void)showRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem {
    // Add the popover button to the left navigation item.
    [fNavigationBar.topItem setLeftBarButtonItem:barButtonItem animated:NO];
}


- (void)invalidateRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem {
    // Remove the popover button.
    [fNavigationBar.topItem setLeftBarButtonItem:nil animated:NO];
}

// Gestures
- (void)initGestureRecognizers {
    UITapGestureRecognizer *doubleTap = [[UITapGestureRecognizer alloc]
                                         initWithTarget:self 
                                         action:@selector(handleDoubleTapGesture:)];
    doubleTap.numberOfTapsRequired = 2;
    [self.view addGestureRecognizer:doubleTap];
    [doubleTap release];
    
    UIPanGestureRecognizer *pan = [[UIPanGestureRecognizer alloc]
                                   initWithTarget:self 
                                   action:@selector(handlePanGesture:)];
    [self.view addGestureRecognizer:pan];
    [pan release];
    
    UIPinchGestureRecognizer *pinch = [[UIPinchGestureRecognizer alloc]
                                       initWithTarget:self 
                                       action:@selector(handlePinchGesture:)];
    [self.view addGestureRecognizer:pinch];
    [pinch release];
    
    UISwipeGestureRecognizer *lswipe = [[UISwipeGestureRecognizer alloc]
                                       initWithTarget:self 
                                       action:@selector(handleSwipeGesture:)];
    lswipe.direction = UISwipeGestureRecognizerDirectionLeft;
    [self.view addGestureRecognizer:lswipe];
    [lswipe release];
    
    UISwipeGestureRecognizer *rswipe = [[UISwipeGestureRecognizer alloc]
                                       initWithTarget:self 
                                       action:@selector(handleSwipeGesture:)];
    //Swipe direction default to right
    [self.view addGestureRecognizer:rswipe];
    [rswipe release];
    
    UIRotationGestureRecognizer *rotation = [[UIRotationGestureRecognizer alloc]
                                       initWithTarget:self 
                                       action:@selector(handleRotationGesture:)];
    [self.view addGestureRecognizer:rotation];
    [rotation release];
}

- (void)handleDoubleTapGesture:(UIGestureRecognizer *)sender {
    [fSkUIView resetTransformations];
}

- (void)handlePanGesture:(UIPanGestureRecognizer *)sender {
    CGPoint translate = [sender translationInView:self.view];
    switch (sender.state) {
        case UIGestureRecognizerStateBegan:
            fInitialOffset = fSkUIView.fOffset;
            fInitialCenter = fSkUIView.fCenter;
            break;
            
        case UIGestureRecognizerStateChanged:
            fSkUIView.fOffset = CGPointMake(fInitialOffset.x + translate.x, 
                                            fInitialOffset.y + translate.y);
            fSkUIView.fCenter = CGPointMake(fInitialCenter.x - translate.x, 
                                            fInitialCenter.y - translate.y);
            break;
        case UIGestureRecognizerStateEnded:
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateFailed:
            break;
        default:
            break;
    }
}

- (void)handlePinchGesture:(UIPinchGestureRecognizer *)sender {
    switch (sender.state) {
        case UIGestureRecognizerStateBegan:
            fInitialScale = fSkUIView.fScale;
            break;
        case UIGestureRecognizerStateChanged:
            fSkUIView.fScale = fInitialScale * [sender scale];
            break;
        case UIGestureRecognizerStateEnded:
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateFailed:
            break;
        default:
            break;
    }
}

- (void)handleSwipeGesture:(UISwipeGestureRecognizer *)sender {
    if (sender.direction == UISwipeGestureRecognizerDirectionLeft) {
        fWind->previousSample(); 
    }
    else {
        fWind->nextSample(); 
    }
}

- (void)handleRotationGesture:(UIRotationGestureRecognizer *)sender {
    switch (sender.state) {
        case UIGestureRecognizerStateBegan:
            fInitialRotation = fSkUIView.fRotation;
            break;
        case UIGestureRecognizerStateChanged:
            fSkUIView.fRotation = fInitialRotation + [sender rotation] * 50.0;
            break;
        case UIGestureRecognizerStateEnded:
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateFailed:
            break;
        default:
            break;
    }
}

@end
