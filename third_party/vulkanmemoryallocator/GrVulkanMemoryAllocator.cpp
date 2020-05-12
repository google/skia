/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// We use our own functions pointers
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

// vma_mem_alloc.h will use VMA_NULLABLE and VMA_NOT_NULL as clang nullability attribtues but does a
// poor job at using them everywhere. Thus it causes lots of clang compiler warnings. We just
// disable them here by defining them to be nothing.
#define VMA_NULLABLE
#define VMA_NOT_NULL

#define VMA_IMPLEMENTATION
#include <vulkan/vulkan_core.h>
#include "GrVulkanMemoryAllocator.h"

