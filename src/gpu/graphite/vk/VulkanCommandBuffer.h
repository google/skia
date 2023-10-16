/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanCommandBuffer_DEFINED
#define skgpu_graphite_VulkanCommandBuffer_DEFINED

#include "src/gpu/graphite/CommandBuffer.h"

#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/DrawPass.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"

namespace skgpu::graphite {

class VulkanResourceProvider;
class VulkanSharedContext;
class Buffer;

class VulkanCommandBuffer final : public CommandBuffer {
public:
    static std::unique_ptr<VulkanCommandBuffer> Make(const VulkanSharedContext*,
                                                     VulkanResourceProvider*);
    ~VulkanCommandBuffer() override;

    bool setNewCommandBufferResources() override;

    bool submit(VkQueue);

    bool isFinished();

    void waitUntilFinished();

    void addBufferMemoryBarrier(const Resource* resource,
                                VkPipelineStageFlags srcStageMask,
                                VkPipelineStageFlags dstStageMask,
                                bool byRegion,
                                VkBufferMemoryBarrier* barrier);
    void addBufferMemoryBarrier(VkPipelineStageFlags srcStageMask,
                                VkPipelineStageFlags dstStageMask,
                                bool byRegion,
                                VkBufferMemoryBarrier* barrier);
    void addImageMemoryBarrier(const Resource*,
                               VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask,
                               bool byRegion,
                               VkImageMemoryBarrier* barrier);

private:
    VulkanCommandBuffer(VkCommandPool pool,
                        VkCommandBuffer primaryCommandBuffer,
                        const VulkanSharedContext* sharedContext,
                        VulkanResourceProvider* resourceProvider);

    void onResetCommandBuffer() override;

    void begin();
    void end();

    void addWaitSemaphores(size_t numWaitSemaphores,
                           const BackendSemaphore* waitSemaphores) override;
    void addSignalSemaphores(size_t numWaitSemaphores,
                             const BackendSemaphore* signalSemaphores) override;
    void prepareSurfaceForStateUpdate(SkSurface* targetSurface,
                                      const MutableTextureState* newState) override;

    bool onAddRenderPass(const RenderPassDesc&,
                        const Texture* colorTexture,
                        const Texture* resolveTexture,
                        const Texture* depthStencilTexture,
                        SkRect viewport,
                        const DrawPassList&) override;

    bool beginRenderPass(const RenderPassDesc&,
                         const Texture* colorTexture,
                         const Texture* resolveTexture,
                         const Texture* depthStencilTexture);
    void endRenderPass();

    void addDrawPass(const DrawPass*);

    // Track descriptor changes for binding prior to draw calls
    void recordBufferBindingInfo(const BindBufferInfo& info, UniformSlot);
    void recordTextureAndSamplerDescSet(
            const DrawPass&, const DrawPassCommands::BindTexturesAndSamplers&);

    void bindTextureSamplers();
    void bindUniformBuffers();
    void syncDescriptorSets();

    void bindGraphicsPipeline(const GraphicsPipeline*);
    void setBlendConstants(float* blendConstants);
    void bindDrawBuffers(const BindBufferInfo& vertices,
                         const BindBufferInfo& instances,
                         const BindBufferInfo& indices,
                         const BindBufferInfo& indirect);
    void bindVertexBuffers(const Buffer* vertexBuffer, size_t vertexOffset,
                           const Buffer* instanceBuffer, size_t instanceOffset);
    void bindInputBuffer(const Buffer* buffer, VkDeviceSize offset, uint32_t binding);
    void bindIndexBuffer(const Buffer* indexBuffer, size_t offset);
    void bindIndirectBuffer(const Buffer* indirectBuffer, size_t offset);
    void setScissor(unsigned int left, unsigned int top,
                    unsigned int width, unsigned int height);

    void draw(PrimitiveType type, unsigned int baseVertex, unsigned int vertexCount);
    void drawIndexed(PrimitiveType type, unsigned int baseIndex, unsigned int indexCount,
                     unsigned int baseVertex);
    void drawInstanced(PrimitiveType type,
                       unsigned int baseVertex, unsigned int vertexCount,
                       unsigned int baseInstance, unsigned int instanceCount);
    void drawIndexedInstanced(PrimitiveType type, unsigned int baseIndex,
                              unsigned int indexCount, unsigned int baseVertex,
                              unsigned int baseInstance, unsigned int instanceCount);
    void drawIndirect(PrimitiveType type);
    void drawIndexedIndirect(PrimitiveType type);

