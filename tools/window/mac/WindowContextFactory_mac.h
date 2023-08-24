
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_mac_DEFINED
#define WindowContextFactory_mac_DEFINED

#include "tools/window/WindowContext.h"

#include <Cocoa/Cocoa.h>

#include <memory>

namespace skwindow {

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

struct MacWindowInfo {
    NSView*   fMainView;
};

#ifdef SK_VULKAN
inline std::unique_ptr<WindowContext> MakeVulkanForMac(const MacWindowInfo&, const DisplayParams&) {
    // No Vulkan support on Mac.
    return nullptr;
}
#endif

#ifdef SK_GL
std::unique_ptr<WindowContext> MakeRasterForMac(const MacWindowInfo&, const DisplayParams&);
std::unique_ptr<WindowContext> MakeGLForMac(const MacWindowInfo&, const DisplayParams&);
#endif

#ifdef SK_DAWN
#if defined(SK_GRAPHITE)
std::unique_ptr<WindowContext> MakeGraphiteDawnMetalForMac(const MacWindowInfo&, const DisplayParams&);
#endif
#endif

#ifdef SK_METAL
std::unique_ptr<WindowContext> MakeMetalForMac(const MacWindowInfo&, const DisplayParams&);
#if defined(SK_GRAPHITE)
std::unique_ptr<WindowContext> MakeGraphiteMetalForMac(const MacWindowInfo&, const DisplayParams&);
#endif
#endif

}  // namespace skwindow

#endif
