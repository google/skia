/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import <UIKit/UIKit.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "tools/ios_utils.h"

#if __has_feature(objc_arc)
#error "File should not be compiled with ARC."
#endif

namespace {
static int gArgc;
static char** gArgv;
}  // namespace

@interface SkiaTestAppDelegate : UIResponder <UIApplicationDelegate>
@property(nonatomic, retain) UIWindow* window;
@end

@implementation SkiaTestAppDelegate
@synthesize window = _window;

- (BOOL)application:(UIApplication*)application
        didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    application.idleTimerDisabled = YES;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
#pragma clang diagnostic pop
    UIViewController* vc = [[[UIViewController alloc] init] autorelease];
    vc.view.backgroundColor = [UIColor blackColor];
    self.window.rootViewController = vc;
    [self.window makeKeyAndVisible];

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 8 * 1024 * 1024);
    pthread_t thread;
    pthread_create(
            &thread,
            &attr,
            [](void*) -> void* {
                int rc = 0;
                @autoreleasepool {
                    rc = skia_ios_main(gArgc, gArgv);
                }
                fflush(stdout);
                fflush(stderr);
                _exit(rc);
                return nullptr;
            },
            nullptr);
    pthread_detach(thread);
    pthread_attr_destroy(&attr);
    return YES;
}

@end

int main(int argc, char** argv) {
    gArgc = argc;
    gArgv = argv;
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, @"SkiaTestAppDelegate");
    }
}
