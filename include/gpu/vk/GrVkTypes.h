
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTypes_DEFINED
#define GrVkTypes_DEFINED

#include <functional>
#include "GrTypes.h"
#include "GrVkDefines.h"

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
struct GrVkImageInfo {
    VkImage        fImage;
    GrVkAlloc      fAlloc;
    VkImageTiling  fImageTiling;
    VkImageLayout  fImageLayout;
    VkFormat       fFormat;
    uint32_t       fLevelCount;
    uint32_t       fCurrentQueueFamily;

    GrVkImageInfo()
            : fImage(VK_NULL_HANDLE)
            , fAlloc()
            , fImageTiling(VK_IMAGE_TILING_OPTIMAL)
            , fImageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
            , fFormat(VK_FORMAT_UNDEFINED)
            , fLevelCount(0)
            , fCurrentQueueFamily(VK_QUEUE_FAMILY_IGNORED) {}

    GrVkImageInfo(VkImage image, GrVkAlloc alloc, VkImageTiling imageTiling, VkImageLayout layout,
                  VkFormat format, uint32_t levelCount,
                  uint32_t currentQueueFamily = VK_QUEUE_FAMILY_IGNORED)
            : fImage(image)
            , fAlloc(alloc)
            , fImageTiling(imageTiling)
            , fImageLayout(layout)
            , fFormat(format)
            , fLevelCount(levelCount)
            , fCurrentQueueFamily(currentQueueFamily) {}

    GrVkImageInfo(const GrVkImageInfo& info, VkImageLayout layout)
            : fImage(info.fImage)
            , fAlloc(info.fAlloc)
            , fImageTiling(info.fImageTiling)
            , fImageLayout(layout)
            , fFormat(info.fFormat)
            , fLevelCount(info.fLevelCount)
            , fCurrentQueueFamily(info.fCurrentQueueFamily) {}

    // This gives a way for a client to update the layout of the Image if they change the layout
    // while we're still holding onto the wrapped texture. They will first need to get a handle
    // to our internal GrVkImageInfo by calling getTextureHandle on a GrVkTexture.
    void updateImageLayout(VkImageLayout layout) { fImageLayout = layout; }

    bool operator==(const GrVkImageInfo& that) const {
        return fImage == that.fImage && fAlloc == that.fAlloc &&
               fImageTiling == that.fImageTiling && fImageLayout == that.fImageLayout &&
               fFormat == that.fFormat && fLevelCount == that.fLevelCount;
    }
};

using GrVkGetProc = std::function<PFN_vkVoidFunction(
        const char*, // function name
        VkInstance,  // instance or VK_NULL_HANDLE
        VkDevice     // device or VK_NULL_HANDLE
        )>;

/**
 * This object is passed in an argument to drawVulkan calls on an SkDrawable. The client will use
 * this call to inject manual Vulkan calls into our stream of GPU draws. There are two modes that
 * this call runs in. The first is a version where draws between Ganesh and the client are
 * synchonized by semaphores. This is used when SkDrawable::requiresFlushBeforeDraw() returns true.
 * The second mode does not use any sort of synchronization. For perfomance, the second mode
 * is preferred if it can be used.
 *
 * In both modes, the client may submit command buffers to the queue which do not use the passed
 * VkImage. This may be useful for when offscreen draws need to be performed that will then be
 * sampled into the VkImage.
 *
 * In the first mode, the client may generate their own command buffers where they draw to the
 * VkImage. In this mode the two VkSemaphore objects will be valid and the command buffer and
 * renderpass will be VK_NULL_HANDLEs. The client also must honor the following two rules on
 * semaphores:
 *   1) Before using the passed in VkImage in any way the client must wait on fWaitSemaphore
 *   2) After finishing all work related to VkImage the client must signal the fSignalSemaphore
 * The client is responsible for destroying the fWaitSemaphore, and they must no destroy the
 * fSignalSemaphore. Additionally the client may change the layout of the image passed in, but they
 * must return the image to the value of fLayout passed in.
 *
 * In the second mode, the client must use the passed in secondary command buffer to submit draws to
 * fImage. The passed in VkRenderPass is compatible to the one that will be used so that the client
 * can generate VkPipeline objects. The attachment index is also passed in. The semaphores will be
 * VK_NULL_HANDLEs since no synchronization is needed as the draw calls will be added directly into
 * our stream of draws. The VkImageLayout will be VK_IMAGE_COLOR_ATTACHMENT_OPTIMAL and the client
 * must not change the layout.
 *
 * The caller additionally can set fBounds to the bounds of the VkImage they drew to. If the client
 * does not do this we will assume the entire VkImage is dirty which could be a perfomance hit.
 */
struct GrVkClientDrawableInfo {
    VkCommandBuffer fSecondaryCommandBuffer;
    uint32_t        fColorAttachmentIndex;
    VkRenderPass    fCompatibleRenderPass;
    uint32_t        fImageAttachmentIndex;
    VkFormat        fFormat;
    VkRect2D*       fDrawBounds;
};

#endif
