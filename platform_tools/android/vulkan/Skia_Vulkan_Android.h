/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Skia_Vulkan_Android_DEFINED
#define Skia_Vulkan_Android_DEFINED

#include "SkTypes.h"

#if !defined(SK_BUILD_FOR_ANDROID)
#error "Must be building for android to use this header"
#endif
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
#  define VK_USE_PLATFORM_ANDROID_KHR
#endif

#include <vulkan/vulkan.h> // IWYU pragma: export

#endif



