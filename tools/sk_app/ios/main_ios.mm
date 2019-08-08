/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkTypes.h"
#include "include/private/SkTHash.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/ios/Window_ios.h"
#include "tools/timer/Timer.h"

#import <UIKit/UIKit.h>

using sk_app::Application;

static int gArgc;
static char** gArgv;

////////////////////////////////////////////////////////////////////

@interface AppDelegate : UIResponder<UIApplicationDelegate>

@property (nonatomic, assign) BOOL done;
@property (strong, nonatomic) UIWindow *window;

@end

@implementation AppDelegate

@synthesize done = _done;
@synthesize window = _window;

- (void)applicationWillTerminate:(UIApplication *)sender {
    _done = TRUE;
}

- (void)launchApp {
    //*** maybe extract argc and argv from NSProcessInfo here?

    Application* app = Application::Create(gArgc, gArgv, nullptr);

    sk_app::Window_ios* mainWindow = sk_app::Window_ios::MainWindow();
    if (!mainWindow) {
        return;
    }
    self.window = mainWindow->uiWindow();

    // do stuff
    bool done = false;
    while (!done) {
        //*** consider using a dispatch queue instead of this
        //*** or CADisplayLink
        const CFTimeInterval kSeconds = 0.000002;
        CFRunLoopRunResult result;
        do {
            result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, kSeconds, TRUE);
        } while (result == kCFRunLoopRunHandledSource);

//*** not sure if this the right approach here
//        // Rather than depending on a Mac event to drive this, we treat our window
//        // invalidation flag as a separate event stream. Window::onPaint() will clear
//        // the invalidation flag, effectively removing it from the stream.
        sk_app::Window_ios::PaintWindow();

        app->onIdle();
    }
    delete app;
}

- (BOOL)application:(UIApplication *)application
        didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // let the system event loop run once, then launch into our main loop
    [self performSelector:@selector(launchApp) withObject:nil afterDelay:0.0];

    return YES;
}

@end

///////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
    int i;

    //*** Skia-fy this
    gArgc = argc;
    gArgv = (char **)malloc((argc+1) * sizeof(char *));
    for (i = 0; i < argc; i++) {
        gArgv[i] = (char*)malloc( (strlen(argv[i])+1) * sizeof(char));
        strcpy(gArgv[i], argv[i]);
    }
    gArgv[i] = NULL;

    /* Give over control to run loop, AppDelegate will handle most things from here */
    @autoreleasepool {
        UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }

    /* free the memory we used to hold copies of argc and argv */
    for (i = 0; i < gArgc; i++) {
        free(gArgv[i]);
    }
    free(gArgv);

    return EXIT_SUCCESS;
}
