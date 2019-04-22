/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVulkanDefines_DEFINED
#define GrVulkanDefines_DEFINED

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_WIN)
#   if !defined(VK_USE_PLATFORM_WIN32_KHR)
#      define VK_USE_PLATFORM_WIN32_KHR
#   endif
#elif defined(SK_BUILD_FOR_ANDROID)
#   if !defined(VK_USE_PLATFORM_ANDROID_KHR)
#      define VK_USE_PLATFORM_ANDROID_KHR
#   endif
#elif defined(SK_BUILD_FOR_UNIX)
#   if !defined(VK_USE_PLATFORM_XCB_KHR)
#      define VK_USE_PLATFORM_XCB_KHR
#   endif
#elif defined(SK_BUILD_FOR_MAC)
#   if !defined(VK_USE_PLATFORM_MACOS_MVK)
#      define VK_USE_PLATFORM_MACOS_MVK
#   endif
#elif defined(SK_BUILD_FOR_IOS)
#   if !defined(VK_USE_PLATFORM_IOS_MVK)
#      define VK_USE_PLATFORM_IOS_MVK
#   endif
#endif

// We create our own function table and never directly call any functions via vk*(). So no
// need to include the prototype functions. We do them for molten vk however.
#if !defined(SK_MOLTENVK) && !defined(VK_NO_PROTOTYPES)
    #define VK_NO_PROTOTYPES
#endif

#include <vulkan/vulkan.h> // IWYU pragma: export

#endif
