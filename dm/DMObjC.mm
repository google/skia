/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC)
#import <Cocoa/Cocoa.h>
#elif defined(SK_BUILD_FOR_IOS)
#import <UIKit/UIKit.h>
#endif

int dmMain(int argc, char** argv);

void unhandledExceptionHandler(NSException *exception)  {
    NSLog(@"CRASH: %@", exception);
    NSLog(@"Stack Trace: %@", [exception callStackSymbols]);
}

int main(int argc, char** argv) {
    NSSetUncaughtExceptionHandler(&unhandledExceptionHandler);

    return dmMain(argc, argv);
}

