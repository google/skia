/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/vk/GrVkImage.h"

#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkImageView.h"
#include "src/gpu/ganesh/vk/GrVkTexture.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/gpu/vk/VulkanMemory.h"
#include "src/gpu/vk/VulkanMutableTextureStatePriv.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#define VK_CALL(GPU, X) GR_VK_CALL(GPU->vkInterface(), X)

sk_sp<GrVkImage> GrVkImage::MakeStencil(GrVkGpu* gpu,
                                        SkISize dimensions,
                                        int sampleCnt,
                                        VkFormat format) {
    VkImageUsageFlags vkUsageFlags =
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return GrVkImage::Make(gpu,
                           dimensions,
                           UsageFlags::kStencilAttachment,
                           sampleCnt,
                           format,
                           /*mipLevels=*/1,
                           vkUsageFlags,
                           GrProtected::kNo,
                           GrMemoryless::kNo,
                           skgpu::Budgeted::kYes);
}

sk_sp<GrVkImage> GrVkImage::MakeMSAA(GrVkGpu* gpu,
                                     SkISize dimensions,
                                     int numSamples,
                                     VkFormat format,
                                     GrProtected isProtected,
                                     GrMemoryless memoryless) {
    SkASSERT(numSamples > 1);

    VkImageUsageFlags vkUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (memoryless == GrMemoryless::kYes) {
        vkUsageFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    } else {
        vkUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    return GrVkImage::Make(gpu,
                           dimensions,
                           UsageFlags::kColorAttachment,
                           numSamples,
                           format,
                           /*mipLevels=*/1,
                           vkUsageFlags,
                           isProtected,
                           memoryless,
                           skgpu::Budgeted::kYes);
}

sk_sp<GrVkImage> GrVkImage::MakeTexture(GrVkGpu* gpu,
                                        SkISize dimensions,
                                        VkFormat format,
                                        uint32_t mipLevels,
                                        GrRenderable renderable,
                                        int numSamples,
                                        skgpu::Budgeted budgeted,
                                        GrProtected isProtected) {
    UsageFlags usageFlags = UsageFlags::kTexture;
    VkImageUsageFlags vkUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (renderable == GrRenderable::kYes) {
        usageFlags |= UsageFlags::kColorAttachment;
        vkUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        // We always make our render targets support being used as input attachments
        vkUsageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    return GrVkImage::Make(gpu,
                           dimensions,
                           usageFlags,
                           numSamples,
                           format,
                           mipLevels,
                           vkUsageFlags,
                           isProtected,
                           GrMemoryless::kNo,
                           budgeted);
}

static bool make_views(GrVkGpu* gpu,
                       const GrVkImageInfo& info,
                       GrAttachment::UsageFlags attachmentUsages,
                       sk_sp<const GrVkImageView>* framebufferView,
                       sk_sp<const GrVkImageView>* textureView) {
    GrVkImageView::Type viewType;
    if (attachmentUsages & GrAttachment::UsageFlags::kStencilAttachment) {
        // If we have stencil usage then we shouldn't have any other usages
        SkASSERT(attachmentUsages == GrAttachment::UsageFlags::kStencilAttachment);
        viewType = GrVkImageView::kStencil_Type;
    } else {
        viewType = GrVkImageView::kColor_Type;
    }

    if (SkToBool(attachmentUsages & GrAttachment::UsageFlags::kStencilAttachment) ||
        SkToBool(attachmentUsages & GrAttachment::UsageFlags::kColorAttachment)) {
        // Attachments can only have a mip level of 1
        *framebufferView = GrVkImageView::Make(
                gpu, info.fImage, info.fFormat, viewType, 1, info.fYcbcrConversionInfo);
        if (!*framebufferView) {
            return false;
        }
    }

    if (attachmentUsages & GrAttachment::UsageFlags::kTexture) {
        *textureView = GrVkImageView::Make(gpu,
                                           info.fImage,
                                           info.fFormat,
                                           viewType,
                                           info.fLevelCount,
                                           info.fYcbcrConversionInfo);
        if (!*textureView) {
            return false;
        }
    }
    return true;
}

sk_sp<GrVkImage> GrVkImage::Make(GrVkGpu* gpu,
                                 SkISize dimensions,
                                 UsageFlags attachmentUsages,
                                 int sampleCnt,
                                 VkFormat format,
                                 uint32_t mipLevels,
                                 VkImageUsageFlags vkUsageFlags,
                                 GrProtected isProtected,
                                 GrMemoryless memoryless,
                                 skgpu::Budgeted budgeted) {
    GrVkImage::ImageDesc imageDesc;
    imageDesc.fImageType = VK_IMAGE_TYPE_2D;
    imageDesc.fFormat = format;
    imageDesc.fWidth = dimensions.width();
    imageDesc.fHeight = dimensions.height();
    imageDesc.fLevels = mipLevels;
    imageDesc.fSamples = sampleCnt;
    imageDesc.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageDesc.fUsageFlags = vkUsageFlags;
    imageDesc.fIsProtected = isProtected;

    GrVkImageInfo info;
    if (!GrVkImage::InitImageInfo(gpu, imageDesc, &info)) {
        return nullptr;
    }

    sk_sp<const GrVkImageView> framebufferView;
    sk_sp<const GrVkImageView> textureView;
    if (!make_views(gpu, info, attachmentUsages, &framebufferView, &textureView)) {
        GrVkImage::DestroyImageInfo(gpu, &info);
        return nullptr;
    }

    auto mutableState = sk_make_sp<skgpu::MutableTextureState>(
            skgpu::MutableTextureStates::MakeVulkan(info.fImageLayout, info.fCurrentQueueFamily));
    return sk_sp<GrVkImage>(new GrVkImage(gpu,
                                          dimensions,
                                          attachmentUsages,
                                          info,
                                          std::move(mutableState),
                                          std::move(framebufferView),
                                          std::move(textureView),
                                          budgeted,
                                          /*label=*/"MakeVkImage"));
}

sk_sp<GrVkImage> GrVkImage::MakeWrapped(GrVkGpu* gpu,
                                        SkISize dimensions,
                                        const GrVkImageInfo& info,
                                        sk_sp<skgpu::MutableTextureState> mutableState,
                                        UsageFlags attachmentUsages,
                                        GrWrapOwnership ownership,
                                        GrWrapCacheable cacheable,
                                        std::string_view label,
                                        bool forSecondaryCB) {
    sk_sp<const GrVkImageView> framebufferView;
    sk_sp<const GrVkImageView> textureView;
    if (!forSecondaryCB) {
        if (!make_views(gpu, info, attachmentUsages, &framebufferView, &textureView)) {
            return nullptr;
        }
    }

    GrBackendObjectOwnership backendOwnership = kBorrow_GrWrapOwnership == ownership
                                                        ? GrBackendObjectOwnership::kBorrowed
                                                        : GrBackendObjectOwnership::kOwned;

    return sk_sp<GrVkImage>(new GrVkImage(gpu,
                                          dimensions,
                                          attachmentUsages,
                                          info,
                                          std::move(mutableState),
                                          std::move(framebufferView),
                                          std::move(textureView),
                                          backendOwnership,
                                          cacheable,
                                          forSecondaryCB,
                                          label));
}

GrVkImage::GrVkImage(GrVkGpu* gpu,
                     SkISize dimensions,
                     UsageFlags supportedUsages,
                     const GrVkImageInfo& info,
                     sk_sp<skgpu::MutableTextureState> mutableState,
                     sk_sp<const GrVkImageView> framebufferView,
                     sk_sp<const GrVkImageView> textureView,
                     skgpu::Budgeted budgeted,
                     std::string_view label)
        : GrAttachment(gpu,
                       dimensions,
                       supportedUsages,
                       info.fSampleCount,
                       info.fLevelCount > 1 ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo,
                       info.fProtected,
                       label,
                       info.fAlloc.fFlags & skgpu::VulkanAlloc::kLazilyAllocated_Flag
                               ? GrMemoryless::kYes
                               : GrMemoryless::kNo)
        , fInfo(info)
        , fInitialQueueFamily(info.fCurrentQueueFamily)
        , fMutableState(std::move(mutableState))
        , fFramebufferView(std::move(framebufferView))
        , fTextureView(std::move(textureView))
        , fIsBorrowed(false) {
    this->init(gpu, false);
    this->registerWithCache(budgeted);
}

GrVkImage::GrVkImage(GrVkGpu* gpu,
                     SkISize dimensions,
                     UsageFlags supportedUsages,
                     const GrVkImageInfo& info,
                     sk_sp<skgpu::MutableTextureState> mutableState,
                     sk_sp<const GrVkImageView> framebufferView,
                     sk_sp<const GrVkImageView> textureView,
                     GrBackendObjectOwnership ownership,
                     GrWrapCacheable cacheable,
                     bool forSecondaryCB,
                     std::string_view label)
        : GrAttachment(gpu,
                       dimensions,
                       supportedUsages,
                       info.fSampleCount,
                       info.fLevelCount > 1 ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo,
                       info.fProtected,
                       label)
        , fInfo(info)
        , fInitialQueueFamily(info.fCurrentQueueFamily)
        , fMutableState(std::move(mutableState))
        , fFramebufferView(std::move(framebufferView))
        , fTextureView(std::move(textureView))
        , fIsBorrowed(GrBackendObjectOwnership::kBorrowed == ownership) {
    this->init(gpu, forSecondaryCB);
    this->registerWithCacheWrapped(cacheable);
}

void GrVkImage::init(GrVkGpu* gpu, bool forSecondaryCB) {
    SkASSERT(skgpu::MutableTextureStates::GetVkImageLayout(fMutableState.get()) == fInfo.fImageLayout);
    SkASSERT(skgpu::MutableTextureStates::GetVkQueueFamilyIndex(fMutableState.get()) == fInfo.fCurrentQueueFamily);
#ifdef SK_DEBUG
    if (fInfo.fImageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        SkASSERT(SkToBool(fInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT));
    } else {
        if (fInfo.fAlloc.fFlags & skgpu::VulkanAlloc::kLazilyAllocated_Flag) {
            SkASSERT(fInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
            SkASSERT(!SkToBool(fInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) &&
                     !SkToBool(fInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
        } else {
            SkASSERT(!SkToBool(fInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT));
            SkASSERT(SkToBool(fInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) &&
                     SkToBool(fInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
        }
    }
    // We can't transfer from the non graphics queue to the graphics queue since we can't
    // release the image from the original queue without having that queue. This limits us in terms
    // of the types of queue indices we can handle.
    if (fInfo.fCurrentQueueFamily != VK_QUEUE_FAMILY_IGNORED &&
        fInfo.fCurrentQueueFamily != VK_QUEUE_FAMILY_EXTERNAL &&
        fInfo.fCurrentQueueFamily != VK_QUEUE_FAMILY_FOREIGN_EXT) {
        if (fInfo.fSharingMode == VK_SHARING_MODE_EXCLUSIVE) {
            if (fInfo.fCurrentQueueFamily != gpu->queueIndex()) {
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
        fResource = new BorrowedResource(gpu, fInfo.fImage, fInfo.fAlloc, fInfo.fImageTiling);
    } else {
        SkASSERT(VK_NULL_HANDLE != fInfo.fAlloc.fMemory);
        fResource = new Resource(gpu, fInfo.fImage, fInfo.fAlloc, fInfo.fImageTiling);
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
// Enable the following block to test new devices to confirm their lazy images stay at 0 memory use.
#if 0
    if (fInfo.fAlloc.fFlags & skgpu::VulkanAlloc::kLazilyAllocated_Flag) {
        VkDeviceSize size;
        VK_CALL(gpu, GetDeviceMemoryCommitment(gpu->device(), fInfo.fAlloc.fMemory, &size));

        SkDebugf("Lazy Image. This: %p, image: %d, size: %d\n", this, fInfo.fImage, size);
    }
#endif
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
    if ((imageDesc.fIsProtected == GrProtected::kYes) &&
        !gpu->vkCaps().supportsProtectedContent()) {
        return false;
    }

    bool isLinear = VK_IMAGE_TILING_LINEAR == imageDesc.fImageTiling;
    VkImageLayout initialLayout = isLinear ? VK_IMAGE_LAYOUT_PREINITIALIZED
                                           : VK_IMAGE_LAYOUT_UNDEFINED;

    // Create Image
    VkSampleCountFlagBits vkSamples;
    if (!skgpu::SampleCountToVkSampleCount(imageDesc.fSamples, &vkSamples)) {
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

    VkImage image = VK_NULL_HANDLE;
    VkResult result;
    GR_VK_CALL_RESULT(gpu, result, CreateImage(gpu->device(), &imageCreateInfo, nullptr, &image));
    if (result != VK_SUCCESS) {
        return false;
    }

    skgpu::Protected isProtected = gpu->protectedContext() ? skgpu::Protected::kYes
                                                           : skgpu::Protected::kNo;
    bool forceDedicatedMemory = gpu->vkCaps().shouldAlwaysUseDedicatedImageMemory();
    bool useLazyAllocation =
            SkToBool(imageDesc.fUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);

    auto checkResult = [gpu, isProtected, forceDedicatedMemory, useLazyAllocation](
                               VkResult result) {
        GR_VK_LOG_IF_NOT_SUCCESS(gpu, result, "skgpu::VulkanMemory::AllocImageMemory"
                                 " (isProtected:%d, forceDedicatedMemory:%d, useLazyAllocation:%d)",
                                 (int)isProtected, (int)forceDedicatedMemory,
                                 (int)useLazyAllocation);
        return gpu->checkVkResult(result);
    };
    auto allocator = gpu->memoryAllocator();
    skgpu::VulkanAlloc alloc;
    if (!skgpu::VulkanMemory::AllocImageMemory(allocator,
                                               image,
                                               isProtected,
                                               forceDedicatedMemory,
                                               useLazyAllocation,
                                               checkResult,
                                               &alloc) ||
        (useLazyAllocation &&
         !SkToBool(alloc.fFlags & skgpu::VulkanAlloc::kLazilyAllocated_Flag))) {
        VK_CALL(gpu, DestroyImage(gpu->device(), image, nullptr));
        return false;
    }

    // Bind buffer
    GR_VK_CALL_RESULT(gpu, result, BindImageMemory(gpu->device(),
                                                   image,
                                                   alloc.fMemory,
                                                   alloc.fOffset));
    if (result) {
        skgpu::VulkanMemory::FreeImageMemory(allocator, alloc);
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
    skgpu::VulkanMemory::FreeImageMemory(gpu->memoryAllocator(), info->fAlloc);
}

GrVkImage::~GrVkImage() {
    // should have been released first
    SkASSERT(!fResource);
    SkASSERT(!fFramebufferView);
    SkASSERT(!fTextureView);
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
    fFramebufferView.reset();
    fTextureView.reset();
    fCachedBlendingInputDescSet.reset();
    fCachedMSAALoadInputDescSet.reset();
}

void GrVkImage::onRelease() {
    this->releaseImage();
    GrAttachment::onRelease();
}

void GrVkImage::onAbandon() {
    this->releaseImage();
    GrAttachment::onAbandon();
}

void GrVkImage::setResourceRelease(sk_sp<RefCntedReleaseProc> releaseHelper) {
    SkASSERT(fResource);
    // Forward the release proc on to GrVkImage::Resource
    fResource->setRelease(std::move(releaseHelper));
}

void GrVkImage::Resource::freeGPUData() const {
    this->invokeReleaseProc();
    VK_CALL(fGpu, DestroyImage(fGpu->device(), fImage, nullptr));
    skgpu::VulkanMemory::FreeImageMemory(fGpu->memoryAllocator(), fAlloc);
}

void GrVkImage::BorrowedResource::freeGPUData() const {
    this->invokeReleaseProc();
}

static void write_input_desc_set(GrVkGpu* gpu,
                                 VkImageView view,
                                 VkImageLayout layout,
                                 VkDescriptorSet descSet) {
    VkDescriptorImageInfo imageInfo;
    memset(&imageInfo, 0, sizeof(VkDescriptorImageInfo));
    imageInfo.sampler = VK_NULL_HANDLE;
    imageInfo.imageView = view;
    imageInfo.imageLayout = layout;

    VkWriteDescriptorSet writeInfo;
    memset(&writeInfo, 0, sizeof(VkWriteDescriptorSet));
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.pNext = nullptr;
    writeInfo.dstSet = descSet;
    writeInfo.dstBinding = GrVkUniformHandler::kInputBinding;
    writeInfo.dstArrayElement = 0;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    writeInfo.pImageInfo = &imageInfo;
    writeInfo.pBufferInfo = nullptr;
    writeInfo.pTexelBufferView = nullptr;

    GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(), 1, &writeInfo, 0, nullptr));
}

gr_rp<const GrVkDescriptorSet> GrVkImage::inputDescSetForBlending(GrVkGpu* gpu) {
    if (!this->supportsInputAttachmentUsage()) {
        return nullptr;
    }
    if (fCachedBlendingInputDescSet) {
        return fCachedBlendingInputDescSet;
    }

    fCachedBlendingInputDescSet.reset(gpu->resourceProvider().getInputDescriptorSet());
    if (!fCachedBlendingInputDescSet) {
        return nullptr;
    }

    write_input_desc_set(gpu,
                         this->framebufferView()->imageView(),
                         VK_IMAGE_LAYOUT_GENERAL,
                         *fCachedBlendingInputDescSet->descriptorSet());

    return fCachedBlendingInputDescSet;
}

gr_rp<const GrVkDescriptorSet> GrVkImage::inputDescSetForMSAALoad(GrVkGpu* gpu) {
    if (!this->supportsInputAttachmentUsage()) {
        return nullptr;
    }
    if (fCachedMSAALoadInputDescSet) {
        return fCachedMSAALoadInputDescSet;
    }

    fCachedMSAALoadInputDescSet.reset(gpu->resourceProvider().getInputDescriptorSet());
    if (!fCachedMSAALoadInputDescSet) {
        return nullptr;
    }

    write_input_desc_set(gpu,
                         this->framebufferView()->imageView(),
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         *fCachedMSAALoadInputDescSet->descriptorSet());

    return fCachedMSAALoadInputDescSet;
}

GrVkGpu* GrVkImage::getVkGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrVkGpu*>(this->getGpu());
}

#if defined(GR_TEST_UTILS)
void GrVkImage::setCurrentQueueFamilyToGraphicsQueue(GrVkGpu* gpu) {
    skgpu::MutableTextureStates::SetVkQueueFamilyIndex(fMutableState.get(), gpu->queueIndex());
}
#endif
