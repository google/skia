/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanResourceProvider_DEFINED
#define skgpu_graphite_VulkanResourceProvider_DEFINED

#include "src/gpu/graphite/ResourceProvider.h"

#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/DescriptorTypes.h"

namespace skgpu::graphite {

class VulkanCommandBuffer;
class VulkanDescriptorSet;
class VulkanFramebuffer;
class VulkanRenderPass;
class VulkanSharedContext;

class VulkanResourceProvider final : public ResourceProvider {
public:
    VulkanResourceProvider(SharedContext* sharedContext,
                           SingleOwner*,
                           uint32_t recorderID,
                           size_t resourceBudget);
    ~VulkanResourceProvider() override;

    sk_sp<Texture> createWrappedTexture(const BackendTexture&) override;

private:
    const VulkanSharedContext* vulkanSharedContext();

    sk_sp<GraphicsPipeline> createGraphicsPipeline(const RuntimeEffectDictionary*,
                                                   const GraphicsPipelineDesc&,
                                                   const RenderPassDesc&) override;
    sk_sp<ComputePipeline> createComputePipeline(const ComputePipelineDesc&) override;

    sk_sp<Texture> createTexture(SkISize, const TextureInfo&, skgpu::Budgeted) override;
    sk_sp<Buffer> createBuffer(size_t size, BufferType type, AccessPattern) override;

    sk_sp<Sampler> createSampler(const SkSamplingOptions&,
                                 SkTileMode xTileMode,
                                 SkTileMode yTileMode) override;
    sk_sp<VulkanFramebuffer> createFramebuffer(
            const VulkanSharedContext*,
            const skia_private::TArray<VkImageView>& attachmentViews,
            const VulkanRenderPass&,
            const int width,
            const int height);

    BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) override;
    void onDeleteBackendTexture(BackendTexture&) override;

    sk_sp<VulkanDescriptorSet> findOrCreateDescriptorSet(SkSpan<DescriptorData>);
    // Find or create a compatible (needed when creating a framebuffer and graphics pipeline) or
    // full (needed when beginning a render pass from the command buffer) RenderPass.
    sk_sp<VulkanRenderPass> findOrCreateRenderPass(const RenderPassDesc&,
                                                   bool compatibleOnly);
    VkPipelineCache pipelineCache();

    friend class VulkanCommandBuffer;
    VkPipelineCache fPipelineCache = VK_NULL_HANDLE;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanResourceProvider_DEFINED
