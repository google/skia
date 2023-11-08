
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTypes_DEFINED
#define GrVkTypes_DEFINED

#include "include/gpu/GpuTypes.h"
#include "include/gpu/vk/VulkanTypes.h"

using GrVkBackendMemory = skgpu::VulkanBackendMemory;
using GrVkAlloc = skgpu::VulkanAlloc;
using GrVkYcbcrConversionInfo = skgpu::VulkanYcbcrConversionInfo;

/*
 * When wrapping a GrBackendTexture or GrBackendRendenderTarget, the fCurrentQueueFamily should
 * either be VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_EXTERNAL, or VK_QUEUE_FAMILY_FOREIGN_EXT. If
 * fSharingMode is VK_SHARING_MODE_EXCLUSIVE then fCurrentQueueFamily can also be the graphics
 * queue index passed into Skia.
 */
struct GrVkImageInfo {
    VkImage                  fImage = VK_NULL_HANDLE;
    skgpu::VulkanAlloc       fAlloc;
    VkImageTiling            fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageLayout            fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkFormat                 fFormat = VK_FORMAT_UNDEFINED;
    VkImageUsageFlags        fImageUsageFlags = 0;
    uint32_t                 fSampleCount = 1;
    uint32_t                 fLevelCount = 0;
    uint32_t                 fCurrentQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    skgpu::Protected         fProtected = skgpu::Protected::kNo;
    GrVkYcbcrConversionInfo  fYcbcrConversionInfo;
    VkSharingMode            fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    bool                     fPartOfSwapchainOrAndroidWindow = false;
#endif

    bool operator==(const GrVkImageInfo& that) const {
        bool equal = fImage == that.fImage && fAlloc == that.fAlloc &&
                     fImageTiling == that.fImageTiling &&
                     fImageLayout == that.fImageLayout &&
                     fFormat == that.fFormat &&
                     fImageUsageFlags == that.fImageUsageFlags &&
                     fSampleCount == that.fSampleCount &&
                     fLevelCount == that.fLevelCount &&
                     fCurrentQueueFamily == that.fCurrentQueueFamily &&
                     fProtected == that.fProtected &&
                     fYcbcrConversionInfo == that.fYcbcrConversionInfo &&
                     fSharingMode == that.fSharingMode;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        equal = equal && (fPartOfSwapchainOrAndroidWindow == that.fPartOfSwapchainOrAndroidWindow);
#endif
        return equal;
    }
};

using GrVkGetProc = skgpu::VulkanGetProc;

/**
 * This object is wrapped in a GrBackendDrawableInfo and passed in as an argument to
 * drawBackendGpu() calls on an SkDrawable. The drawable will use this info to inject direct
 * Vulkan calls into our stream of GPU draws.
 *
 * The SkDrawable is given a secondary VkCommandBuffer in which to record draws. The GPU backend
 * will then execute that command buffer within a render pass it is using for its own draws. The
 * drawable is also given the attachment of the color index, a compatible VkRenderPass, and the
 * VkFormat of the color attachment so that it can make VkPipeline objects for the draws. The
 * SkDrawable must not alter the state of the VkRenderpass or sub pass.
 *
 * Additionally, the SkDrawable may fill in the passed in fDrawBounds with the bounds of the draws
 * that it submits to the command buffer. This will be used by the GPU backend for setting the
 * bounds in vkCmdBeginRenderPass. If fDrawBounds is not updated, we will assume that the entire
 * attachment may have been written to.
 *
 * The SkDrawable is always allowed to create its own command buffers and submit them to the queue
 * to render offscreen textures which will be sampled in draws added to the passed in
 * VkCommandBuffer. If this is done the SkDrawable is in charge of adding the required memory
 * barriers to the queue for the sampled images since the Skia backend will not do this.
 */
struct GrVkDrawableInfo {
    VkCommandBuffer fSecondaryCommandBuffer;
    uint32_t        fColorAttachmentIndex;
    VkRenderPass    fCompatibleRenderPass;
    VkFormat        fFormat;
    VkRect2D*       fDrawBounds;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    bool            fFromSwapchainOrAndroidWindow;
#endif
};

struct GrVkSurfaceInfo {
    uint32_t fSampleCount = 1;
    uint32_t fLevelCount = 0;
    skgpu::Protected fProtected = skgpu::Protected::kNo;

    VkImageTiling fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormat fFormat = VK_FORMAT_UNDEFINED;
    VkImageUsageFlags fImageUsageFlags = 0;
    GrVkYcbcrConversionInfo fYcbcrConversionInfo;
    VkSharingMode fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
};

#endif
