
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_mac_DEFINED
#define WindowContextFactory_mac_DEFINED

#include "tools/sk_app/WindowContext.h"

#include <Cocoa/Cocoa.h>

#include <memory>

namespace sk_app {

struct DisplayParams;

static inline CGFloat GetBackingScaleFactor(NSView* view) {
    #ifdef SK_BUILD_FOR_IOS
    UIScreen* screen = view.window.screen ?: [UIScreen mainScreen];
    return screen.nativeScale;
    #else
    NSScreen* screen = view.window.screen ?: [NSScreen mainScreen];
    return screen.backingScaleFactor;
    #endif
}

namespace window_context_factory {

struct MacWindowInfo {
    NSView*   fMainView;
};

inline std::unique_ptr<WindowContext> MakeVulkanForMac(const MacWindowInfo&, const DisplayParams&) {
    // No Vulkan support on Mac.
    return nullptr;
}

std::unique_ptr<WindowContext> MakeGLForMac(const MacWindowInfo&, const DisplayParams&);

#ifdef SK_DAWN
std::unique_ptr<WindowContext> MakeDawnMTLForMac(const MacWindowInfo&, const DisplayParams&);
#endif

std::unique_ptr<WindowContext> MakeRasterForMac(const MacWindowInfo&, const DisplayParams&);
#ifdef SK_METAL
std::unique_ptr<WindowContext> MakeMetalForMac(const MacWindowInfo&, const DisplayParams&);
#endif

}  // namespace window_context_factory

}  // namespace sk_app

#endif
