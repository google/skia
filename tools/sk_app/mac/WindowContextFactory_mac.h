
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_mac_DEFINED
#define WindowContextFactory_mac_DEFINED

#include "SDL.h"

namespace sk_app {

class WindowContext;
struct DisplayParams;

namespace window_context_factory {

struct MacWindowInfo {
    SDL_Window*  fWindow;
};

inline WindowContext* NewVulkanForMac(const MacWindowInfo&, const DisplayParams&) {
    // No Vulkan support on Mac.
    return nullptr;
}

WindowContext* NewGLForMac(const MacWindowInfo&, const DisplayParams&);

WindowContext* NewRasterForMac(const MacWindowInfo&, const DisplayParams&);

}  // namespace window_context_factory

}  // namespace sk_app

#endif
