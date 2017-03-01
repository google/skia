/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkCommandBuffer_DEFINED
#define GrVkCommandBuffer_DEFINED

#include "GrVkGpu.h"
#include "GrVkResource.h"
#include "GrVkSemaphore.h"
#include "GrVkUtil.h"
#include "vk/GrVkDefines.h"

class GrVkFramebuffer;
class GrVkPipeline;
class GrVkRenderPass;
class GrVkRenderTarget;
class GrVkTransferBuffer;

class GrVkCommandBuffer : public GrVkResource {
public:
    void invalidateState();

    ////////////////////////////////////////////////////////////////////////////
    // CommandBuffer commands
    ////////////////////////////////////////////////////////////////////////////
    enum BarrierType {
        kMemory_BarrierType,
        kBufferMemory_BarrierType,
        kImageMemory_BarrierType
    };

    void pipelineBarrier(const GrVkGpu* gpu,
                         VkPipelineStageFlags srcStageMask,
                         VkPipelineStageFlags dstStageMask,
                         bool byRegion,
                         BarrierType barrierType,
                         void* barrier) const;

    void bindVertexBuffer(GrVkGpu* gpu, GrVkVertexBuffer* vbuffer) {
        VkBuffer vkBuffer = vbuffer->buffer();
        // TODO: once vbuffer->offset() no longer always returns 0, we will need to track the offset
        // to know if we can skip binding or not.
        if (!fBoundVertexBufferIsValid || vkBuffer != fBoundVertexBuffer) {
            VkDeviceSize offset = vbuffer->offset();
            GR_VK_CALL(gpu->vkInterface(), CmdBindVertexBuffers(fCmdBuffer,
                                                                0,
                                                                1,
                                                                &vkBuffer,
                                                                &offset));
            fBoundVertexBufferIsValid = true;
            fBoundVertexBuffer = vkBuffer;
            addResource(vbuffer->resource());
        }
    }

    void bindIndexBuffer(GrVkGpu* gpu, GrVkIndexBuffer* ibuffer) {
        VkBuffer vkBuffer = ibuffer->buffer();
        // TODO: once ibuffer->offset() no longer always returns 0, we will need to track the offset
        // to know if we can skip binding or not.
        if (!fBoundIndexBufferIsValid || vkBuffer != fBoundIndexBuffer) {
            GR_VK_CALL(gpu->vkInterface(), CmdBindIndexBuffer(fCmdBuffer,
                                                              vkBuffer,
                                                              ibuffer->offset(),
                                                              VK_INDEX_TYPE_UINT16));
            fBoundIndexBufferIsValid = true;
            fBoundIndexBuffer = vkBuffer;
            addResource(ibuffer->resource());
        }
    }

    void bindPipeline(const GrVkGpu* gpu, const GrVkPipeline* pipeline);

    void bindDescriptorSets(const GrVkGpu* gpu,
                            GrVkPipelineState*,
                            VkPipelineLayout layout,
                            uint32_t firstSet,
                            uint32_t setCount,
                            const VkDescriptorSet* descriptorSets,
                            uint32_t dynamicOffsetCount,
                            const uint32_t* dynamicOffsets);

    void bindDescriptorSets(const GrVkGpu* gpu,
                            const SkTArray<const GrVkRecycledResource*>&,
                            const SkTArray<const GrVkResource*>&,
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
                          const VkClearRect* clearRects) const;

    void drawIndexed(const GrVkGpu* gpu,
                     uint32_t indexCount,
                     uint32_t instanceCount,
                     uint32_t firstIndex,
                     int32_t vertexOffset,
                     uint32_t firstInstance) const;

    void draw(const GrVkGpu* gpu,
              uint32_t vertexCount,
              uint32_t instanceCount,
              uint32_t firstVertex,
              uint32_t firstInstance) const;

    // Add ref-counted resource that will be tracked and released when this
    // command buffer finishes execution
    void addResource(const GrVkResource* resource) {
        resource->ref();
        fTrackedResources.append(1, &resource);
    }

    // Add ref-counted resource that will be tracked and released when this command buffer finishes
    // execution. When it is released, it will signal that the resource can be recycled for reuse.
    void addRecycledResource(const GrVkRecycledResource* resource) {
        resource->ref();
        fTrackedRecycledResources.append(1, &resource);
    }

