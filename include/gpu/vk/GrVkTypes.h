
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTypes_DEFINED
#define GrVkTypes_DEFINED

#ifdef SK_VULKAN
#include <vulkan/vulkan_core.h>
#else
#include "../../third_party/vulkan/vulkan/vulkan_core.h"
#endif
#ifndef VK_VERSION_1_1
#error Skia requires the use of Vulkan 1.1 headers
#endif

#include <functional>
#include "GrTypes.h"

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
            : fYcbcrModel(VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY)
            , fYcbcrRange(VK_SAMPLER_YCBCR_RANGE_ITU_FULL)
            , fXChromaOffset(VK_CHROMA_LOCATION_COSITED_EVEN)
            , fYChromaOffset(VK_CHROMA_LOCATION_COSITED_EVEN)
            , fChromaFilter(VK_FILTER_NEAREST)
            , fForceExplicitReconstruction(false)
            , fExternalFormat(0)
            , fExternalFormatFeatures(0) {}

    GrVkYcbcrConversionInfo(VkSamplerYcbcrModelConversion ycbcrModel,
                            VkSamplerYcbcrRange ycbcrRange,
                            VkChromaLocation xChromaOffset,
                            VkChromaLocation yChromaOffset,
                            VkFilter chromaFilter,
                            VkBool32 forceExplicitReconstruction,
                            uint64_t externalFormat,
                            VkFormatFeatureFlags externalFormatFeatures)
            : fYcbcrModel(ycbcrModel)
            , fYcbcrRange(ycbcrRange)
            , fXChromaOffset(xChromaOffset)
            , fYChromaOffset(yChromaOffset)
            , fChromaFilter(chromaFilter)
            , fForceExplicitReconstruction(forceExplicitReconstruction)
            , fExternalFormat(externalFormat)
            , fExternalFormatFeatures(externalFormatFeatures) {
        SkASSERT(fExternalFormat);
    }

    bool operator==(const GrVkYcbcrConversionInfo& that) const {
        // Invalid objects are not required to have all other fields intialized or matching.
        if (!this->isValid() && !that.isValid()) {
            return true;
        }
        return this->fYcbcrModel == that.fYcbcrModel &&
               this->fYcbcrRange == that.fYcbcrRange &&
               this->fXChromaOffset == that.fXChromaOffset &&
               this->fYChromaOffset == that.fYChromaOffset &&
               this->fChromaFilter == that.fChromaFilter &&
               this->fForceExplicitReconstruction == that.fForceExplicitReconstruction &&
               this->fExternalFormat == that.fExternalFormat;
        // We don't check fExternalFormatFeatures here since all matching external formats must have
        // the same format features at least in terms of how they effect ycbcr sampler conversion.
    }
    bool operator!=(const GrVkYcbcrConversionInfo& that) const { return !(*this == that); }

    bool isValid() const { return fExternalFormat != 0; }

    VkSamplerYcbcrModelConversion    fYcbcrModel;
    VkSamplerYcbcrRange              fYcbcrRange;
    VkChromaLocation                 fXChromaOffset;
    VkChromaLocation                 fYChromaOffset;
    VkFilter                         fChromaFilter;
    VkBool32                         fForceExplicitReconstruction;
    // The external format should be compatible to be used in a VkExternalFormatANDROID struct
    uint64_t                         fExternalFormat;
    // The format features here should be those returned by a call to
    // vkAndroidHardwareBufferFormatPropertiesANDROID
    VkFormatFeatureFlags             fExternalFormatFeatures;
};

struct GrVkImageInfo {
    VkImage                  fImage;
    GrVkAlloc                fAlloc;
    VkImageTiling            fImageTiling;
    VkImageLayout            fImageLayout;
    VkFormat                 fFormat;
    uint32_t                 fLevelCount;
    uint32_t                 fCurrentQueueFamily;
    GrVkYcbcrConversionInfo  fYcbcrConversionInfo;

    GrVkImageInfo()
            : fImage(VK_NULL_HANDLE)
            , fAlloc()
            , fImageTiling(VK_IMAGE_TILING_OPTIMAL)
            , fImageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            , fFormat(VK_FORMAT_UNDEFINED)
            , fLevelCount(0)
            , fCurrentQueueFamily(VK_QUEUE_FAMILY_IGNORED)
            , fYcbcrConversionInfo() {}

    GrVkImageInfo(VkImage image, GrVkAlloc alloc, VkImageTiling imageTiling, VkImageLayout layout,
                  VkFormat format, uint32_t levelCount,
                  uint32_t currentQueueFamily = VK_QUEUE_FAMILY_IGNORED,
                  GrVkYcbcrConversionInfo ycbcrConversionInfo = GrVkYcbcrConversionInfo())
            : fImage(image)
            , fAlloc(alloc)
            , fImageTiling(imageTiling)
            , fImageLayout(layout)
            , fFormat(format)
            , fLevelCount(levelCount)
            , fCurrentQueueFamily(currentQueueFamily)
            , fYcbcrConversionInfo(ycbcrConversionInfo) {}

    GrVkImageInfo(const GrVkImageInfo& info, VkImageLayout layout)
            : fImage(info.fImage)
            , fAlloc(info.fAlloc)
            , fImageTiling(info.fImageTiling)
            , fImageLayout(layout)
            , fFormat(info.fFormat)
            , fLevelCount(info.fLevelCount)
            , fCurrentQueueFamily(info.fCurrentQueueFamily)
            , fYcbcrConversionInfo(info.fYcbcrConversionInfo) {}

    // This gives a way for a client to update the layout of the Image if they change the layout
    // while we're still holding onto the wrapped texture. They will first need to get a handle
    // to our internal GrVkImageInfo by calling getTextureHandle on a GrVkTexture.
    void updateImageLayout(VkImageLayout layout) { fImageLayout = layout; }

    bool operator==(const GrVkImageInfo& that) const {
        return fImage == that.fImage && fAlloc == that.fAlloc &&
               fImageTiling == that.fImageTiling && fImageLayout == that.fImageLayout &&
               fFormat == that.fFormat && fLevelCount == that.fLevelCount &&
               fCurrentQueueFamily == that.fCurrentQueueFamily &&
               fYcbcrConversionInfo == that.fYcbcrConversionInfo;
    }
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
 */
struct GrVkDrawableInfo {
    VkCommandBuffer fSecondaryCommandBuffer;
    uint32_t        fColorAttachmentIndex;
    VkRenderPass    fCompatibleRenderPass;
    VkFormat        fFormat;
    VkRect2D*       fDrawBounds;
};

#endif
