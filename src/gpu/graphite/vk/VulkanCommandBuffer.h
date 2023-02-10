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

namespace skgpu::graphite {

class VulkanResourceProvider;
class VulkanSharedContext;

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

    // TODO: The virtuals in this class have not yet been implemented as we still haven't
    // implemented the objects they use.
    bool onAddRenderPass(const RenderPassDesc&,
                         const Texture* colorTexture,
                         const Texture* resolveTexture,
                         const Texture* depthStencilTexture,
                         SkRect viewport,
                         const std::vector<std::unique_ptr<DrawPass>>& drawPasses) override;
    bool onAddComputePass(const ComputePassDesc&,
                          const ComputePipeline*,
                          const std::vector<ResourceBinding>& bindings) override;

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
                                SkIPoint dstPoint) override;

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

#ifdef SK_ENABLE_PIET_GPU
    void onRenderPietScene(const skgpu::piet::Scene& scene, const Texture* target) override;
#endif

    VkCommandPool fPool;
    VkCommandBuffer fPrimaryCommandBuffer;
    const VulkanSharedContext* fSharedContext;
    VulkanResourceProvider* fResourceProvider;

    // Stores a pointer to the current active render pass (i.e. begin has been called but not
    // end). A nullptr means there is no active render pass. The VulkanCommandBuffer does not own
    // the render pass.
    // TODO: define what this is once we implement renderpasses.
    const void* fActiveRenderPass = nullptr;

    VkFence fSubmitFence = VK_NULL_HANDLE;

    // Tracking of memory barriers so that we can submit them all in a batch together.
    SkSTArray<1, VkBufferMemoryBarrier> fBufferBarriers;
    SkSTArray<2, VkImageMemoryBarrier> fImageBarriers;
    bool fBarriersByRegion = false;
    VkPipelineStageFlags fSrcStageMask = 0;
    VkPipelineStageFlags fDstStageMask = 0;

#ifdef SK_DEBUG
    bool fActive = false;
#endif
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanCommandBuffer_DEFINED

