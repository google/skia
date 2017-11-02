
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkDefines_DEFINED
#define GrVkDefines_DEFINED

#ifdef SK_VULKAN

#if defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_WIN32)
#   if !defined(VK_USE_PLATFORM_WIN32_KHR)
#      define VK_USE_PLATFORM_WIN32_KHR
#   endif
#elif defined(SK_BUILD_FOR_ANDROID)
#   if !defined(VK_USE_PLATFORM_ANDROID_KHR)
#      define VK_USE_PLATFORM_ANDROID_KHR
#   endif
#elif defined(SK_BUILD_FOR_UNIX)
#   if defined(__Fuchsia__)
#     if !defined(VK_USE_PLATFORM_MAGMA_KHR)
#       define VK_USE_PLATFORM_MAGMA_KHR
#     endif
#   else
#     if !defined(VK_USE_PLATFORM_XCB_KHR)
#        define VK_USE_PLATFORM_XCB_KHR
#     endif
#   endif
#endif

#include <vulkan/vulkan.h>

#define SKIA_REQUIRED_VULKAN_HEADER_VERSION 17
#if VK_HEADER_VERSION < SKIA_REQUIRED_VULKAN_HEADER_VERSION
#error "Vulkan header version is too low"
#endif

#endif

#endif
