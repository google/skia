/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MacWindowInfo_DEFINED
#define MacWindowInfo_DEFINED

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_IOS)
#error "This is Mac-only code"
#endif

#include <Cocoa/Cocoa.h>

namespace skwindow {

struct MacWindowInfo {
    NSView* fMainView;
};

static inline CGFloat GetBackingScaleFactor(NSView* view) {
    NSScreen* screen = view.window.screen ?: [NSScreen mainScreen];
    return screen.backingScaleFactor;
}

}  // namespace skwindow

#endif
