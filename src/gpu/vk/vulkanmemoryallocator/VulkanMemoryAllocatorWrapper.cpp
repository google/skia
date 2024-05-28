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

// On windows, vk_mem_alloc.h uses Windows SRWLock, which is normally included by vulkan.h, and so
// the relevant symbols/defines need to be included here.
#ifdef _WIN32
#include <windows.h>
#endif

// Work around https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/issues/312
// We should be able to remove this when we are able to update to the latest
// version (as of Oct 2023, blocked on version mismatch in G3)
#include <cstdio>

#include <vulkan/vulkan_core.h>
#include "src/gpu/vk/vulkanmemoryallocator/VulkanMemoryAllocatorWrapper.h"
