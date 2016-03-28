/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkMemory_DEFINED
#define GrVkMemory_DEFINED

#include "vk/GrVkDefines.h"

class GrVkGpu;

namespace GrVkMemory {
    /**
    * Allocates vulkan device memory and binds it to the gpu's device for the given object.
    * Returns true of allocation succeeded.
    */
    bool AllocAndBindBufferMemory(const GrVkGpu* gpu,
                                  VkBuffer buffer,
                                  const VkMemoryPropertyFlags flags,
                                  VkDeviceMemory* memory);

    bool AllocAndBindImageMemory(const GrVkGpu* gpu,
                                 VkImage image,
                                 const VkMemoryPropertyFlags flags,
                                 VkDeviceMemory* memory);

    VkPipelineStageFlags LayoutToPipelineStageFlags(const VkImageLayout layout);

    VkAccessFlags LayoutToSrcAccessMask(const VkImageLayout layout);
}

#endif
