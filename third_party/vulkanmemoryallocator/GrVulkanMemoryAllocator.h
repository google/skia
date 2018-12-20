/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// We use this header to include vk_mem_alloc.h to make sure we always include GrVkDefines.h first.
// We need to do this so that the corect defines are setup before we include vulkan.h inside of
// vk_mem_alloc.h

#ifndef GrVulkanMemoryAllocator_DEFINED
#define GrVulkanMemoryAllocator_DEFINED

// We only ever include this from src files which have already included vulkan.
#ifndef VULKAN_CORE_H_
#error "vulkan_core.h has not been included before trying to include the GrVulkanMemoryAllocator"
#endif
#include "include/vk_mem_alloc.h"

#endif
