/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanTexture.h"

#include "src/core/SkMipmap.h"
#include "src/gpu/MutableTextureStateRef.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/vk/VulkanMemory.h"

namespace skgpu::graphite {

bool VulkanTexture::MakeVkImage(const VulkanSharedContext* sharedContext,
                                SkISize dimensions,
                                const TextureInfo& info,
                                CreatedImageInfo* outInfo) {
    SkASSERT(outInfo);
    if (dimensions.isEmpty()) {
        SKGPU_LOG_E("Tried to create VkImage with empty dimensions.");
        return false;
    }

    const VulkanCaps& caps = sharedContext->vulkanCaps();
    if (info.isProtected() == Protected::kYes && !caps.protectedSupport()) {
        SKGPU_LOG_E("Tried to create protected VkImage when protected not supported.");
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
    if (info.isProtected() == Protected::kYes) {
        SkASSERT(caps.protectedSupport());
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

    auto checkResult = [](VkResult result) {
        return result == VK_SUCCESS;
    };
    if (!skgpu::VulkanMemory::AllocImageMemory(allocator,
                                               image,
                                               info.isProtected(),
                                               forceDedicatedMemory,
                                               useLazyAllocation,
                                               checkResult,
                                               &outInfo->fMemoryAlloc) ||
        (useLazyAllocation &&
         !SkToBool(outInfo->fMemoryAlloc.fFlags & skgpu::VulkanAlloc::kLazilyAllocated_Flag))) {
        VULKAN_CALL(interface, DestroyImage(device, image, nullptr));
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
    outInfo->fMutableState = sk_make_sp<MutableTextureStateRef>(initialLayout,
                                                                VK_QUEUE_FAMILY_IGNORED);
    return true;
}

sk_sp<Texture> VulkanTexture::Make(const VulkanSharedContext* sharedContext,
                                   SkISize dimensions,
                                   const TextureInfo& info,
                                   SkBudgeted budgeted) {
    CreatedImageInfo imageInfo;
    if (!MakeVkImage(sharedContext, dimensions, info, &imageInfo)) {
        return nullptr;
    }
    return sk_sp<Texture>(new VulkanTexture(sharedContext,
                                            dimensions,
                                            info,
                                            std::move(imageInfo.fMutableState),
                                            imageInfo.fImage,
                                            imageInfo.fMemoryAlloc,
                                            Ownership::kOwned,
                                            budgeted));
}

sk_sp<Texture> VulkanTexture::MakeWrapped(const VulkanSharedContext* sharedContext,
                                   SkISize dimensions,
                                   const TextureInfo& info,
                                   sk_sp<MutableTextureStateRef> mutableState,
                                   VkImage image,
                                   const VulkanAlloc& alloc) {
    return sk_sp<Texture>(new VulkanTexture(sharedContext,
                                            dimensions,
                                            info,
                                            std::move(mutableState),
                                            image,
                                            alloc,
                                            Ownership::kWrapped,
                                            SkBudgeted::kNo));
}

VulkanTexture::VulkanTexture(const VulkanSharedContext* sharedContext,
                             SkISize dimensions,
                             const TextureInfo& info,
                             sk_sp<MutableTextureStateRef> mutableState,
                             VkImage image,
                             const VulkanAlloc& alloc,
                             Ownership ownership,
                             SkBudgeted budgeted)
        : Texture(sharedContext, dimensions, info, std::move(mutableState), ownership, budgeted)
        , fImage(image)
        , fMemoryAlloc(alloc) {}

} // namespace skgpu::graphite
