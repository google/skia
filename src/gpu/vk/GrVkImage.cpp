/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImage.h"
#include "src/gpu/vk/GrVkMemory.h"
#include "src/gpu/vk/GrVkTexture.h"
#include "src/gpu/vk/GrVkUtil.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

VkPipelineStageFlags GrVkImage::LayoutToPipelineSrcStageFlags(const VkImageLayout layout) {
    if (VK_IMAGE_LAYOUT_GENERAL == layout) {
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else if (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    } else if (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == layout) {
        return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (VK_IMAGE_LAYOUT_PREINITIALIZED == layout) {
        return VK_PIPELINE_STAGE_HOST_BIT;
    } else if (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR == layout) {
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    SkASSERT(VK_IMAGE_LAYOUT_UNDEFINED == layout);
    return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}

VkAccessFlags GrVkImage::LayoutToSrcAccessMask(const VkImageLayout layout) {
    // Currently we assume we will never being doing any explict shader writes (this doesn't include
    // color attachment or depth/stencil writes). So we will ignore the
    // VK_MEMORY_OUTPUT_SHADER_WRITE_BIT.

    // We can only directly access the host memory if we are in preinitialized or general layout,
    // and the image is linear.
    // TODO: Add check for linear here so we are not always adding host to general, and we should
    //       only be in preinitialized if we are linear
    VkAccessFlags flags = 0;
    if (VK_IMAGE_LAYOUT_GENERAL == layout) {
        flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                VK_ACCESS_TRANSFER_WRITE_BIT |
                VK_ACCESS_TRANSFER_READ_BIT |
                VK_ACCESS_SHADER_READ_BIT |
                VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_HOST_READ_BIT;
    } else if (VK_IMAGE_LAYOUT_PREINITIALIZED == layout) {
        flags = VK_ACCESS_HOST_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL == layout) {
        flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    } else if (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == layout) {
        flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL == layout) {
        flags = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == layout ||
               VK_IMAGE_LAYOUT_PRESENT_SRC_KHR == layout) {
        // There are no writes that need to be made available
        flags = 0;
    }
    return flags;
}

VkImageAspectFlags vk_format_to_aspect_flags(VkFormat format) {
    switch (format) {
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D24_UNORM_S8_UINT: // fallthrough
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

void GrVkImage::setImageLayout(const GrVkGpu* gpu, VkImageLayout newLayout,
                               VkAccessFlags dstAccessMask,
                               VkPipelineStageFlags dstStageMask,
                               bool byRegion, bool releaseFamilyQueue) {
    SkASSERT(!gpu->isDeviceLost());
    SkASSERT(VK_IMAGE_LAYOUT_UNDEFINED != newLayout &&
             VK_IMAGE_LAYOUT_PREINITIALIZED != newLayout);
    VkImageLayout currentLayout = this->currentLayout();

    if (releaseFamilyQueue && fInfo.fCurrentQueueFamily == fInitialQueueFamily &&
        newLayout == currentLayout) {
        // We never transfered the image to this queue and we are releasing it so don't do anything.
        return;
    }

    // If the old and new layout are the same and the layout is a read only layout, there is no need
    // to put in a barrier unless we also need to switch queues.
    if (newLayout == currentLayout && !releaseFamilyQueue &&
        (fInfo.fCurrentQueueFamily == VK_QUEUE_FAMILY_IGNORED ||
         fInfo.fCurrentQueueFamily == gpu->queueIndex()) &&
        (VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL == currentLayout ||
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == currentLayout ||
         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == currentLayout)) {
        return;
    }

    VkAccessFlags srcAccessMask = GrVkImage::LayoutToSrcAccessMask(currentLayout);
    VkPipelineStageFlags srcStageMask = GrVkImage::LayoutToPipelineSrcStageFlags(currentLayout);

    VkImageAspectFlags aspectFlags = vk_format_to_aspect_flags(fInfo.fFormat);

    uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    if (fInfo.fCurrentQueueFamily != VK_QUEUE_FAMILY_IGNORED &&
        gpu->queueIndex() != fInfo.fCurrentQueueFamily) {
        // The image still is owned by its original queue family and we need to transfer it into
        // ours.
        SkASSERT(!releaseFamilyQueue);
        SkASSERT(fInfo.fCurrentQueueFamily == fInitialQueueFamily);

        srcQueueFamilyIndex = fInfo.fCurrentQueueFamily;
        dstQueueFamilyIndex = gpu->queueIndex();
        fInfo.fCurrentQueueFamily = gpu->queueIndex();
    } else if (releaseFamilyQueue) {
        // We are releasing the image so we must transfer the image back to its original queue
        // family.
        srcQueueFamilyIndex = fInfo.fCurrentQueueFamily;
        dstQueueFamilyIndex = fInitialQueueFamily;
        fInfo.fCurrentQueueFamily = fInitialQueueFamily;
    }

    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,          // sType
        nullptr,                                         // pNext
        srcAccessMask,                                   // srcAccessMask
        dstAccessMask,                                   // dstAccessMask
        currentLayout,                                   // oldLayout
        newLayout,                                       // newLayout
        srcQueueFamilyIndex,                             // srcQueueFamilyIndex
        dstQueueFamilyIndex,                             // dstQueueFamilyIndex
        fInfo.fImage,                                    // image
        { aspectFlags, 0, fInfo.fLevelCount, 0, 1 }      // subresourceRange
    };

    gpu->addImageMemoryBarrier(this->resource(), srcStageMask, dstStageMask, byRegion,
                               &imageMemoryBarrier);

    this->updateImageLayout(newLayout);
}

bool GrVkImage::InitImageInfo(GrVkGpu* gpu, const ImageDesc& imageDesc, GrVkImageInfo* info) {
    if (0 == imageDesc.fWidth || 0 == imageDesc.fHeight) {
        return false;
    }
    if ((imageDesc.fIsProtected == GrProtected::kYes) && !gpu->vkCaps().supportsProtectedMemory()) {
        return false;
    }
    VkImage image = VK_NULL_HANDLE;
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

    VkImageCreateFlags createflags = 0;
    if (imageDesc.fIsProtected == GrProtected::kYes || gpu->protectedContext()) {
        createflags |= VK_IMAGE_CREATE_PROTECTED_BIT;
    }
    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        nullptr,                                     // pNext
        createflags,                                 // VkImageCreateFlags
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

    VkResult result;
    GR_VK_CALL_RESULT(gpu, result, CreateImage(gpu->device(), &imageCreateInfo, nullptr, &image));
    if (result != VK_SUCCESS) {
        return false;
    }

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
    info->fCurrentQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    info->fProtected =
            (createflags & VK_IMAGE_CREATE_PROTECTED_BIT) ? GrProtected::kYes : GrProtected::kNo;
    return true;
}

void GrVkImage::DestroyImageInfo(const GrVkGpu* gpu, GrVkImageInfo* info) {
    VK_CALL(gpu, DestroyImage(gpu->device(), info->fImage, nullptr));
    bool isLinear = VK_IMAGE_TILING_LINEAR == info->fImageTiling;
    GrVkMemory::FreeImageMemory(gpu, isLinear, info->fAlloc);
}

GrVkImage::~GrVkImage() {
    // should have been released first
    SkASSERT(!fResource);
}

void GrVkImage::prepareForPresent(GrVkGpu* gpu) {
    VkImageLayout layout = this->currentLayout();
    if (fInitialQueueFamily != VK_QUEUE_FAMILY_EXTERNAL &&
        fInitialQueueFamily != VK_QUEUE_FAMILY_FOREIGN_EXT) {
        if (gpu->vkCaps().supportsSwapchain()) {
            layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
    }
    this->setImageLayout(gpu, layout, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, false, true);
}

void GrVkImage::prepareForExternal(GrVkGpu* gpu) {
    this->setImageLayout(gpu, this->currentLayout(), 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, false,
                         true);
}

void GrVkImage::releaseImage(GrVkGpu* gpu) {
    if (!gpu->isDeviceLost() && fInfo.fCurrentQueueFamily != fInitialQueueFamily) {
        // The Vulkan spec is vague on what to put for the dstStageMask here. The spec for image
        // memory barrier says the dstStageMask must not be zero. However, in the spec when it talks
        // about family queue transfers it says the dstStageMask is ignored and should be set to
        // zero. Assuming it really is ignored we set it to VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT here
        // since it makes the Vulkan validation layers happy.
        this->setImageLayout(gpu, this->currentLayout(), 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             false, true);
    }
    if (fResource) {
        fResource->removeOwningTexture();
        fResource->unref();
        fResource = nullptr;
    }
}

void GrVkImage::setResourceRelease(sk_sp<GrRefCntedCallback> releaseHelper) {
    SkASSERT(fResource);
    // Forward the release proc on to GrVkImage::Resource
    fResource->setRelease(std::move(releaseHelper));
}

void GrVkImage::Resource::freeGPUData() const {
    this->invokeReleaseProc();
    VK_CALL(fGpu, DestroyImage(fGpu->device(), fImage, nullptr));
    bool isLinear = (VK_IMAGE_TILING_LINEAR == fImageTiling);
    GrVkMemory::FreeImageMemory(fGpu, isLinear, fAlloc);
}

void GrVkImage::BorrowedResource::freeGPUData() const {
    this->invokeReleaseProc();
}

#if GR_TEST_UTILS
void GrVkImage::setCurrentQueueFamilyToGraphicsQueue(GrVkGpu* gpu) {
    fInfo.fCurrentQueueFamily = gpu->queueIndex();
}
#endif

