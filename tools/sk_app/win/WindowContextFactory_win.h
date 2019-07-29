
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef WindowContextFactory_win_DEFINED
#define WindowContextFactory_win_DEFINED

#include <Windows.h>

namespace sk_app {

class WindowContext;
struct DisplayParams;

namespace window_context_factory {

WindowContext* NewVulkanForWin(HWND, const DisplayParams&);

WindowContext* NewGLForWin(HWND, const DisplayParams&);

WindowContext* NewANGLEForWin(HWND, const DisplayParams&);

#ifdef SK_DAWN
WindowContext* NewDawnGLForWin(HWND, const DisplayParams&);
WindowContext* NewDawnD3D12ForWin(HWND, const DisplayParams&);
#endif

WindowContext* NewRasterForWin(HWND, const DisplayParams&);

}  // namespace window_context_factory

}  // namespace sk_app

#endif
