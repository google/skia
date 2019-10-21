/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/sk_app/ios/WindowContextFactory_ios.h"
#include "tools/sk_app/ios/Window_ios.h"

@interface WindowViewController : UIViewController

- (WindowViewController*)initWithWindow:(sk_app::Window_ios*)initWindow;

@end

///////////////////////////////////////////////////////////////////////////////

using sk_app::Window;

namespace sk_app {

Window_ios* Window_ios::gWindow = nullptr;

Window* Window::CreateNativeWindow(void*) {
    // already have a window
    if (Window_ios::MainWindow()) {
        return nullptr;
    }

    Window_ios* window = new Window_ios();
    if (!window->initWindow()) {
        delete window;
        return nullptr;
    }

    return window;
}

bool Window_ios::initWindow() {
    // we already have a window
    if (fWindow) {
        return true;
    }

    // Create a view controller to track certain events
    WindowViewController* viewController = [[WindowViewController alloc] initWithWindow:this];
    if (nil == viewController) {
        return false;
    }

    fWindow = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    if (nil == fWindow) {
        [viewController release];
        return false;
    }
    fWindow.backgroundColor = [UIColor whiteColor];

    viewController.view = nil;
    [fWindow setRootViewController:viewController];
    [fWindow makeKeyAndVisible];

    gWindow = this;

    return true;
}

void Window_ios::closeWindow() {
    if (nil != fWindow) {
        gWindow = nullptr;
        [fWindow release];
        fWindow = nil;
    }
}

bool Window_ios::attach(BackendType attachType) {
    this->initWindow();

    window_context_factory::IOSWindowInfo info;
    info.fWindow = this;
    info.fViewController = fWindow.rootViewController;
    switch (attachType) {
        case kRaster_BackendType:
            fWindowContext = MakeRasterForIOS(info, fRequestedDisplayParams);
            break;
#ifdef SK_METAL
        case kMetal_BackendType:
            fWindowContext = MakeMetalForIOS(info, fRequestedDisplayParams);
            break;
#endif
        case kNativeGL_BackendType:
        default:
            fWindowContext = MakeGLForIOS(info, fRequestedDisplayParams);
            break;
    }
    this->onBackendCreated();

    return (SkToBool(fWindowContext));
}

void Window_ios::PaintWindow() {
    gWindow->onPaint();
}

void Window_ios::onInval() {
    // TODO: send expose event
}

}   // namespace sk_app

///////////////////////////////////////////////////////////////////////////////

@implementation WindowViewController {
    sk_app::Window_ios* fWindow;
}

- (WindowViewController*)initWithWindow:(sk_app::Window_ios *)initWindow {
    fWindow = initWindow;

    return self;
}

- (void)viewDidLoad {
    // nothing yet
}

- (void)didReceiveMemoryWarning {
    // nothing yet
}

- (void)viewWillTransitionToSize:(CGSize)size
       withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator {
    // handle rotations here
}
@end

///////////////////////////////////////////////////////////////////////////////

@implementation MainView {
    sk_app::Window_ios* fWindow;
}

- (IBAction)panGestureAction:(UIGestureRecognizer*)sender {
    CGPoint location = [sender locationInView:self];
    switch (sender.state) {
        case UIGestureRecognizerStateBegan:
            fWindow->onMouse(location.x, location.y,
                             skui::InputState::kDown, skui::ModifierKey::kNone);
            break;
        case UIGestureRecognizerStateChanged:
            fWindow->onMouse(location.x, location.y,
                             skui::InputState::kMove, skui::ModifierKey::kNone);
            break;
        case UIGestureRecognizerStateEnded:
            fWindow->onMouse(location.x, location.y,
                             skui::InputState::kUp, skui::ModifierKey::kNone);
            break;
        case UIGestureRecognizerStateCancelled:
            fWindow->onMouse(location.x, location.y,
                             skui::InputState::kUp, skui::ModifierKey::kNone);
            break;
        default:
            break;
    }
}

