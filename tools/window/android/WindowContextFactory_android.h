/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_android_DEFINED
#define WindowContextFactory_android_DEFINED

#include <android/native_window_jni.h>

#include <memory>

#include "tools/sk_app/Window.h"

namespace skwindow {

class WindowContext;
class DisplayParams;

std::unique_ptr<WindowContext> MakeVulkanForAndroid(ANativeWindow*,
                                                    std::unique_ptr<const DisplayParams>);

std::unique_ptr<WindowContext> MakeGraphiteVulkanForAndroid(ANativeWindow*,
                                                            std::unique_ptr<const DisplayParams>);

std::unique_ptr<WindowContext> MakeGraphiteDawnForAndroid(ANativeWindow*,
                                                          std::unique_ptr<const DisplayParams>,
                                                          sk_app::Window::BackendType backendType);

std::unique_ptr<WindowContext> MakeGLForAndroid(ANativeWindow*,
                                                std::unique_ptr<const DisplayParams>);

std::unique_ptr<WindowContext> MakeRasterForAndroid(ANativeWindow*,
                                                    std::unique_ptr<const DisplayParams>);

}  // namespace skwindow

#endif
