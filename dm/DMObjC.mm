/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "dm/DMObjC.h"
#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC)
#import <Cocoa/Cocoa.h>
#elif defined(SK_BUILD_FOR_IOS)
#import <UIKit/UIKit.h>
#endif

int dmMain(int argc, char** argv);

static void unhandled_exception_handler(NSException *exception)  {
    NSLog(@"CRASH: %@", exception);
    NSLog(@"Stack Trace: %@", [exception callStackSymbols]);
}

void SetObjectiveCExceptionHandler() {
    NSSetUncaughtExceptionHandler(&unhandled_exception_handler);
}

int main(int argc, char** argv) {
    @try {
        return dmMain(argc, argv);
    } @catch (NSException* exception) {
        SkDebugf("Got one.\n");
        unhandled_exception_handler(exception);
    }
}
