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
                               VkAccessFlags dstAccessMask,
                               VkPipelineStageFlags dstStageMask,
                               bool byRegion) {
    SkASSERT(VK_IMAGE_LAYOUT_UNDEFINED != newLayout &&
             VK_IMAGE_LAYOUT_PREINITIALIZED != newLayout);
    VkImageLayout currentLayout = this->currentLayout();

    // If the old and new layout are the same and the layout is a read only layout, there is no need
    // to put in a barrier.
    if (newLayout == currentLayout &&
        (VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL == currentLayout ||
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == currentLayout ||
         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == currentLayout)) {
        return;
    }

    VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(currentLayout);
    VkPipelineStageFlags srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(currentLayout);

    VkImageAspectFlags aspectFlags = vk_format_to_aspect_flags(fInfo.fFormat);
    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,          // sType
        nullptr,                                         // pNext
        srcAccessMask,                                   // outputMask
        dstAccessMask,                                   // inputMask
        currentLayout,                                   // oldLayout
        newLayout,                                       // newLayout
        VK_QUEUE_FAMILY_IGNORED,                         // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                         // dstQueueFamilyIndex
        fInfo.fImage,                                    // image
        { aspectFlags, 0, fInfo.fLevelCount, 0, 1 }      // subresourceRange
    };

    gpu->addImageMemoryBarrier(srcStageMask, dstStageMask, byRegion, &imageMemoryBarrier);

    fInfo.fImageLayout = newLayout;
}

bool GrVkImage::InitImageInfo(const GrVkGpu* gpu, const ImageDesc& imageDesc, GrVkImageInfo* info) {
    if (0 == imageDesc.fWidth || 0 == imageDesc.fHeight) {
        return false;
    }
    VkImage image = 0;
    GrVkAlloc alloc;

    bool isLinear = VK_IMAGE_TILING_LINEAR == imageDesc.fImageTiling;
    VkImageLayout initialLayout = isLinear ? VK_IMAGE_LAYOUT_PREINITIALIZED
                                           : VK_IMAGE_LAYOUT_UNDEFINED;

    // Create Image
    VkSampleCountFlagBits vkSamples;
    if (!GrSampleCountToVkSampleCount(imageDesc.fSamples, &vkSamples)) {
        return false;
    }

    SkASSERT(VK_IMAGE_TILING_OPTIMAL == imageDesc.fImageTiling ||
             VK_SAMPLE_COUNT_1_BIT == vkSamples);

    // sRGB format images may need to be aliased to linear for various reasons (legacy mode):
    VkImageCreateFlags createFlags = GrVkFormatIsSRGB(imageDesc.fFormat, nullptr)
        ? VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT : 0;

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        NULL,                                        // pNext
        createFlags,                                 // VkImageCreateFlags
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

    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), CreateImage(gpu->device(), &imageCreateInfo, nullptr,
                                                        &image));

    if (!GrVkMemory::AllocAndBindImageMemory(gpu, image, isLinear, &alloc)) {
        VK_CALL(gpu, DestroyImage(gpu->device(), image, nullptr));
        return false;
    }

    info->fImage = image;
    info->fAlloc = alloc;
    info->fImageTiling = imageDesc.fImageTiling;
    info->fImageLayout = initialLayout;
    info->fFormat = imageDesc.fFormat;
    info->fLevelCount = imageDesc.fLevels;
    return true;
}

void GrVkImage::DestroyImageInfo(const GrVkGpu* gpu, GrVkImageInfo* info) {
    VK_CALL(gpu, DestroyImage(gpu->device(), info->fImage, nullptr));
    bool isLinear = VK_IMAGE_TILING_LINEAR == info->fImageTiling;
    GrVkMemory::FreeImageMemory(gpu, isLinear, info->fAlloc);
}

void GrVkImage::setNewResource(VkImage image, const GrVkAlloc& alloc, VkImageTiling tiling) {
    fResource = new Resource(image, alloc, tiling);
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
    bool isLinear = (VK_IMAGE_TILING_LINEAR == fImageTiling);
    GrVkMemory::FreeImageMemory(gpu, isLinear, fAlloc);
}

void GrVkImage::BorrowedResource::freeGPUData(const GrVkGpu* gpu) const {
}
