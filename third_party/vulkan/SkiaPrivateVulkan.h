/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkiaPrivateVulkan_DEFINED
#define SkiaPrivateVulkan_DEFINED

#ifdef VULKAN_CORE_H_
#error "Skia's private vulkan header must be included before any other vulkan header."
#endif

#include "vulkan/vulkan_core.h"
//#include "vulkan/vulkan_android.h"

#if defined(ANDROID) || defined(__ANDROID__)
#ifdef VULKAN_ANDROID_H_
#error "Skia's private vulkan android header must be included before any other vulkan header."
#endif
// This is needed to get android extensions for external memory
#include "vulkan/vulkan_android.h"
#endif

#endif