    // TODO: The virtuals in this class have not yet been implemented as we still haven't
    // implemented the objects they use.
    bool onAddComputePass(const DispatchGroupList&) override;

    bool onCopyBufferToBuffer(const Buffer* srcBuffer,
                              size_t srcOffset,
                              const Buffer* dstBuffer,
                              size_t dstOffset,
                              size_t size) override;
    bool onCopyTextureToBuffer(const Texture*,
                               SkIRect srcRect,
                               const Buffer*,
                               size_t bufferOffset,
                               size_t bufferRowBytes) override;
    bool onCopyBufferToTexture(const Buffer*,
                               const Texture*,
                               const BufferTextureCopyData* copyData,
                               int count) override;
    bool onCopyTextureToTexture(const Texture* src,
                                SkIRect srcRect,
                                const Texture* dst,
                                SkIPoint dstPoint,
                                int mipLevel) override;

    bool onSynchronizeBufferToCpu(const Buffer*, bool* outDidResultInWork) override;
    bool onClearBuffer(const Buffer*, size_t offset, size_t size) override;

    enum BarrierType {
        kBufferMemory_BarrierType,
        kImageMemory_BarrierType
    };
    void pipelineBarrier(const Resource* resource,
                         VkPipelineStageFlags srcStageMask,
                         VkPipelineStageFlags dstStageMask,
                         bool byRegion,
                         BarrierType barrierType,
                         void* barrier);
    void submitPipelineBarriers(bool forSelfDependency = false);

    // Update the intrinsic constant uniform with the latest rtAdjust value as determined by a
    // given viewport. The resource provider is responsible for finding a suitable buffer and
    // managing its lifetime.
    void updateRtAdjustUniform(const SkRect& viewport);

    VkCommandPool fPool;
    VkCommandBuffer fPrimaryCommandBuffer;
    const VulkanSharedContext* fSharedContext;
    VulkanResourceProvider* fResourceProvider;

    // begin() has been called, but not end()
    bool fActive = false;
    // Track whether there is currently an active render pass (beginRenderPass has been called, but
    // not endRenderPass)
    bool fActiveRenderPass = false;

    const VulkanGraphicsPipeline* fActiveGraphicsPipeline = nullptr;

    VkFence fSubmitFence = VK_NULL_HANDLE;

    // Current semaphores
    skia_private::STArray<1, VkSemaphore> fWaitSemaphores;
    skia_private::STArray<1, VkSemaphore> fSignalSemaphores;

    // Tracking of memory barriers so that we can submit them all in a batch together.
    skia_private::STArray<1, VkBufferMemoryBarrier> fBufferBarriers;
    skia_private::STArray<2, VkImageMemoryBarrier> fImageBarriers;
    bool fBarriersByRegion = false;
    VkPipelineStageFlags fSrcStageMask = 0;
    VkPipelineStageFlags fDstStageMask = 0;

    // Track whether certain descriptor sets need to be bound
    bool fBindUniformBuffers = false;
    bool fBindTextureSamplers = false;

    std::array<BindBufferInfo, VulkanGraphicsPipeline::kNumUniformBuffers> fUniformBuffersToBind
            = {{{nullptr, 0}}};
    VkDescriptorSet fTextureSamplerDescSetToBind = VK_NULL_HANDLE;

    int fNumTextureSamplers = 0;

    VkBuffer fBoundInputBuffers[VulkanGraphicsPipeline::kNumInputBuffers];
    size_t fBoundInputBufferOffsets[VulkanGraphicsPipeline::kNumInputBuffers];

    VkBuffer fBoundIndexBuffer = VK_NULL_HANDLE;
    VkBuffer fBoundIndirectBuffer = VK_NULL_HANDLE;
    size_t fBoundIndexBufferOffset = 0;
    size_t fBoundIndirectBufferOffset = 0;

    float fCachedBlendConstant[4];
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanCommandBuffer_DEFINED

