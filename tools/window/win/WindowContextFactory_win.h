
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_win_DEFINED
#define WindowContextFactory_win_DEFINED

#include "tools/sk_app/Window.h"

#include <Windows.h>

#include <memory>

namespace skwindow {

class WindowContext;
class DisplayParams;

#ifdef SK_VULKAN
std::unique_ptr<WindowContext> MakeVulkanForWin(HWND, std::unique_ptr<const DisplayParams>);
#if defined(SK_GRAPHITE)
std::unique_ptr<WindowContext> MakeGraphiteVulkanForWin(HWND, std::unique_ptr<const DisplayParams>);
#endif
#endif

#ifdef SK_GL
std::unique_ptr<WindowContext> MakeGLForWin(HWND, std::unique_ptr<const DisplayParams>);
#endif

#ifdef SK_ANGLE
std::unique_ptr<WindowContext> MakeANGLEForWin(HWND, std::unique_ptr<const DisplayParams>);
#endif

#ifdef SK_DIRECT3D
std::unique_ptr<WindowContext> MakeD3D12ForWin(HWND, std::unique_ptr<const DisplayParams>);
#endif

#ifdef SK_DAWN
#if defined(SK_GRAPHITE)
std::unique_ptr<WindowContext> MakeGraphiteDawnForWin(HWND,
                                                      std::unique_ptr<const DisplayParams>,
                                                      sk_app::Window::BackendType backendType);
#endif
#endif

std::unique_ptr<WindowContext> MakeRasterForWin(HWND, std::unique_ptr<const DisplayParams>);

}  // namespace skwindow

#endif
