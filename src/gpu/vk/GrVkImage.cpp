/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkGpu.h"
#include "GrVkImage.h"
#include "GrVkMemory.h"
#include "GrVkUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

VkImageAspectFlags vk_format_to_aspect_flags(VkFormat format) {
    switch (format) {
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D24_UNORM_S8_UINT: // fallthrough
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            SkASSERT(GrVkFormatToPixelConfig(format, nullptr));
            return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

void GrVkImage::setImageLayout(const GrVkGpu* gpu, VkImageLayout newLayout,
                               VkAccessFlags srcAccessMask,
                               VkAccessFlags dstAccessMask,
                               VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask,
                               bool byRegion) {
    SkASSERT(VK_IMAGE_LAYOUT_UNDEFINED != newLayout &&
             VK_IMAGE_LAYOUT_PREINITIALIZED != newLayout);
    // Is this reasonable? Could someone want to keep the same layout but use the masks to force
    // a barrier on certain things?
    if (newLayout == fCurrentLayout) {
        return;
    }
    VkImageAspectFlags aspectFlags = vk_format_to_aspect_flags(fResource->fFormat);
    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,          // sType
        NULL,                                            // pNext
        srcAccessMask,                                   // outputMask
        dstAccessMask,                                   // inputMask
        fCurrentLayout,                                  // oldLayout
        newLayout,                                       // newLayout
        VK_QUEUE_FAMILY_IGNORED,                         // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                         // dstQueueFamilyIndex
        fResource->fImage,                               // image
        { aspectFlags, 0, fResource->fLevelCount, 0, 1 } // subresourceRange
    };

    // TODO: restrict to area of image we're interested in
    gpu->addImageMemoryBarrier(srcStageMask, dstStageMask, byRegion, &imageMemoryBarrier);

    fCurrentLayout = newLayout;
}

const GrVkImage::Resource* GrVkImage::CreateResource(const GrVkGpu* gpu,
                                                     const ImageDesc& imageDesc) {
    VkImage image = 0;
    VkDeviceMemory alloc;

    VkImageLayout initialLayout = (VK_IMAGE_TILING_LINEAR == imageDesc.fImageTiling)
        ? VK_IMAGE_LAYOUT_PREINITIALIZED
        : VK_IMAGE_LAYOUT_UNDEFINED;

    // Create Image
    VkSampleCountFlagBits vkSamples;
    if (!GrSampleCountToVkSampleCount(imageDesc.fSamples, &vkSamples)) {
        return nullptr;
    }

    SkASSERT(VK_IMAGE_TILING_OPTIMAL == imageDesc.fImageTiling ||
             VK_SAMPLE_COUNT_1_BIT == vkSamples);

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        NULL,                                        // pNext
        0,                                           // VkImageCreateFlags
        imageDesc.fImageType,                        // VkImageType
        imageDesc.fFormat,                           // VkFormat
        { imageDesc.fWidth, imageDesc.fHeight, 1 },  // VkExtent3D
        imageDesc.fLevels,                           // mipLevels
        1,                                           // arrayLayers
        vkSamples,                                   // samples
        imageDesc.fImageTiling,                      // VkImageTiling
        imageDesc.fUsageFlags,                       // VkImageUsageFlags
        VK_SHARING_MODE_EXCLUSIVE,                   // VkSharingMode
        0,                                           // queueFamilyCount
        0,                                           // pQueueFamilyIndices
        initialLayout                                // initialLayout
    };

    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), CreateImage(gpu->device(), &imageCreateInfo, nullptr, &image));

    if (!GrVkMemory::AllocAndBindImageMemory(gpu, image, imageDesc.fMemProps, &alloc)) {
        VK_CALL(gpu, DestroyImage(gpu->device(), image, nullptr));
        return nullptr;
    }

    GrVkImage::Resource::Flags flags =
        (VK_IMAGE_TILING_LINEAR == imageDesc.fImageTiling) ? Resource::kLinearTiling_Flag
                                                           : Resource::kNo_Flags;

    return (new GrVkImage::Resource(image, alloc, imageDesc.fFormat, imageDesc.fLevels, flags));
}

GrVkImage::~GrVkImage() {
    // should have been released or abandoned first
    SkASSERT(!fResource);
}

void GrVkImage::releaseImage(const GrVkGpu* gpu) {
    if (fResource) {
        fResource->unref(gpu);
        fResource = nullptr;
    }
}

void GrVkImage::abandonImage() {
    if (fResource) {
        fResource->unrefAndAbandon();
        fResource = nullptr;
    }
}

void GrVkImage::Resource::freeGPUData(const GrVkGpu* gpu) const {
    VK_CALL(gpu, DestroyImage(gpu->device(), fImage, nullptr));
    VK_CALL(gpu, FreeMemory(gpu->device(), fAlloc, nullptr));
}

void GrVkImage::BorrowedResource::freeGPUData(const GrVkGpu* gpu) const {
}