    void reset(GrVkGpu* gpu);

protected:
        GrVkCommandBuffer(VkCommandBuffer cmdBuffer, const GrVkRenderPass* rp = VK_NULL_HANDLE)
            : fIsActive(false)
            , fActiveRenderPass(rp)
            , fCmdBuffer(cmdBuffer)
            , fBoundVertexBufferIsValid(false)
            , fBoundIndexBufferIsValid(false)
            , fNumResets(0) {
            fTrackedResources.setReserve(kInitialTrackedResourcesCount);
            fTrackedRecycledResources.setReserve(kInitialTrackedResourcesCount);
            this->invalidateState();
        }

        SkTDArray<const GrVkResource*>          fTrackedResources;
        SkTDArray<const GrVkRecycledResource*>  fTrackedRecycledResources;

        // Tracks whether we are in the middle of a command buffer begin/end calls and thus can add
        // new commands to the buffer;
        bool fIsActive;

        // Stores a pointer to the current active render pass (i.e. begin has been called but not
        // end). A nullptr means there is no active render pass. The GrVKCommandBuffer does not own
        // the render pass.
        const GrVkRenderPass*     fActiveRenderPass;

        VkCommandBuffer           fCmdBuffer;

private:
    static const int kInitialTrackedResourcesCount = 32;

    void freeGPUData(const GrVkGpu* gpu) const override;
    virtual void onFreeGPUData(const GrVkGpu* gpu) const = 0;
    void abandonSubResources() const override;

    virtual void onReset(GrVkGpu* gpu) {}

    VkBuffer                                fBoundVertexBuffer;
    bool                                    fBoundVertexBufferIsValid;

    VkBuffer                                fBoundIndexBuffer;
    bool                                    fBoundIndexBufferIsValid;

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
};

class GrVkSecondaryCommandBuffer;

class GrVkPrimaryCommandBuffer : public GrVkCommandBuffer {
public:
    ~GrVkPrimaryCommandBuffer() override;

    static GrVkPrimaryCommandBuffer* Create(const GrVkGpu* gpu, VkCommandPool cmdPool);

    void begin(const GrVkGpu* gpu);
    void end(const GrVkGpu* gpu);

    // Begins render pass on this command buffer. The framebuffer from GrVkRenderTarget will be used
    // in the render pass.
    void beginRenderPass(const GrVkGpu* gpu,
                         const GrVkRenderPass* renderPass,
                         const VkClearValue* clearValues,
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
                   VkFilter filter) {
        this->blitImage(gpu,
                        srcImage.resource(),
                        srcImage.image(),
                        srcImage.currentLayout(),
                        dstImage.resource(),
                        dstImage.image(),
                        dstImage.currentLayout(),
                        blitRegionCount,
                        blitRegions,
                        filter);
    }

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
                       const GrVkSemaphore::Resource* signalSemaphore,
                       SkTArray<const GrVkSemaphore::Resource*>& waitSemaphores);
    bool finished(const GrVkGpu* gpu) const;

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkPrimaryCommandBuffer: %d (%d refs)\n", fCmdBuffer, this->getRefCnt());
    }
#endif

private:
    explicit GrVkPrimaryCommandBuffer(VkCommandBuffer cmdBuffer)
        : INHERITED(cmdBuffer)
        , fSubmitFence(VK_NULL_HANDLE) {}

    void onFreeGPUData(const GrVkGpu* gpu) const override;

    void onReset(GrVkGpu* gpu) override;

    SkTArray<GrVkSecondaryCommandBuffer*, true> fSecondaryCommandBuffers;
    VkFence                                     fSubmitFence;

    typedef GrVkCommandBuffer INHERITED;
};

class GrVkSecondaryCommandBuffer : public GrVkCommandBuffer {
public:
    static GrVkSecondaryCommandBuffer* Create(const GrVkGpu* gpu, VkCommandPool cmdPool);

    void begin(const GrVkGpu* gpu, const GrVkFramebuffer* framebuffer,
               const GrVkRenderPass* compatibleRenderPass);
    void end(const GrVkGpu* gpu);

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkSecondaryCommandBuffer: %d (%d refs)\n", fCmdBuffer, this->getRefCnt());
    }
#endif

private:
    explicit GrVkSecondaryCommandBuffer(VkCommandBuffer cmdBuffer)
        : INHERITED(cmdBuffer) {
    }

    void onFreeGPUData(const GrVkGpu* gpu) const override {}

    friend class GrVkPrimaryCommandBuffer;

    typedef GrVkCommandBuffer INHERITED;
};

#endif
