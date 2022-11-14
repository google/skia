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

#ifdef SK_ENABLE_PIET_GPU
    void onRenderPietScene(const skgpu::piet::Scene& scene, const Texture* target) override;
#endif

    VkCommandPool fPool;
    VkCommandBuffer fPrimaryCommandBuffer;
    const VulkanSharedContext* fSharedContext;
    VulkanResourceProvider* fResourceProvider;

    VkFence fSubmitFence = VK_NULL_HANDLE;

#ifdef SK_DEBUG
    bool fActive = false;
#endif
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanCommandBuffer_DEFINED

