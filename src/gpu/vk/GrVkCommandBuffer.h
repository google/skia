/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkCommandBuffer_DEFINED
#define GrVkCommandBuffer_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkResource.h"
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

class GrVkCommandBuffer : public GrVkResource {
public:
    void invalidateState();

    ////////////////////////////////////////////////////////////////////////////
    // CommandBuffer commands
    ////////////////////////////////////////////////////////////////////////////
    enum BarrierType {
        kBufferMemory_BarrierType,
        kImageMemory_BarrierType
    };

    void pipelineBarrier(const GrVkGpu* gpu,
                         const GrVkResource* resource,
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
                            GrVkPipelineLayout* layout,
                            uint32_t firstSet,
                            uint32_t setCount,
                            const VkDescriptorSet* descriptorSets,
                            uint32_t dynamicOffsetCount,
                            const uint32_t* dynamicOffsets);

    void bindDescriptorSets(const GrVkGpu* gpu,
                            const SkTArray<const GrVkRecycledResource*>&,
                            const SkTArray<const GrVkResource*>&,
                            GrVkPipelineLayout* layout,
                            uint32_t firstSet,
                            uint32_t setCount,
                            const VkDescriptorSet* descriptorSets,
                            uint32_t dynamicOffsetCount,
                            const uint32_t* dynamicOffsets);

    GrVkCommandPool* commandPool() { return fCmdPool; }

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
    void addResource(const GrVkResource* resource) {
        resource->ref();
        resource->notifyAddedToCommandBuffer();
        fTrackedResources.append(1, &resource);
    }

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution. When it is released, it will signal that the resource can be recycled for reuse.
    void addRecycledResource(const GrVkRecycledResource* resource) {
        resource->ref();
        resource->notifyAddedToCommandBuffer();
        fTrackedRecycledResources.append(1, &resource);
    }

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // recording.
    void addRecordingResource(const GrVkResource* resource) {
        resource->ref();
        resource->notifyAddedToCommandBuffer();
        fTrackedRecordingResources.append(1, &resource);
    }

    void releaseResources(GrVkGpu* gpu);

    bool hasWork() const { return fHasWork; }

protected:
        GrVkCommandBuffer(VkCommandBuffer cmdBuffer, GrVkCommandPool* cmdPool,
                          const GrVkRenderPass* rp = nullptr)
            : fIsActive(false)
            , fActiveRenderPass(rp)
            , fCmdBuffer(cmdBuffer)
            , fCmdPool(cmdPool)
            , fNumResets(0) {
            fTrackedResources.setReserve(kInitialTrackedResourcesCount);
            fTrackedRecycledResources.setReserve(kInitialTrackedResourcesCount);
            fTrackedRecordingResources.setReserve(kInitialTrackedResourcesCount);
            this->invalidateState();
        }

        bool isWrapped() const {
            return fCmdPool == nullptr;
        }

        void addingWork(const GrVkGpu* gpu);

        void submitPipelineBarriers(const GrVkGpu* gpu);

        SkTDArray<const GrVkResource*>          fTrackedResources;
        SkTDArray<const GrVkRecycledResource*>  fTrackedRecycledResources;
        SkTDArray<const GrVkResource*>          fTrackedRecordingResources;

        // Tracks whether we are in the middle of a command buffer begin/end calls and thus can add
        // new commands to the buffer;
        bool                      fIsActive;
        bool                      fHasWork = false;

        // Stores a pointer to the current active render pass (i.e. begin has been called but not
        // end). A nullptr means there is no active render pass. The GrVKCommandBuffer does not own
        // the render pass.
        const GrVkRenderPass*     fActiveRenderPass;

        VkCommandBuffer           fCmdBuffer;

        // Raw pointer, not refcounted. The command pool controls the command buffer's lifespan, so
        // it's guaranteed to outlive us.
        GrVkCommandPool*          fCmdPool;

private:
    static const int kInitialTrackedResourcesCount = 32;

    void freeGPUData(GrVkGpu* gpu) const final override;
    virtual void onFreeGPUData(GrVkGpu* gpu) const = 0;
    void abandonGPUData() const final override;
    virtual void onAbandonGPUData() const = 0;

    virtual void onReleaseResources(GrVkGpu* gpu) {}

    static constexpr uint32_t kMaxInputBuffers = 2;

    VkBuffer fBoundInputBuffers[kMaxInputBuffers];
    VkBuffer fBoundIndexBuffer;

    // When resetting the command buffer, we remove the tracked resources from their arrays, and
    // we prefer to not free all the memory every time so usually we just rewind. However, to avoid
    // all arrays growing to the max size, after so many resets we'll do a full reset of the tracked
    // resource arrays.
    static const int kNumRewindResetsBeforeFullReset = 8;
    int              fNumResets;

