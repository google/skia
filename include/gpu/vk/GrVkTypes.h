
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTypes_DEFINED
#define GrVkTypes_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/vk/GrVkVulkan.h"

#ifndef VK_VERSION_1_1
#error Skia requires the use of Vulkan 1.1 headers
#endif

#include <functional>
#include "include/gpu/GrTypes.h"

typedef intptr_t GrVkBackendMemory;

/**
 * Types for interacting with Vulkan resources created externally to Skia. GrBackendObjects for
 * Vulkan textures are really const GrVkImageInfo*
 */
struct GrVkAlloc {
    GrVkAlloc()
            : fMemory(VK_NULL_HANDLE)
            , fOffset(0)
            , fSize(0)
            , fFlags(0)
            , fBackendMemory(0)
            , fUsesSystemHeap(false) {}

    GrVkAlloc(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, uint32_t flags)
            : fMemory(memory)
            , fOffset(offset)
            , fSize(size)
            , fFlags(flags)
            , fBackendMemory(0)
            , fUsesSystemHeap(false) {}

    VkDeviceMemory    fMemory;  // can be VK_NULL_HANDLE iff is an RT and is borrowed
    VkDeviceSize      fOffset;
    VkDeviceSize      fSize;    // this can be indeterminate iff Tex uses borrow semantics
    uint32_t          fFlags;
    GrVkBackendMemory fBackendMemory; // handle to memory allocated via GrVkMemoryAllocator.

    enum Flag {
        kNoncoherent_Flag = 0x1,   // memory must be flushed to device after mapping
        kMappable_Flag    = 0x2,   // memory is able to be mapped.
    };

    bool operator==(const GrVkAlloc& that) const {
        return fMemory == that.fMemory && fOffset == that.fOffset && fSize == that.fSize &&
               fFlags == that.fFlags && fUsesSystemHeap == that.fUsesSystemHeap;
    }

private:
    friend class GrVkHeap; // For access to usesSystemHeap
    bool fUsesSystemHeap;
};

// This struct is used to pass in the necessary information to create a VkSamplerYcbcrConversion
// object for an VkExternalFormatANDROID.
struct GrVkYcbcrConversionInfo {
    GrVkYcbcrConversionInfo()
            : fFormat(VK_FORMAT_UNDEFINED)
            , fExternalFormat(0)
            , fYcbcrModel(VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY)
            , fYcbcrRange(VK_SAMPLER_YCBCR_RANGE_ITU_FULL)
            , fXChromaOffset(VK_CHROMA_LOCATION_COSITED_EVEN)
            , fYChromaOffset(VK_CHROMA_LOCATION_COSITED_EVEN)
            , fChromaFilter(VK_FILTER_NEAREST)
            , fForceExplicitReconstruction(false) {}

    GrVkYcbcrConversionInfo(VkFormat format,
                            int64_t externalFormat,
                            VkSamplerYcbcrModelConversion ycbcrModel,
                            VkSamplerYcbcrRange ycbcrRange,
                            VkChromaLocation xChromaOffset,
                            VkChromaLocation yChromaOffset,
                            VkFilter chromaFilter,
                            VkBool32 forceExplicitReconstruction,
                            VkFormatFeatureFlags formatFeatures)
            : fFormat(format)
            , fExternalFormat(externalFormat)
            , fYcbcrModel(ycbcrModel)
            , fYcbcrRange(ycbcrRange)
            , fXChromaOffset(xChromaOffset)
            , fYChromaOffset(yChromaOffset)
            , fChromaFilter(chromaFilter)
            , fForceExplicitReconstruction(forceExplicitReconstruction)
            , fFormatFeatures(formatFeatures) {
        SkASSERT(fYcbcrModel != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY);
        // Either format or externalFormat must be specified.
        SkASSERT((fFormat != VK_FORMAT_UNDEFINED) ^ (externalFormat != 0));
    }

    GrVkYcbcrConversionInfo(VkSamplerYcbcrModelConversion ycbcrModel,
                            VkSamplerYcbcrRange ycbcrRange,
                            VkChromaLocation xChromaOffset,
                            VkChromaLocation yChromaOffset,
                            VkFilter chromaFilter,
                            VkBool32 forceExplicitReconstruction,
                            int64_t externalFormat,
                            VkFormatFeatureFlags externalFormatFeatures)
            : GrVkYcbcrConversionInfo(VK_FORMAT_UNDEFINED, externalFormat, ycbcrModel, ycbcrRange,
                                      xChromaOffset, yChromaOffset, chromaFilter,
                                      forceExplicitReconstruction, externalFormatFeatures) {}

    bool operator==(const GrVkYcbcrConversionInfo& that) const {
        // Invalid objects are not required to have all other fields initialized or matching.
        if (!this->isValid() && !that.isValid()) {
            return true;
        }
        return this->fFormat == that.fFormat &&
               this->fExternalFormat == that.fExternalFormat &&
               this->fYcbcrModel == that.fYcbcrModel &&
               this->fYcbcrRange == that.fYcbcrRange &&
               this->fXChromaOffset == that.fXChromaOffset &&
               this->fYChromaOffset == that.fYChromaOffset &&
               this->fChromaFilter == that.fChromaFilter &&
               this->fForceExplicitReconstruction == that.fForceExplicitReconstruction;
    }
    bool operator!=(const GrVkYcbcrConversionInfo& that) const { return !(*this == that); }

