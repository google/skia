
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef VULKANTESTCONTEXT_ANDROID_DEFINED
#define VULKANTESTCONTEXT_ANDROID_DEFINED

#ifdef SK_VULKAN

#include "../VulkanTestContext.h"

struct ANativeWindow;

struct ContextPlatformData_android {
    ANativeWindow* fNativeWindow;
};

#endif // SK_VULKAN

#endif
