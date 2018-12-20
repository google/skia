/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Workaround to make sure we align non-coherent memory to nonCoherentAtomSize.
#define VMA_DEBUG_ALIGNMENT 256

// We use our own functions pointers
#define VMA_STATIC_VULKAN_FUNCTIONS 0

#define VMA_IMPLEMENTATION
#include <vulkan/vulkan_core.h>
#include "GrVulkanMemoryAllocator.h"

