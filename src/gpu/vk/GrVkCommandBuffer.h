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
#include "src/gpu/GrRefCnt.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkSemaphore.h"
#include "src/gpu/vk/GrVkUtil.h"

class GrVkFramebuffer;
class GrVkImage;
class GrVkPipeline;
class GrVkPipelineState;
class GrVkRenderPass;
class GrVkRenderTarget;

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

    void bindInputBuffer(GrVkGpu* gpu, uint32_t binding, sk_sp<const GrBuffer> buffer);

    void bindIndexBuffer(GrVkGpu* gpu, sk_sp<const GrBuffer> buffer);

    void bindPipeline(const GrVkGpu* gpu, sk_sp<const GrVkPipeline> pipeline);

    void bindDescriptorSets(const GrVkGpu* gpu,
                            VkPipelineLayout layout,
                            uint32_t firstSet,
                            uint32_t setCount,
                            const VkDescriptorSet* descriptorSets,
                            uint32_t dynamicOffsetCount,
                            const uint32_t* dynamicOffsets);

    void pushConstants(const GrVkGpu* gpu, VkPipelineLayout layout,
                       VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                       const void* values);

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

    void drawIndirect(const GrVkGpu* gpu,
                      sk_sp<const GrBuffer> indirectBuffer,
                      VkDeviceSize offset,
                      uint32_t drawCount,
                      uint32_t stride);

    void drawIndexedIndirect(const GrVkGpu* gpu,
                             sk_sp<const GrBuffer> indirectBuffer,
                             VkDeviceSize offset,
                             uint32_t drawCount,
                             uint32_t stride);

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution
    void addResource(sk_sp<const GrManagedResource> resource) {
        SkASSERT(resource);
        resource->notifyQueuedForWorkOnGpu();
        fTrackedResources.push_back(std::move(resource));
    }
    void addResource(const GrManagedResource* resource) {
        this->addResource(sk_ref_sp(resource));
    }

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution. When it is released, it will signal that the resource can be recycled for reuse.
    void addRecycledResource(gr_rp<const GrRecycledResource> resource) {
        SkASSERT(resource);
        resource->notifyQueuedForWorkOnGpu();
        fTrackedRecycledResources.push_back(std::move(resource));
    }

    void addRecycledResource(const GrRecycledResource* resource) {
        this->addRecycledResource(gr_ref_rp<const GrRecycledResource>(resource));
    }

    void addGrBuffer(sk_sp<const GrBuffer> buffer) {
        fTrackedGpuBuffers.push_back(std::move(buffer));
    }

    void addGrSurface(sk_sp<const GrSurface> surface) {
        fTrackedGpuSurfaces.push_back(std::move(surface));
    }

    void releaseResources();

    void freeGPUData(const GrGpu* gpu, VkCommandPool pool) const;

    bool hasWork() const { return fHasWork; }

protected:
    GrVkCommandBuffer(VkCommandBuffer cmdBuffer, bool isWrapped = false)
            : fCmdBuffer(cmdBuffer)
            , fIsWrapped(isWrapped) {
        this->invalidateState();
    }

    bool isWrapped() const { return fIsWrapped; }

    void addingWork(const GrVkGpu* gpu);

    void submitPipelineBarriers(const GrVkGpu* gpu, bool forSelfDependency = false);

private:
    static constexpr int kInitialTrackedResourcesCount = 32;

protected:
    template <typename T> using TrackedResourceArray = SkSTArray<kInitialTrackedResourcesCount, T>;
    TrackedResourceArray<sk_sp<const GrManagedResource>> fTrackedResources;
    TrackedResourceArray<gr_rp<const GrRecycledResource>> fTrackedRecycledResources;
    SkSTArray<16, sk_sp<const GrBuffer>> fTrackedGpuBuffers;
    SkSTArray<16, gr_cb<const GrSurface>> fTrackedGpuSurfaces;

    // Tracks whether we are in the middle of a command buffer begin/end calls and thus can add
    // new commands to the buffer;
    bool                      fIsActive = false;
    bool                      fHasWork = false;

    // Stores a pointer to the current active render pass (i.e. begin has been called but not
    // end). A nullptr means there is no active render pass. The GrVKCommandBuffer does not own
    // the render pass.
    const GrVkRenderPass*     fActiveRenderPass = nullptr;

    VkCommandBuffer           fCmdBuffer;

    virtual void onReleaseResources() {}
    virtual void onFreeGPUData(const GrVkGpu* gpu) const = 0;

    static constexpr uint32_t kMaxInputBuffers = 2;

    VkBuffer fBoundInputBuffers[kMaxInputBuffers];
    VkBuffer fBoundIndexBuffer;

    // Cached values used for dynamic state updates
    VkViewport fCachedViewport;
    VkRect2D   fCachedScissor;
    float      fCachedBlendConstant[4];

    // Tracking of memory barriers so that we can submit them all in a batch together.
    SkSTArray<1, VkBufferMemoryBarrier> fBufferBarriers;
    SkSTArray<2, VkImageMemoryBarrier> fImageBarriers;
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
    void end(GrVkGpu* gpu, bool abandoningBuffer = false);

    // Begins render pass on this command buffer. The framebuffer from GrVkRenderTarget will be used
    // in the render pass.
    bool beginRenderPass(GrVkGpu* gpu,
                         const GrVkRenderPass* renderPass,
                         const VkClearValue clearValues[],
                         GrVkRenderTarget* target,
                         const SkIRect& bounds,
                         bool forSecondaryCB);
    void endRenderPass(const GrVkGpu* gpu);

    void nexSubpass(GrVkGpu* gpu, bool forSecondaryCB);

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
                           sk_sp<GrGpuBuffer> dstBuffer,
                           uint32_t copyRegionCount,
                           const VkBufferImageCopy* copyRegions);

    // All uses of copyBufferToImage are done with buffers from our staging manager. The staging
    // manager will handle making sure the command buffer refs the buffer. Thus we just pass in the
    // raw VkBuffer here and don't worry about refs.
    void copyBufferToImage(const GrVkGpu* gpu,
                           VkBuffer srcBuffer,
                           GrVkImage* dstImage,
                           VkImageLayout dstLayout,
                           uint32_t copyRegionCount,
                           const VkBufferImageCopy* copyRegions);

    void copyBuffer(GrVkGpu* gpu,
                    sk_sp<GrGpuBuffer> srcBuffer,
                    sk_sp<GrGpuBuffer> dstBuffer,
                    uint32_t regionCount,
                    const VkBufferCopy* regions);

    void updateBuffer(GrVkGpu* gpu,
                      sk_sp<GrVkBuffer> dstBuffer,
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

    void callFinishedProcs() {
        fFinishedProcs.reset();
    }

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

    using INHERITED = GrVkCommandBuffer;
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

    using INHERITED = GrVkCommandBuffer;
};

#endif
