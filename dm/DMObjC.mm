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

static void unhandled_exception_handler(NSException *exception)  {
    NSLog(@"CRASH: %@", exception);
    NSLog(@"Stack Trace: %@", [exception callStackSymbols]);
}

void SetObjectiveCExceptionHandler() {
    NSSetUncaughtExceptionHandler(&unhandled_exception_handler);
}

