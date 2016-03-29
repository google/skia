/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkMemory.h"

#include "GrVkGpu.h"
#include "GrVkUtil.h"

static bool get_valid_memory_type_index(VkPhysicalDeviceMemoryProperties physDevMemProps,
                                        uint32_t typeBits,
                                        VkMemoryPropertyFlags requestedMemFlags,
                                        uint32_t* typeIndex) {
    uint32_t checkBit = 1;
    for (uint32_t i = 0; i < 32; ++i) {
        if (typeBits & checkBit) {
            uint32_t supportedFlags = physDevMemProps.memoryTypes[i].propertyFlags &
                                      requestedMemFlags;
            if (supportedFlags == requestedMemFlags) {
                *typeIndex = i;
                return true;
            }
        }
        checkBit <<= 1;
    }
    return false;
}

static bool alloc_device_memory(const GrVkGpu* gpu,
                                VkMemoryRequirements* memReqs,
                                const VkMemoryPropertyFlags flags,
                                VkDeviceMemory* memory) {
    uint32_t typeIndex;
    if (!get_valid_memory_type_index(gpu->physicalDeviceMemoryProperties(),
                                     memReqs->memoryTypeBits,
                                     flags,
                                     &typeIndex)) {
        return false;
    }

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
        NULL,                                        // pNext
        memReqs->size,                               // allocationSize
        typeIndex,                                   // memoryTypeIndex
    };

    VkResult err = GR_VK_CALL(gpu->vkInterface(), AllocateMemory(gpu->device(),
                                                                 &allocInfo,
                                                                 nullptr,
                                                                 memory));
    if (err) {
        return false;
    }
    return true;
}

bool GrVkMemory::AllocAndBindBufferMemory(const GrVkGpu* gpu,
                                          VkBuffer buffer,
                                          const VkMemoryPropertyFlags flags,
                                          VkDeviceMemory* memory) {
    const GrVkInterface* iface = gpu->vkInterface();
    VkDevice device = gpu->device();

    VkMemoryRequirements memReqs;
    GR_VK_CALL(iface, GetBufferMemoryRequirements(device, buffer, &memReqs));


    if (!alloc_device_memory(gpu, &memReqs, flags, memory)) {
        return false;
    }

    // Bind Memory to queue
    VkResult err = GR_VK_CALL(iface, BindBufferMemory(device, buffer, *memory, 0));
    if (err) {
        GR_VK_CALL(iface, FreeMemory(device, *memory, nullptr));
        return false;
    }
    return true;
}

bool GrVkMemory::AllocAndBindImageMemory(const GrVkGpu* gpu,
                                         VkImage image,
                                         const VkMemoryPropertyFlags flags,
                                         VkDeviceMemory* memory) {
    const GrVkInterface* iface = gpu->vkInterface();
    VkDevice device = gpu->device();

    VkMemoryRequirements memReqs;
    GR_VK_CALL(iface, GetImageMemoryRequirements(device, image, &memReqs));

    if (!alloc_device_memory(gpu, &memReqs, flags, memory)) {
        return false;
    }

    // Bind Memory to queue
    VkResult err = GR_VK_CALL(iface, BindImageMemory(device, image, *memory, 0));
    if (err) {
        GR_VK_CALL(iface, FreeMemory(device, *memory, nullptr));
        return false;
    }
    return true;
}

VkPipelineStageFlags GrVkMemory::LayoutToPipelineStageFlags(const VkImageLayout layout) {
    if (VK_IMAGE_LAYOUT_GENERAL == layout) {
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    } else if (VK_IMAGE_LAYOUT_PREINITIALIZED == layout) {
        return VK_PIPELINE_STAGE_HOST_BIT;
    }

    SkASSERT(VK_IMAGE_LAYOUT_UNDEFINED == layout);
    return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}

VkAccessFlags GrVkMemory::LayoutToSrcAccessMask(const VkImageLayout layout) {
    // Currently we assume we will never being doing any explict shader writes (this doesn't include
    // color attachment or depth/stencil writes). So we will ignore the
    // VK_MEMORY_OUTPUT_SHADER_WRITE_BIT.

    // We can only directly access the host memory if we are in preinitialized or general layout,
    // and the image is linear.
    // TODO: Add check for linear here so we are not always adding host to general, and we should
    //       only be in preinitialized if we are linear
    VkAccessFlags flags = 0;;
    if (VK_IMAGE_LAYOUT_GENERAL == layout) {
        flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_TRANSFER_WRITE_BIT |
            VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_HOST_READ_BIT;
    } else if (VK_IMAGE_LAYOUT_PREINITIALIZED == layout) {
        flags = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_HOST_READ_BIT;
    } else if (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL == layout) {
        flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == layout) {
        flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == layout) {
        flags = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == layout) {
        flags = VK_ACCESS_TRANSFER_READ_BIT;
    } else if (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == layout) {
        flags = VK_ACCESS_SHADER_READ_BIT;
    }
    return flags;
}
