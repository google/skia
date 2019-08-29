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
    // Extract argc and argv from NSProcessInfo
    NSArray *arguments = [[NSProcessInfo processInfo] arguments];
    int argc = arguments.count;
    char** argv = (char **)malloc((argc+1) * sizeof(char *));
    int i = 0;
    for (NSString* string in arguments) {
        size_t bufferSize = (string.length+1) * sizeof(char);
        argv[i] = (char*)malloc(bufferSize);
        [string getCString:argv[i]
                 maxLength:bufferSize
                  encoding:NSUTF8StringEncoding];
        ++i;
    }
    argv[i] = NULL;

    Application* app = Application::Create(argc, argv, nullptr);

    // Free the memory we used for argc and argv
    for (i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);

    sk_app::Window_ios* mainWindow = sk_app::Window_ios::MainWindow();
    if (!mainWindow) {
        return;
    }
    self.window = mainWindow->uiWindow();

    // take over the main event loop
    bool done = false;
    while (!done) {
        // TODO: consider using a dispatch queue or CADisplayLink instead of this
        const CFTimeInterval kSeconds = 0.000002;
        CFRunLoopRunResult result;
        do {
            result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, kSeconds, TRUE);
        } while (result == kCFRunLoopRunHandledSource);

        // TODO: is this the right approach for iOS?
        // Rather than depending on an iOS event to drive this, we treat our window
        // invalidation flag as a separate event stream. Window::onPaint() will clear
        // the invalidation flag, effectively removing it from the stream.
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
    /* Give over control to run loop, AppDelegate will handle most things from here */
    @autoreleasepool {
        UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }

    return EXIT_SUCCESS;
}
