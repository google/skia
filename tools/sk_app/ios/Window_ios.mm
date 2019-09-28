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

    // Create a delegate to track certain events
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

@end

///////////////////////////////////////////////////////////////////////////////

@implementation MainView {
    sk_app::Window_ios* fWindow;
}

- (MainView*)initWithWindow:(sk_app::Window_ios *)initWindow {
    self = [super init];

    fWindow = initWindow;

    return self;
}

@end

