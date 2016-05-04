
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef VULKANTESTCONTEXT_WIN_DEFINED
#define VULKANTESTCONTEXT_WIN_DEFINED

#ifdef SK_VULKAN

#include <windows.h>
#include "../VulkanTestContext.h"

// for Windows
struct ContextPlatformData_win {
    HINSTANCE fHInstance;
    HWND      fHWnd;
};

#endif // SK_VULKAN

#endif
