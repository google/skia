/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkCommandBuffer_DEFINED
#define GrVkCommandBuffer_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrManagedResource.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkSemaphore.h"
#include "src/gpu/vk/GrVkUtil.h"

class GrVkBuffer;
class GrVkFramebuffer;
class GrVkIndexBuffer;
class GrVkImage;
class GrVkPipeline;
class GrVkPipelineState;
class GrVkRenderPass;
class GrVkRenderTarget;
class GrVkTransferBuffer;
class GrVkVertexBuffer;

class GrVkCommandBuffer {
public:
    virtual ~GrVkCommandBuffer() {}

    void invalidateState();

    ////////////////////////////////////////////////////////////////////////////
    // CommandBuffer commands
    ////////////////////////////////////////////////////////////////////////////
    enum BarrierType {
        kBufferMemory_BarrierType,
        kImageMemory_BarrierType
    };

    void pipelineBarrier(const GrVkGpu* gpu,
                         const GrManagedResource* resource,
                         VkPipelineStageFlags srcStageMask,
                         VkPipelineStageFlags dstStageMask,
                         bool byRegion,
                         BarrierType barrierType,
                         void* barrier);

    void bindInputBuffer(GrVkGpu* gpu, uint32_t binding, const GrVkVertexBuffer* vbuffer);

    void bindIndexBuffer(GrVkGpu* gpu, const GrVkIndexBuffer* ibuffer);

    void bindPipeline(const GrVkGpu* gpu, const GrVkPipeline* pipeline);

    void bindDescriptorSets(const GrVkGpu* gpu,
                            GrVkPipelineState*,
                            VkPipelineLayout layout,
                            uint32_t firstSet,
                            uint32_t setCount,
                            const VkDescriptorSet* descriptorSets,
                            uint32_t dynamicOffsetCount,
                            const uint32_t* dynamicOffsets);

    void setViewport(const GrVkGpu* gpu,
                     uint32_t firstViewport,
                     uint32_t viewportCount,
                     const VkViewport* viewports);

    void setScissor(const GrVkGpu* gpu,
                    uint32_t firstScissor,
                    uint32_t scissorCount,
                    const VkRect2D* scissors);

    void setBlendConstants(const GrVkGpu* gpu, const float blendConstants[4]);

    // Commands that only work inside of a render pass
    void clearAttachments(const GrVkGpu* gpu,
                          int numAttachments,
                          const VkClearAttachment* attachments,
                          int numRects,
                          const VkClearRect* clearRects);

    void drawIndexed(const GrVkGpu* gpu,
                     uint32_t indexCount,
                     uint32_t instanceCount,
                     uint32_t firstIndex,
                     int32_t vertexOffset,
                     uint32_t firstInstance);

    void draw(const GrVkGpu* gpu,
              uint32_t vertexCount,
              uint32_t instanceCount,
              uint32_t firstVertex,
              uint32_t firstInstance);

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution
    void addResource(const GrManagedResource* resource) {
        SkASSERT(resource);
        resource->ref();
        resource->notifyQueuedForWorkOnGpu();
        fTrackedResources.append(1, &resource);
    }

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution. When it is released, it will signal that the resource can be recycled for reuse.
    void addRecycledResource(const GrRecycledResource* resource) {
        resource->ref();
        resource->notifyQueuedForWorkOnGpu();
        fTrackedRecycledResources.append(1, &resource);
    }

    void releaseResources();

    void freeGPUData(const GrGpu* gpu, VkCommandPool pool) const;

    bool hasWork() const { return fHasWork; }

protected:
    GrVkCommandBuffer(VkCommandBuffer cmdBuffer, bool isWrapped = false)
            : fCmdBuffer(cmdBuffer)
            , fIsWrapped(isWrapped) {
        fTrackedResources.setReserve(kInitialTrackedResourcesCount);
        fTrackedRecycledResources.setReserve(kInitialTrackedResourcesCount);
        this->invalidateState();
    }

    bool isWrapped() const { return fIsWrapped; }

    void addingWork(const GrVkGpu* gpu);