- (IBAction)tapGestureAction:(UIGestureRecognizer*)sender {
    CGPoint location = [sender locationInView:self];
    switch (sender.state) {
        case UIGestureRecognizerStateEnded:
            fWindow->onMouse(location.x, location.y,
                             skui::InputState::kDown, skui::ModifierKey::kNone);
            fWindow->onMouse(location.x, location.y,
                             skui::InputState::kUp, skui::ModifierKey::kNone);
            break;
        default:
            break;
    }
}

- (IBAction)pinchGestureAction:(UIGestureRecognizer*)sender {
    CGPoint location = [sender locationInView:self];
    UIPinchGestureRecognizer* pinchGestureRecognizer = (UIPinchGestureRecognizer*) sender;
    float scale = pinchGestureRecognizer.scale;
    switch (sender.state) {
        case UIGestureRecognizerStateBegan:
            fWindow->onPinch(skui::InputState::kDown, scale, location.x, location.y);
            break;
        case UIGestureRecognizerStateChanged:
            fWindow->onPinch(skui::InputState::kMove, scale, location.x, location.y);
            break;
        case UIGestureRecognizerStateEnded:
            fWindow->onPinch(skui::InputState::kUp, scale, location.x, location.y);
            break;
        case UIGestureRecognizerStateCancelled:
            fWindow->onPinch(skui::InputState::kUp, scale, location.x, location.y);
            break;
        default:
            break;
    }
}

- (IBAction)swipeRightGestureAction:(UIGestureRecognizer*)sender {
    if (UIGestureRecognizerStateEnded == sender.state) {
        fWindow->onFling(skui::InputState::kRight);
    }
}

- (IBAction)swipeLeftGestureAction:(UIGestureRecognizer*)sender {
    if (UIGestureRecognizerStateEnded == sender.state) {
        fWindow->onFling(skui::InputState::kLeft);
    }
}

- (MainView*)initWithWindow:(sk_app::Window_ios *)initWindow {
    self = [super init];

    UIPanGestureRecognizer* panGestureRecognizer = [[UIPanGestureRecognizer alloc] init];
    panGestureRecognizer.maximumNumberOfTouches = 1;
    [panGestureRecognizer addTarget:self action:@selector(panGestureAction:)];
    [self addGestureRecognizer:panGestureRecognizer];

    UITapGestureRecognizer* tapGestureRecognizer = [[UITapGestureRecognizer alloc] init];
    [tapGestureRecognizer addTarget:self action:@selector(tapGestureAction:)];
    [self addGestureRecognizer:tapGestureRecognizer];

    UIPinchGestureRecognizer* pinchGestureRecognizer = [[UIPinchGestureRecognizer alloc] init];
    [pinchGestureRecognizer addTarget:self action:@selector(pinchGestureAction:)];
    [self addGestureRecognizer:pinchGestureRecognizer];

    UISwipeGestureRecognizer* swipeRightGestureRecognizer = [[UISwipeGestureRecognizer alloc] init];
    swipeRightGestureRecognizer.direction = UISwipeGestureRecognizerDirectionRight;
    [swipeRightGestureRecognizer addTarget:self action:@selector(swipeRightGestureAction:)];
    [self addGestureRecognizer:swipeRightGestureRecognizer];

    UISwipeGestureRecognizer* swipeLeftGestureRecognizer = [[UISwipeGestureRecognizer alloc] init];
    swipeLeftGestureRecognizer.direction = UISwipeGestureRecognizerDirectionLeft;
    [swipeLeftGestureRecognizer addTarget:self action:@selector(swipeLeftGestureAction:)];
    [self addGestureRecognizer:swipeLeftGestureRecognizer];

    // disable pan recognition until swipes fail
    [panGestureRecognizer requireGestureRecognizerToFail:swipeLeftGestureRecognizer];
    [panGestureRecognizer requireGestureRecognizerToFail:swipeRightGestureRecognizer];

    fWindow = initWindow;

    return self;
}

@end

