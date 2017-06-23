
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTypes_DEFINED
#define GrVkTypes_DEFINED

#include "GrTypes.h"
#include "SkRect.h"
#include "vk/GrVkDefines.h"

/**
 * KHR_debug
 */
/*typedef void (GR_GL_FUNCTION_TYPE* GrVkDEBUGPROC)(GrVkenum source,
                                                  GrVkenum type,
                                                  GrVkuint id,
                                                  GrVkenum severity,
                                                  GrVksizei length,
                                                  const GrVkchar* message,
                                                  const void* userParam);*/



///////////////////////////////////////////////////////////////////////////////
/**
 * Types for interacting with Vulkan resources created externally to Skia. GrBackendObjects for
 * Vulkan textures are really const GrVkImageInfo*
 */
struct GrVkAlloc {
    VkDeviceMemory fMemory;  // can be VK_NULL_HANDLE iff Tex is an RT and uses borrow semantics
    VkDeviceSize   fOffset;
    VkDeviceSize   fSize;    // this can be indeterminate iff Tex uses borrow semantics
    uint32_t       fFlags;

    enum Flag {
        kNoncoherent_Flag = 0x1,   // memory must be flushed to device after mapping
    };
};

struct GrVkImageInfo {
    /**
     * If the image's format is sRGB (GrVkFormatIsSRGB returns true), then the image must have
     * been created with VkImageCreateFlags containing VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.
     */
    VkImage        fImage;
    GrVkAlloc      fAlloc;
    VkImageTiling  fImageTiling;
    VkImageLayout  fImageLayout;
    VkFormat       fFormat;
    uint32_t       fLevelCount;

    // This gives a way for a client to update the layout of the Image if they change the layout
    // while we're still holding onto the wrapped texture. They will first need to get a handle
    // to our internal GrVkImageInfo by calling getTextureHandle on a GrVkTexture.
    void updateImageLayout(VkImageLayout layout) { fImageLayout = layout; }
};
GR_STATIC_ASSERT(sizeof(GrBackendObject) >= sizeof(const GrVkImageInfo*));

/**
 * This object is passed in an argument to drawVulkan calls on an SkDrawable. The idea is that a
 * client will use this call to inject manual Vulkan calls into our stream of GPU draws. There are
 * two modes that this call runs in. The first is a version where draws between Ganesh and the
 * client are synchonized by semaphores. Happens when SkDrawable::requiresFlushBeforeDraw() returns
 * true. The second mode does not use any sort of synchronization. For perfomance, the second mode
 * is preferred if it can be used.
 *
 * In both modes, the client should only use the passed in VkImage as a render targer and should not
 * use it for texturing or other operations. The passed in VkImageLayout lets the client know what
 * layout the image is in (this will usually be either COLOR_ATTACHMENT or GENERAL). The client
 * should not change the image layout.
 *
 * Additionally in both modes, the client may submit command buffers to the queue which do not use
 * the passed VkImage. This may be useful for when offscreen draws need to be performed that will
 * then be sampled into the VkImage.
 *
 * In the first mode, the client may generate their own command buffers where the draw to the
 * VkImage. In this mode the two VkSemaphore objects will be valid and the command buffer and
 * renderpass will be VK_NULL_HANDLEs. The client also must honor the following two rules on
 * semaphores:
 *   1) Before using the passed in VkImage in any way the client must wait on fWaitSemaphore
 *   2) After finishing all work related to VkImage the client must signal the fSignalSemaphore
 *
 * In the second mode, the client must use the passed in secondary command buffer to submit draws to
 * VkImage. The passed in VkRenderPass is compatible to the one that will be used so that the client
 * can generate VkPipeline objects. The attachment index is also passed in. The semaphores will be
 * VK_NULL_HANDLEs since no synchronization is needed as the draw calls will be added directly into
 * our stream of draws.
 *
 * The caller additionally can set fBounds to the bounds of the VkImage they drew to. If the client
 * does not do this we will assume the entire VkImage is dirty which could be a perfomance hit.
 */
struct GrVkClientDrawableInfo {
    VkDevice        fDevice;
    VkQueue         fQueue;
    uint32_t        fQueueFamilyIndex;
    VkImage         fImage;
    VkImageLayout   fLayout;
    // Semaphores are only valid if using the first mode described above, will be VK_NULL_HANDLE
    // otherwise.
    VkSemaphore     fWaitSemaphore;
    VkSemaphore     fSignalSemaphore;
    // RenderPass and CommandBuffer are only valid if using the second mode described above, will be
    // VK_NULL_HANDLE otherswise.
    VkCommandBuffer fCommandBuffer;
    VkRenderPass    fRenderPass;
    uint32_t        fImageAttachmentIndex;
    SkRect*         fBounds;
};

#endif
