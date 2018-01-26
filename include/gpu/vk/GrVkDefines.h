
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkDefines_DEFINED
#define GrVkDefines_DEFINED

#ifdef SK_VULKAN

#ifdef SK_VULKAN_HEADER
#include SK_VULKAN_HEADER
#else
// This is deprecated and all clients should define their own custum header shim that sets up
// defines and includes the vulkan.h header. Then they should define SK_VULKAN_HEADER or set the
// skia_vulkan_header in gn to point to their custom header.
#  if defined(SK_BUILD_FOR_WIN)
#     if !defined(VK_USE_PLATFORM_WIN32_KHR)
#        define VK_USE_PLATFORM_WIN32_KHR
#     endif
#  elif defined(SK_BUILD_FOR_ANDROID)
#     if !defined(VK_USE_PLATFORM_ANDROID_KHR)
#        define VK_USE_PLATFORM_ANDROID_KHR
#     endif
#  elif defined(SK_BUILD_FOR_UNIX)
#     if defined(__Fuchsia__)
#       if !defined(VK_USE_PLATFORM_MAGMA_KHR)
#         define VK_USE_PLATFORM_MAGMA_KHR
#       endif
#     else
#       if !defined(VK_USE_PLATFORM_XCB_KHR)
#          define VK_USE_PLATFORM_XCB_KHR
#       endif
#     endif
#  endif

// We create our own function table and never directly call any functions via vk*(). So no need to
// include the prototype functions.
#  if !defined(VK_NO_PROTOTYPES) && !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
#    define VK_NO_PROTOTYPES
#  endif

#  include <vulkan/vulkan.h>
#endif // SK_VULKAN_HEADER

#define SKIA_REQUIRED_VULKAN_HEADER_VERSION 17
#if VK_HEADER_VERSION < SKIA_REQUIRED_VULKAN_HEADER_VERSION
#error "Vulkan header version is too low"
#endif

#endif

#endif
