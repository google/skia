
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_ios_DEFINED
#define WindowContextFactory_ios_DEFINED

#include "SDL.h"

#include "tools/sk_app/WindowContext.h"

#include <memory>

namespace sk_app {

struct DisplayParams;

namespace window_context_factory {

struct IOSWindowInfo {
    SDL_Window*   fWindow;
    SDL_GLContext fGLContext;
};

inline std::unique_ptr<WindowContext> MakeVulkanForIOS(const IOSWindowInfo&, const DisplayParams&) {
    // No Vulkan support on iOS.
    return nullptr;
}

std::unique_ptr<WindowContext> MakeGLForIOS(const IOSWindowInfo&, const DisplayParams&);

std::unique_ptr<WindowContext> MakeRasterForIOS(const IOSWindowInfo&, const DisplayParams&);

}  // namespace window_context_factory

}  // namespace sk_app

#endif
