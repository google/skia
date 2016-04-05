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
#include "GrVkUtil.h"
#include "vk/GrVkDefines.h"

class GrVkPipeline;
class GrVkRenderPass;
class GrVkRenderTarget;
class GrVkTransferBuffer;

class GrVkCommandBuffer : public GrVkResource {
public:
    static GrVkCommandBuffer* Create(const GrVkGpu* gpu, VkCommandPool cmdPool);
    ~GrVkCommandBuffer() override;

    void begin(const GrVkGpu* gpu);
    void end(const GrVkGpu* gpu);

    void invalidateState();

    // Begins render pass on this command buffer. The framebuffer from GrVkRenderTarget will be used
    // in the render pass.
    void beginRenderPass(const GrVkGpu* gpu,
                         const GrVkRenderPass* renderPass,
                         const GrVkRenderTarget& target);
    void endRenderPass(const GrVkGpu* gpu);

    void submitToQueue(const GrVkGpu* gpu, VkQueue queue, GrVkGpu::SyncQueue sync);
    bool finished(const GrVkGpu* gpu) const;

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
        if (!fBoundVertexBufferIsValid || vkBuffer != fBoundVertexBuffer) {
            VkDeviceSize offset = 0;
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
        if (!fBoundIndexBufferIsValid || vkBuffer != fBoundIndexBuffer) {
            GR_VK_CALL(gpu->vkInterface(), CmdBindIndexBuffer(fCmdBuffer,
                                                              vkBuffer,
                                                              0,
                                                              VK_INDEX_TYPE_UINT16));
            fBoundIndexBufferIsValid = true;
            fBoundIndexBuffer = vkBuffer;
            addResource(ibuffer->resource());
        }
    }

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
                   GrVkImage* srcImage,
                   VkImageLayout srcLayout,
                   GrVkImage* dstImage,
                   VkImageLayout dstLayout,
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

    // Commands that only work inside of a render pass
    void clearAttachments(const GrVkGpu* gpu,
                          int numAttachments,
                          const VkClearAttachment* attachments,
                          int numRects,
                          const VkClearRect* clearRects) const;

    void bindPipeline(const GrVkGpu* gpu, const GrVkPipeline* pipeline);


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
        fTrackedResources.push_back(resource);
    }

private:
    static const int kInitialTrackedResourcesCount = 32;

    explicit GrVkCommandBuffer(VkCommandBuffer cmdBuffer)
        : fTrackedResources(kInitialTrackedResourcesCount)
        , fCmdBuffer(cmdBuffer)
        , fSubmitFence(VK_NULL_HANDLE)
        , fBoundVertexBufferIsValid(false)
        , fBoundIndexBufferIsValid(false)
        , fIsActive(false)
        , fActiveRenderPass(nullptr) {
        this->invalidateState();
    }

    void freeGPUData(const GrVkGpu* gpu) const override;
    void abandonSubResources() const override;

    SkTArray<const GrVkResource*, true>     fTrackedResources;

    VkCommandBuffer                         fCmdBuffer;
    VkFence                                 fSubmitFence;

    VkBuffer                                fBoundVertexBuffer;
    bool                                    fBoundVertexBufferIsValid;

    VkBuffer                                fBoundIndexBuffer;
    bool                                    fBoundIndexBufferIsValid;

    // Tracks whether we are in the middle of a command buffer begin/end calls and thus can add new
    // commands to the buffer;
    bool fIsActive;

    // Stores a pointer to the current active render pass (i.e. begin has been called but not end).
    // A nullptr means there is no active render pass. The GrVKCommandBuffer does not own the render
    // pass.
    const GrVkRenderPass*     fActiveRenderPass;

    // Cached values used for dynamic state updates
    VkViewport fCachedViewport;
    VkRect2D   fCachedScissor;
    float      fCachedBlendConstant[4];
};


#endif