    // Cached values used for dynamic state updates
    VkViewport fCachedViewport;
    VkRect2D   fCachedScissor;
    float      fCachedBlendConstant[4];

#ifdef SK_DEBUG
    mutable bool fResourcesReleased = false;
#endif
    // Tracking of memory barriers so that we can submit them all in a batch together.
    SkSTArray<4, VkBufferMemoryBarrier> fBufferBarriers;
    SkSTArray<1, VkImageMemoryBarrier> fImageBarriers;
    bool fBarriersByRegion = false;
    VkPipelineStageFlags fSrcStageMask = 0;
    VkPipelineStageFlags fDstStageMask = 0;
};

class GrVkSecondaryCommandBuffer;

class GrVkPrimaryCommandBuffer : public GrVkCommandBuffer {
public:
    ~GrVkPrimaryCommandBuffer() override;

    static GrVkPrimaryCommandBuffer* Create(const GrVkGpu* gpu, GrVkCommandPool* cmdPool);

    void begin(const GrVkGpu* gpu);
    void end(GrVkGpu* gpu);

    // Begins render pass on this command buffer. The framebuffer from GrVkRenderTarget will be used
    // in the render pass.
    void beginRenderPass(const GrVkGpu* gpu,
                         const GrVkRenderPass* renderPass,
                         const VkClearValue clearValues[],
                         const GrVkRenderTarget& target,
                         const SkIRect& bounds,
                         bool forSecondaryCB);
    void endRenderPass(const GrVkGpu* gpu);

    // Submits the SecondaryCommandBuffer into this command buffer. It is required that we are
    // currently inside a render pass that is compatible with the one used to create the
    // SecondaryCommandBuffer.
    void executeCommands(const GrVkGpu* gpu,
                         GrVkSecondaryCommandBuffer* secondaryBuffer);

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
                   const GrVkResource* srcResource,
                   VkImage srcImage,
                   VkImageLayout srcLayout,
                   const GrVkResource* dstResource,
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

    void submitToQueue(const GrVkGpu* gpu, VkQueue queue, GrVkGpu::SyncQueue sync,
                       SkTArray<GrVkSemaphore::Resource*>& signalSemaphores,
                       SkTArray<GrVkSemaphore::Resource*>& waitSemaphores);
    bool finished(const GrVkGpu* gpu);

    void addFinishedProc(sk_sp<GrRefCntedCallback> finishedProc);

    void recycleSecondaryCommandBuffers();

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkPrimaryCommandBuffer: %d (%d refs)\n", fCmdBuffer, this->getRefCnt());
    }
#endif

private:
    explicit GrVkPrimaryCommandBuffer(VkCommandBuffer cmdBuffer, GrVkCommandPool* cmdPool)
        : INHERITED(cmdBuffer, cmdPool)
        , fSubmitFence(VK_NULL_HANDLE) {}

    void onFreeGPUData(GrVkGpu* gpu) const override;

    void onAbandonGPUData() const override;

    void onReleaseResources(GrVkGpu* gpu) override;

    SkTArray<GrVkSecondaryCommandBuffer*, true> fSecondaryCommandBuffers;
    VkFence                                     fSubmitFence;
    SkTArray<sk_sp<GrRefCntedCallback>>         fFinishedProcs;

    typedef GrVkCommandBuffer INHERITED;
};

class GrVkSecondaryCommandBuffer : public GrVkCommandBuffer {
public:
    static GrVkSecondaryCommandBuffer* Create(const GrVkGpu* gpu, GrVkCommandPool* cmdPool);
    // Used for wrapping an external secondary command buffer.
    static GrVkSecondaryCommandBuffer* Create(VkCommandBuffer externalSecondaryCB);

    void begin(const GrVkGpu* gpu, const GrVkFramebuffer* framebuffer,
               const GrVkRenderPass* compatibleRenderPass);
    void end(GrVkGpu* gpu);

    VkCommandBuffer vkCommandBuffer() { return fCmdBuffer; }

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkSecondaryCommandBuffer: %d (%d refs)\n", fCmdBuffer, this->getRefCnt());
    }
#endif

private:
    explicit GrVkSecondaryCommandBuffer(VkCommandBuffer cmdBuffer, GrVkCommandPool* cmdPool)
        : INHERITED(cmdBuffer, cmdPool) {}

    void onFreeGPUData(GrVkGpu* gpu) const override {}

    void onAbandonGPUData() const override {}

    friend class GrVkPrimaryCommandBuffer;

    typedef GrVkCommandBuffer INHERITED;
};

#endif
