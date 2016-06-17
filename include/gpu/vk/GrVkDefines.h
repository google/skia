
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkDefines_DEFINED
#define GrVkDefines_DEFINED

#if defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_WIN32)
#   define VK_USE_PLATFORM_WIN32_KHR
#elif defined(SK_BUILD_FOR_ANDROID)
#   define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(SK_BUILD_FOR_UNIX)
#   define VK_USE_PLATFORM_XLIB_KHR
#endif

#if defined(Bool) || defined(Status) || defined(True) || defined(False)
#   pragma error "Macros unexpectedly defined."
#endif

#include <vulkan/vulkan.h>


// Xlib.h may define these macros with common names (Grrr)
#ifdef Bool
#   undef Bool
#endif
#ifdef Status
#   undef Status
#endif
#ifdef True
#   undef True
#endif
#ifdef False
#   undef False
#endif

#endif
