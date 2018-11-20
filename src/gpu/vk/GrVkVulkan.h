/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkVulkan_DEFINED
#define GrVkVulkan_DEFINED

#include "SkTypes.h"

#ifdef VULKAN_CORE_H_
#error "Skia's private vulkan header must be included before any other vulkan header."
#endif

#include "../../../third_party/vulkan/vulkan/vulkan_core.h"

#ifdef SK_BUILD_FOR_ANDROID
#ifdef VULKAN_ANDROID_H_
#error "Skia's private vulkan android header must be included before any other vulkan header."
#endif
// This is needed to get android extensions for external memory
#include "../../../third_party/vulkan/vulkan/vulkan_android.h"
#endif

#endif
