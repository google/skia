
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_ios_DEFINED
#define WindowContextFactory_ios_DEFINED

#include "tools/sk_app/ios/Window_ios.h"

#import <UIKit/UIKit.h>

namespace sk_app {

class WindowContext;
struct DisplayParams;

namespace window_context_factory {

struct IOSWindowInfo {
    sk_app::Window_ios* fWindow;
    UIViewController*   fViewController;
};

inline WindowContext* NewVulkanForIOS(const IOSWindowInfo&, const DisplayParams&) {
    // No Vulkan support on iOS yet.
    return nullptr;
}

inline WindowContext* NewMetalForIOS(const IOSWindowInfo&, const DisplayParams&) {
    // No Metal support on iOS yet.
    return nullptr;
}

WindowContext* NewGLForIOS(const IOSWindowInfo&, const DisplayParams&);

WindowContext* NewRasterForIOS(const IOSWindowInfo&, const DisplayParams&);

}  // namespace window_context_factory

}  // namespace sk_app

#endif
