/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanTexture.h"

#include "include/gpu/MutableTextureState.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/vk/VulkanMemory.h"
#include "src/gpu/vk/VulkanMutableTextureStatePriv.h"

namespace skgpu::graphite {

bool VulkanTexture::MakeVkImage(const VulkanSharedContext* sharedContext,
                                SkISize dimensions,
                                const TextureInfo& info,
                                CreatedImageInfo* outInfo) {
    SkASSERT(outInfo);
    const VulkanCaps& caps = sharedContext->vulkanCaps();

    if (dimensions.isEmpty()) {
        SKGPU_LOG_E("Tried to create VkImage with empty dimensions.");
        return false;
    }
    if (dimensions.width() > caps.maxTextureSize() ||
        dimensions.height() > caps.maxTextureSize()) {
        SKGPU_LOG_E("Tried to create VkImage with too large a size.");
        return false;
    }

    if ((info.isProtected() == Protected::kYes) != caps.protectedSupport()) {
        SKGPU_LOG_E("Tried to create %s VkImage in %s Context.",
                    info.isProtected() == Protected::kYes ? "protected" : "unprotected",
                    caps.protectedSupport() ? "protected" : "unprotected");
        return false;
    }

    const VulkanTextureSpec& spec = info.vulkanTextureSpec();

    bool isLinear = spec.fImageTiling == VK_IMAGE_TILING_LINEAR;
    VkImageLayout initialLayout = isLinear ? VK_IMAGE_LAYOUT_PREINITIALIZED
                                           : VK_IMAGE_LAYOUT_UNDEFINED;

    // Create Image
    VkSampleCountFlagBits vkSamples;
    if (!SampleCountToVkSampleCount(info.numSamples(), &vkSamples)) {
        SKGPU_LOG_E("Failed creating VkImage because we could not covert the number of samples: "
                    "%d to a VkSampleCountFlagBits.", info.numSamples());
        return false;
    }

    SkASSERT(!isLinear || vkSamples == VK_SAMPLE_COUNT_1_BIT);

    VkImageCreateFlags createflags = 0;
    if (info.isProtected() == Protected::kYes && caps.protectedSupport()) {
        createflags |= VK_IMAGE_CREATE_PROTECTED_BIT;
    }

    uint32_t numMipLevels = 1;
    if (info.mipmapped() == Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    uint32_t width = static_cast<uint32_t>(dimensions.fWidth);
    uint32_t height = static_cast<uint32_t>(dimensions.fHeight);

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, // sType
        nullptr,                             // pNext
        createflags,                         // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                    // VkImageType
        spec.fFormat,                        // VkFormat
        { width, height, 1 },                // VkExtent3D
        numMipLevels,                        // mipLevels
        1,                                   // arrayLayers
        vkSamples,                           // samples
        spec.fImageTiling,                   // VkImageTiling
        spec.fImageUsageFlags,               // VkImageUsageFlags
        spec.fSharingMode,                   // VkSharingMode
        0,                                   // queueFamilyCount
        nullptr,                             // pQueueFamilyIndices
        initialLayout                        // initialLayout
    };

    auto interface = sharedContext->interface();
    auto device = sharedContext->device();

    VkImage image = VK_NULL_HANDLE;
    VkResult result;
    VULKAN_CALL_RESULT(interface, result,
                       CreateImage(device, &imageCreateInfo, nullptr, &image));
    if (result != VK_SUCCESS) {
        SKGPU_LOG_E("Failed call to vkCreateImage with error: %d", result);
        return false;
    }

    auto allocator = sharedContext->memoryAllocator();
    bool forceDedicatedMemory = caps.shouldAlwaysUseDedicatedImageMemory();
    bool useLazyAllocation =
            SkToBool(spec.fImageUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);

    auto checkResult = [sharedContext](VkResult result) {
        return sharedContext->checkVkResult(result);
    };
    if (!skgpu::VulkanMemory::AllocImageMemory(allocator,
                                               image,
                                               info.isProtected(),
                                               forceDedicatedMemory,
                                               useLazyAllocation,
                                               checkResult,
                                               &outInfo->fMemoryAlloc)) {
        VULKAN_CALL(interface, DestroyImage(device, image, nullptr));
        return false;
    }

    if (useLazyAllocation &&
        !SkToBool(outInfo->fMemoryAlloc.fFlags & skgpu::VulkanAlloc::kLazilyAllocated_Flag)) {
        SKGPU_LOG_E("Failed allocate lazy vulkan memory when requested");
        skgpu::VulkanMemory::FreeImageMemory(allocator, outInfo->fMemoryAlloc);
        return false;
    }

    VULKAN_CALL_RESULT(interface, result, BindImageMemory(device,
                                                          image,
                                                          outInfo->fMemoryAlloc.fMemory,
                                                          outInfo->fMemoryAlloc.fOffset));
    if (result != VK_SUCCESS) {
        skgpu::VulkanMemory::FreeImageMemory(allocator, outInfo->fMemoryAlloc);
        VULKAN_CALL(interface, DestroyImage(device, image, nullptr));
        return false;
    }

    outInfo->fImage = image;
    outInfo->fMutableState = sk_make_sp<MutableTextureState>(
            skgpu::MutableTextureStates::MakeVulkan(initialLayout, VK_QUEUE_FAMILY_IGNORED));
    return true;
}

sk_sp<Texture> VulkanTexture::Make(const VulkanSharedContext* sharedContext,
                                   const VulkanResourceProvider* resourceProvider,
                                   SkISize dimensions,
                                   const TextureInfo& info,
                                   skgpu::Budgeted budgeted) {
    CreatedImageInfo imageInfo;
    if (!MakeVkImage(sharedContext, dimensions, info, &imageInfo)) {
        return nullptr;
    }
    auto ycbcrConversion = resourceProvider->findOrCreateCompatibleSamplerYcbcrConversion(
            info.vulkanTextureSpec().fYcbcrConversionInfo);

    return sk_sp<Texture>(new VulkanTexture(sharedContext,
                                            dimensions,
                                            info,
                                            std::move(imageInfo.fMutableState),
                                            imageInfo.fImage,
                                            imageInfo.fMemoryAlloc,
                                            Ownership::kOwned,
                                            budgeted,
                                            std::move(ycbcrConversion)));
}

sk_sp<Texture> VulkanTexture::MakeWrapped(const VulkanSharedContext* sharedContext,
                                          const VulkanResourceProvider* resourceProvider,
                                          SkISize dimensions,
                                          const TextureInfo& info,
                                          sk_sp<MutableTextureState> mutableState,
                                          VkImage image,
                                          const VulkanAlloc& alloc) {
    auto ycbcrConversion = resourceProvider->findOrCreateCompatibleSamplerYcbcrConversion(
            info.vulkanTextureSpec().fYcbcrConversionInfo);

    return sk_sp<Texture>(new VulkanTexture(sharedContext,
                                            dimensions,
                                            info,
                                            std::move(mutableState),
                                            image,
                                            alloc,
                                            Ownership::kWrapped,
                                            skgpu::Budgeted::kNo,
                                            std::move(ycbcrConversion)));
}

VkImageAspectFlags vk_format_to_aspect_flags(VkFormat format) {
    switch (format) {
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            [[fallthrough]];
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

void VulkanTexture::setImageLayoutAndQueueIndex(VulkanCommandBuffer* cmdBuffer,
                                                VkImageLayout newLayout,
                                                VkAccessFlags dstAccessMask,
                                                VkPipelineStageFlags dstStageMask,
                                                bool byRegion,
                                                uint32_t newQueueFamilyIndex) const {

    SkASSERT(newLayout == this->currentLayout() ||
             (VK_IMAGE_LAYOUT_UNDEFINED != newLayout &&
              VK_IMAGE_LAYOUT_PREINITIALIZED != newLayout));
    VkImageLayout currentLayout = this->currentLayout();
    uint32_t currentQueueIndex = this->currentQueueFamilyIndex();

    VulkanTextureInfo textureInfo;
    this->textureInfo().getVulkanTextureInfo(&textureInfo);
    auto sharedContext = static_cast<const VulkanSharedContext*>(this->sharedContext());

    // Enable the following block on new devices to test that their lazy images stay at 0 memory use
#if 0
    auto interface = sharedContext->interface();
    auto device = sharedContext->device();
    if (fAlloc.fFlags & skgpu::VulkanAlloc::kLazilyAllocated_Flag) {
        VkDeviceSize size;
        VULKAN_CALL(interface, GetDeviceMemoryCommitment(device, fAlloc.fMemory, &size));

        SkDebugf("Lazy Image. This: %p, image: %d, size: %d\n", this, fImage, size);
    }
#endif
#ifdef SK_DEBUG
    if (textureInfo.fSharingMode == VK_SHARING_MODE_CONCURRENT) {
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
        SkASSERT(textureInfo.fSharingMode == VK_SHARING_MODE_EXCLUSIVE);
        if (newQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED ||
            currentQueueIndex == sharedContext->queueIndex()) {
            SkASSERT(currentQueueIndex == VK_QUEUE_FAMILY_IGNORED ||
                     currentQueueIndex == VK_QUEUE_FAMILY_EXTERNAL ||
                     currentQueueIndex == VK_QUEUE_FAMILY_FOREIGN_EXT ||
                     currentQueueIndex == sharedContext->queueIndex());
        } else if (newQueueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
                   newQueueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT) {
            SkASSERT(currentQueueIndex == VK_QUEUE_FAMILY_IGNORED ||
                     currentQueueIndex == sharedContext->queueIndex());
        }
    }
#endif

    if (textureInfo.fSharingMode == VK_SHARING_MODE_EXCLUSIVE) {
        if (newQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED) {
            newQueueFamilyIndex = sharedContext->queueIndex();
        }
        if (currentQueueIndex == VK_QUEUE_FAMILY_IGNORED) {
            currentQueueIndex = sharedContext->queueIndex();
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

    VkAccessFlags srcAccessMask = VulkanTexture::LayoutToSrcAccessMask(currentLayout);
    VkPipelineStageFlags srcStageMask = VulkanTexture::LayoutToPipelineSrcStageFlags(currentLayout);

    VkImageAspectFlags aspectFlags = vk_format_to_aspect_flags(textureInfo.fFormat);
    uint32_t numMipLevels = 1;
    SkISize dimensions = this->dimensions();
    if (this->mipmapped() == Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }
    VkImageMemoryBarrier imageMemoryBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,          // sType
        nullptr,                                         // pNext
        srcAccessMask,                                   // srcAccessMask
        dstAccessMask,                                   // dstAccessMask
        currentLayout,                                   // oldLayout
        newLayout,                                       // newLayout
        currentQueueIndex,                               // srcQueueFamilyIndex
        newQueueFamilyIndex,                             // dstQueueFamilyIndex
        fImage,                                          // image
        { aspectFlags, 0, numMipLevels, 0, 1 }           // subresourceRange
    };
    SkASSERT(srcAccessMask == imageMemoryBarrier.srcAccessMask);
    cmdBuffer->addImageMemoryBarrier(this, srcStageMask, dstStageMask, byRegion,
                                     &imageMemoryBarrier);

    skgpu::MutableTextureStates::SetVkImageLayout(this->mutableState(), newLayout);
    skgpu::MutableTextureStates::SetVkQueueFamilyIndex(this->mutableState(), newQueueFamilyIndex);
}

VulkanTexture::VulkanTexture(const VulkanSharedContext* sharedContext,
                             SkISize dimensions,
                             const TextureInfo& info,
                             sk_sp<MutableTextureState> mutableState,
                             VkImage image,
                             const VulkanAlloc& alloc,
                             Ownership ownership,
                             skgpu::Budgeted budgeted,
                             sk_sp<VulkanSamplerYcbcrConversion> ycbcrConversion)
        : Texture(sharedContext, dimensions, info, std::move(mutableState), ownership, budgeted)
        , fImage(image)
        , fMemoryAlloc(alloc)
        , fSamplerYcbcrConversion(std::move(ycbcrConversion)) {}

void VulkanTexture::freeGpuData() {
    // Need to delete any ImageViews first
    fImageViews.clear();

    // If the texture is wrapped we don't own this data
    if (this->ownership() != Ownership::kWrapped) {
        auto sharedContext = static_cast<const VulkanSharedContext*>(this->sharedContext());
        VULKAN_CALL(sharedContext->interface(),
                    DestroyImage(sharedContext->device(), fImage, nullptr));
        skgpu::VulkanMemory::FreeImageMemory(sharedContext->memoryAllocator(), fMemoryAlloc);
    }
}

void VulkanTexture::updateImageLayout(VkImageLayout newLayout) {
    skgpu::MutableTextureStates::SetVkImageLayout(this->mutableState(), newLayout);
}

VkImageLayout VulkanTexture::currentLayout() const {
    return skgpu::MutableTextureStates::GetVkImageLayout(this->mutableState());
}

uint32_t VulkanTexture::currentQueueFamilyIndex() const {
    return skgpu::MutableTextureStates::GetVkQueueFamilyIndex(this->mutableState());
}

VkPipelineStageFlags VulkanTexture::LayoutToPipelineSrcStageFlags(const VkImageLayout layout) {
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

VkAccessFlags VulkanTexture::LayoutToSrcAccessMask(const VkImageLayout layout) {
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

const VulkanImageView* VulkanTexture::getImageView(VulkanImageView::Usage usage) const {
    for (int i = 0; i < fImageViews.size(); ++i) {
        if (fImageViews[i]->usage() == usage) {
            return fImageViews[i].get();
        }
    }

    auto sharedContext = static_cast<const VulkanSharedContext*>(this->sharedContext());
    VulkanTextureInfo vkTexInfo;
    this->textureInfo().getVulkanTextureInfo(&vkTexInfo);
    int miplevels = this->textureInfo().mipmapped() == Mipmapped::kYes
                    ? SkMipmap::ComputeLevelCount(this->dimensions().width(),
                                                  this->dimensions().height()) + 1
                    : 1;
    auto imageView = VulkanImageView::Make(sharedContext,
                                           fImage,
                                           vkTexInfo.fFormat,
                                           usage,
                                           miplevels,
                                           fSamplerYcbcrConversion);
    return fImageViews.push_back(std::move(imageView)).get();
}


} // namespace skgpu::graphite