    bool isValid() const { return fYcbcrModel != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY; }

    // Format of the source image. Must be set to VK_FORMAT_UNDEFINED for external images or
    // a valid image format otherwise.
    VkFormat                         fFormat;

    // The external format. Must be non-zero for external images, zero otherwise.
    // Should be compatible to be used in a VkExternalFormatANDROID struct.
    int64_t                          fExternalFormat;

    VkSamplerYcbcrModelConversion    fYcbcrModel;
    VkSamplerYcbcrRange              fYcbcrRange;
    VkChromaLocation                 fXChromaOffset;
    VkChromaLocation                 fYChromaOffset;
    VkFilter                         fChromaFilter;
    VkBool32                         fForceExplicitReconstruction;

    // For external images format features here should be those returned by a call to
    // vkAndroidHardwareBufferFormatPropertiesANDROID
    VkFormatFeatureFlags             fFormatFeatures;
};

/*
 * When wrapping a GrBackendTexture or GrBackendRendenderTarget, the fCurrentQueueFamily should
 * either be VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_EXTERNAL, or VK_QUEUE_FAMILY_FOREIGN_EXT. If
 * fSharingMode is VK_SHARING_MODE_EXCLUSIVE then fCurrentQueueFamily can also be the graphics
 * queue index passed into Skia.
 */
struct GrVkImageInfo {
    VkImage                  fImage;
    GrVkAlloc                fAlloc;
    VkImageTiling            fImageTiling;
    VkImageLayout            fImageLayout;
    VkFormat                 fFormat;
    uint32_t                 fLevelCount;
    uint32_t                 fCurrentQueueFamily;
    GrProtected              fProtected;
    GrVkYcbcrConversionInfo  fYcbcrConversionInfo;
    VkSharingMode            fSharingMode;

    GrVkImageInfo()
            : fImage(VK_NULL_HANDLE)
            , fAlloc()
            , fImageTiling(VK_IMAGE_TILING_OPTIMAL)
            , fImageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            , fFormat(VK_FORMAT_UNDEFINED)
            , fLevelCount(0)
            , fCurrentQueueFamily(VK_QUEUE_FAMILY_IGNORED)
            , fProtected(GrProtected::kNo)
            , fYcbcrConversionInfo()
            , fSharingMode(VK_SHARING_MODE_EXCLUSIVE) {}

    GrVkImageInfo(VkImage image,
                  GrVkAlloc alloc,
                  VkImageTiling imageTiling,
                  VkImageLayout layout,
                  VkFormat format,
                  uint32_t levelCount,
                  uint32_t currentQueueFamily = VK_QUEUE_FAMILY_IGNORED,
                  GrProtected isProtected = GrProtected::kNo,
                  GrVkYcbcrConversionInfo ycbcrConversionInfo = GrVkYcbcrConversionInfo(),
                  VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
            : fImage(image)
            , fAlloc(alloc)
            , fImageTiling(imageTiling)
            , fImageLayout(layout)
            , fFormat(format)
            , fLevelCount(levelCount)
            , fCurrentQueueFamily(currentQueueFamily)
            , fProtected(isProtected)
            , fYcbcrConversionInfo(ycbcrConversionInfo)
            , fSharingMode(sharingMode) {}

    GrVkImageInfo(const GrVkImageInfo& info, VkImageLayout layout, uint32_t familyQueueIndex)
            : fImage(info.fImage)
            , fAlloc(info.fAlloc)
            , fImageTiling(info.fImageTiling)
            , fImageLayout(layout)
            , fFormat(info.fFormat)
            , fLevelCount(info.fLevelCount)
            , fCurrentQueueFamily(familyQueueIndex)
            , fProtected(info.fProtected)
            , fYcbcrConversionInfo(info.fYcbcrConversionInfo)
            , fSharingMode(info.fSharingMode) {}

#if GR_TEST_UTILS
    bool operator==(const GrVkImageInfo& that) const {
        return fImage == that.fImage && fAlloc == that.fAlloc &&
               fImageTiling == that.fImageTiling && fImageLayout == that.fImageLayout &&
               fFormat == that.fFormat && fLevelCount == that.fLevelCount &&
               fCurrentQueueFamily == that.fCurrentQueueFamily && fProtected == that.fProtected &&
               fYcbcrConversionInfo == that.fYcbcrConversionInfo &&
               fSharingMode == that.fSharingMode;
    }
#endif
};

using GrVkGetProc = std::function<PFN_vkVoidFunction(
        const char*, // function name
        VkInstance,  // instance or VK_NULL_HANDLE
        VkDevice     // device or VK_NULL_HANDLE
        )>;

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
 *
 * The VkImage is informational only and should not be used or modified in any ways.
 */
struct GrVkDrawableInfo {
    VkCommandBuffer fSecondaryCommandBuffer;
    uint32_t        fColorAttachmentIndex;
    VkRenderPass    fCompatibleRenderPass;
    VkFormat        fFormat;
    VkRect2D*       fDrawBounds;
    VkImage         fImage;
};

#endif