    void submitPipelineBarriers(const GrVkGpu* gpu);

    SkTDArray<const GrManagedResource*>   fTrackedResources;
    SkTDArray<const GrRecycledResource*>  fTrackedRecycledResources;

    // Tracks whether we are in the middle of a command buffer begin/end calls and thus can add
    // new commands to the buffer;
    bool                      fIsActive = false;
    bool                      fHasWork = false;

    // Stores a pointer to the current active render pass (i.e. begin has been called but not
    // end). A nullptr means there is no active render pass. The GrVKCommandBuffer does not own
    // the render pass.
    const GrVkRenderPass*     fActiveRenderPass = nullptr;

    VkCommandBuffer           fCmdBuffer;

private:
    static const int kInitialTrackedResourcesCount = 32;

    virtual void onReleaseResources() {}
    virtual void onFreeGPUData(const GrVkGpu* gpu) const = 0;

    static constexpr uint32_t kMaxInputBuffers = 2;

    VkBuffer fBoundInputBuffers[kMaxInputBuffers];
    VkBuffer fBoundIndexBuffer;

    // When resetting the command buffer, we remove the tracked resources from their arrays, and
    // we prefer to not free all the memory every time so usually we just rewind. However, to avoid
    // all arrays growing to the max size, after so many resets we'll do a full reset of the tracked
    // resource arrays.
    static const int kNumRewindResetsBeforeFullReset = 8;
    int              fNumResets = 0;

    // Cached values used for dynamic state updates
    VkViewport fCachedViewport;
    VkRect2D   fCachedScissor;
    float      fCachedBlendConstant[4];

    // Tracking of memory barriers so that we can submit them all in a batch together.
    SkSTArray<4, VkBufferMemoryBarrier> fBufferBarriers;
    SkSTArray<1, VkImageMemoryBarrier> fImageBarriers;
    bool fBarriersByRegion = false;
    VkPipelineStageFlags fSrcStageMask = 0;
    VkPipelineStageFlags fDstStageMask = 0;

    bool fIsWrapped;
};

class GrVkSecondaryCommandBuffer;

class GrVkPrimaryCommandBuffer : public GrVkCommandBuffer {
public:
    ~GrVkPrimaryCommandBuffer() override;

    static GrVkPrimaryCommandBuffer* Create(GrVkGpu* gpu, VkCommandPool cmdPool);

    void begin(GrVkGpu* gpu);
    void end(GrVkGpu* gpu);

    // Begins render pass on this command buffer. The framebuffer from GrVkRenderTarget will be used
    // in the render pass.
    bool beginRenderPass(GrVkGpu* gpu,
                         const GrVkRenderPass* renderPass,
                         const VkClearValue clearValues[],
                         GrVkRenderTarget* target,
                         const SkIRect& bounds,
                         bool forSecondaryCB);
    void endRenderPass(const GrVkGpu* gpu);

    // Submits the SecondaryCommandBuffer into this command buffer. It is required that we are
    // currently inside a render pass that is compatible with the one used to create the
    // SecondaryCommandBuffer.
    void executeCommands(const GrVkGpu* gpu,
                         std::unique_ptr<GrVkSecondaryCommandBuffer> secondaryBuffer);

    // Commands that only work outside of a render pass
    void clearColorImage(const GrVkGpu* gpu,
                         GrVkImage* image,
                         const VkClearColorValue* color,
                         uint32_t subRangeCount,
                         const VkImageSubresourceRange* subRanges);

    void clearDepthStencilImage(const GrVkGpu* gpu,
                                GrVkImage* image,
                                const VkClearDepthStencilValue* color,
                                uint32_t subRangeCount,
                                const VkImageSubresourceRange* subRanges);

    void copyImage(const GrVkGpu* gpu,
                   GrVkImage* srcImage,
                   VkImageLayout srcLayout,
                   GrVkImage* dstImage,
                   VkImageLayout dstLayout,
                   uint32_t copyRegionCount,
                   const VkImageCopy* copyRegions);

