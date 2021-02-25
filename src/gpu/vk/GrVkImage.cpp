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

GrVkImage::GrVkImage(const GrVkGpu* gpu,
                     const GrVkImageInfo& info,
                     sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                     GrBackendObjectOwnership ownership,
                     bool forSecondaryCB)
        : fInfo(info)
        , fInitialQueueFamily(info.fCurrentQueueFamily)
        , fMutableState(std::move(mutableState))
        , fIsBorrowed(GrBackendObjectOwnership::kBorrowed == ownership) {
    SkASSERT(fMutableState->getImageLayout() == fInfo.fImageLayout);
    SkASSERT(fMutableState->getQueueFamilyIndex() == fInfo.fCurrentQueueFamily);
#ifdef SK_DEBUG
    if (info.fImageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        SkASSERT(SkToBool(info.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT));
    } else {
        SkASSERT(SkToBool(info.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) &&
                 SkToBool(info.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
    }
    // We can't transfer from the non graphics queue to the graphics queue since we can't
    // release the image from the original queue without having that queue. This limits us in terms
    // of the types of queue indices we can handle.
    if (info.fCurrentQueueFamily != VK_QUEUE_FAMILY_IGNORED &&
        info.fCurrentQueueFamily != VK_QUEUE_FAMILY_EXTERNAL &&
        info.fCurrentQueueFamily != VK_QUEUE_FAMILY_FOREIGN_EXT) {
        if (info.fSharingMode == VK_SHARING_MODE_EXCLUSIVE) {
            if (info.fCurrentQueueFamily != gpu->queueIndex()) {
                SkASSERT(false);
            }
        } else {
            SkASSERT(false);
        }
    }
#endif
    if (forSecondaryCB) {
        fResource = nullptr;
    } else if (fIsBorrowed) {
        fResource = new BorrowedResource(gpu, info.fImage, info.fAlloc, info.fImageTiling);
    } else {
        SkASSERT(VK_NULL_HANDLE != info.fAlloc.fMemory);
        fResource = new Resource(gpu, info.fImage, info.fAlloc, info.fImageTiling);
    }
}

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
                VK_ACCESS_HOST_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_PREINITIALIZED == layout) {
        flags = VK_ACCESS_HOST_WRITE_BIT;
    } else if (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL == layout) {
        flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
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

void GrVkImage::setImageLayoutAndQueueIndex(const GrVkGpu* gpu,
                                            VkImageLayout newLayout,
                                            VkAccessFlags dstAccessMask,
                                            VkPipelineStageFlags dstStageMask,
                                            bool byRegion,
                                            uint32_t newQueueFamilyIndex) {
    SkASSERT(!gpu->isDeviceLost());
    SkASSERT(newLayout == this->currentLayout() ||
             (VK_IMAGE_LAYOUT_UNDEFINED != newLayout &&
              VK_IMAGE_LAYOUT_PREINITIALIZED != newLayout));
    VkImageLayout currentLayout = this->currentLayout();
    uint32_t currentQueueIndex = this->currentQueueFamilyIndex();

#ifdef SK_DEBUG
    if (fInfo.fSharingMode == VK_SHARING_MODE_CONCURRENT) {
        if (newQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED) {
            SkASSERT(currentQueueIndex == VK_QUEUE_FAMILY_IGNORED ||
                     currentQueueIndex == VK_QUEUE_FAMILY_EXTERNAL ||
                     currentQueueIndex == VK_QUEUE_FAMILY_FOREIGN_EXT);
        } else {
            SkASSERT(newQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
                     newQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT);
            SkASSERT(currentQueueIndex == VK_QUEUE_FAMILY_IGNORED);
        }
    } else {
        SkASSERT(fInfo.fSharingMode == VK_SHARING_MODE_EXCLUSIVE);
        if (newQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED ||
            currentQueueIndex == gpu->queueIndex()) {
            SkASSERT(currentQueueIndex == VK_QUEUE_FAMILY_IGNORED ||
                     currentQueueIndex == VK_QUEUE_FAMILY_EXTERNAL ||
                     currentQueueIndex == VK_QUEUE_FAMILY_FOREIGN_EXT ||
                     currentQueueIndex == gpu->queueIndex());
        } else if (newQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
                   newQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT) {
            SkASSERT(currentQueueIndex == VK_QUEUE_FAMILY_IGNORED ||
                     currentQueueIndex == gpu->queueIndex());
        }
    }
#endif

    if (fInfo.fSharingMode == VK_SHARING_MODE_EXCLUSIVE) {
        if (newQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED) {
            newQueueFamilyIndex = gpu->queueIndex();
        }
        if (currentQueueIndex == VK_QUEUE_FAMILY_IGNORED) {
            currentQueueIndex = gpu->queueIndex();
        }
    }

    // If the old and new layout are the same and the layout is a read only layout, there is no need
    // to put in a barrier unless we also need to switch queues.
    if (newLayout == currentLayout && currentQueueIndex == newQueueFamilyIndex &&
        (VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL == currentLayout ||
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL == currentLayout ||
         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL == currentLayout)) {
        return;
    }

    VkAccessFlags srcAccessMask = GrVkImage::LayoutToSrcAccessMask(currentLayout);
    VkPipelineStageFlags srcStageMask = GrVkImage::LayoutToPipelineSrcStageFlags(currentLayout);

    VkImageAspectFlags aspectFlags = vk_format_to_aspect_flags(fInfo.fFormat);

    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,          // sType
        nullptr,                                         // pNext
        srcAccessMask,                                   // srcAccessMask
        dstAccessMask,                                   // dstAccessMask
        currentLayout,                                   // oldLayout
        newLayout,                                       // newLayout
        currentQueueIndex,                               // srcQueueFamilyIndex
        newQueueFamilyIndex,                             // dstQueueFamilyIndex
        fInfo.fImage,                                    // image
        { aspectFlags, 0, fInfo.fLevelCount, 0, 1 }      // subresourceRange
    };
    SkASSERT(srcAccessMask == imageMemoryBarrier.srcAccessMask);
    gpu->addImageMemoryBarrier(this->resource(), srcStageMask, dstStageMask, byRegion,
                               &imageMemoryBarrier);

    this->updateImageLayout(newLayout);
    this->setQueueFamilyIndex(newQueueFamilyIndex);
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
        nullptr,                                     // pQueueFamilyIndices
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
    info->fImageUsageFlags = imageDesc.fUsageFlags;
    info->fSampleCount = imageDesc.fSamples;
    info->fLevelCount = imageDesc.fLevels;
    info->fCurrentQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    info->fProtected =
            (createflags & VK_IMAGE_CREATE_PROTECTED_BIT) ? GrProtected::kYes : GrProtected::kNo;
    info->fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
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
    this->setImageLayoutAndQueueIndex(gpu, layout, 0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, false,
                                      fInitialQueueFamily);
}

void GrVkImage::prepareForExternal(GrVkGpu* gpu) {
    this->setImageLayoutAndQueueIndex(gpu, this->currentLayout(), 0,
                                      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, false,
                                     fInitialQueueFamily);
}

void GrVkImage::releaseImage() {
    if (fResource) {
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
    fMutableState->setQueueFamilyIndex(gpu->queueIndex());
}
#endif