    void blitImage(const GrVkGpu* gpu,
                   const GrManagedResource* srcResource,
                   VkImage srcImage,
                   VkImageLayout srcLayout,
                   const GrManagedResource* dstResource,
                   VkImage dstImage,
                   VkImageLayout dstLayout,
                   uint32_t blitRegionCount,
                   const VkImageBlit* blitRegions,
                   VkFilter filter);

    void blitImage(const GrVkGpu* gpu,
                   const GrVkImage& srcImage,
                   const GrVkImage& dstImage,
                   uint32_t blitRegionCount,
                   const VkImageBlit* blitRegions,
                   VkFilter filter);

    void copyImageToBuffer(const GrVkGpu* gpu,
                           GrVkImage* srcImage,
                           VkImageLayout srcLayout,
                           GrVkTransferBuffer* dstBuffer,
                           uint32_t copyRegionCount,
                           const VkBufferImageCopy* copyRegions);

    void copyBufferToImage(const GrVkGpu* gpu,
                           GrVkTransferBuffer* srcBuffer,
                           GrVkImage* dstImage,
                           VkImageLayout dstLayout,
                           uint32_t copyRegionCount,
                           const VkBufferImageCopy* copyRegions);

    void copyBuffer(GrVkGpu* gpu,
                    GrVkBuffer* srcBuffer,
                    GrVkBuffer* dstBuffer,
                    uint32_t regionCount,
                    const VkBufferCopy* regions);

    void updateBuffer(GrVkGpu* gpu,
                      GrVkBuffer* dstBuffer,
                      VkDeviceSize dstOffset,
                      VkDeviceSize dataSize,
                      const void* data);

    void resolveImage(GrVkGpu* gpu,
                      const GrVkImage& srcImage,
                      const GrVkImage& dstImage,
                      uint32_t regionCount,
                      const VkImageResolve* regions);

    bool submitToQueue(GrVkGpu* gpu, VkQueue queue,
                       SkTArray<GrVkSemaphore::Resource*>& signalSemaphores,
                       SkTArray<GrVkSemaphore::Resource*>& waitSemaphores);

    void forceSync(GrVkGpu* gpu);

    bool finished(GrVkGpu* gpu);

    void addFinishedProc(sk_sp<GrRefCntedCallback> finishedProc);

    void recycleSecondaryCommandBuffers(GrVkCommandPool* cmdPool);

private:
    explicit GrVkPrimaryCommandBuffer(VkCommandBuffer cmdBuffer)
        : INHERITED(cmdBuffer)
        , fSubmitFence(VK_NULL_HANDLE) {}

    void onFreeGPUData(const GrVkGpu* gpu) const override;

    void onReleaseResources() override;

    SkTArray<std::unique_ptr<GrVkSecondaryCommandBuffer>, true> fSecondaryCommandBuffers;
    VkFence                                                     fSubmitFence;
    SkTArray<sk_sp<GrRefCntedCallback>>                         fFinishedProcs;

    typedef GrVkCommandBuffer INHERITED;
};

class GrVkSecondaryCommandBuffer : public GrVkCommandBuffer {
public:
    static GrVkSecondaryCommandBuffer* Create(GrVkGpu* gpu, GrVkCommandPool* cmdPool);
    // Used for wrapping an external secondary command buffer.
    static GrVkSecondaryCommandBuffer* Create(VkCommandBuffer externalSecondaryCB);

    void begin(GrVkGpu* gpu, const GrVkFramebuffer* framebuffer,
               const GrVkRenderPass* compatibleRenderPass);
    void end(GrVkGpu* gpu);

    void recycle(GrVkCommandPool* cmdPool);

    VkCommandBuffer vkCommandBuffer() { return fCmdBuffer; }

private:
    explicit GrVkSecondaryCommandBuffer(VkCommandBuffer cmdBuffer, bool isWrapped)
        : INHERITED(cmdBuffer, isWrapped) {}

    void onFreeGPUData(const GrVkGpu* gpu) const override {}

    // Used for accessing fIsActive (on GrVkCommandBuffer)
    friend class GrVkPrimaryCommandBuffer;

    typedef GrVkCommandBuffer INHERITED;
};

#endif
