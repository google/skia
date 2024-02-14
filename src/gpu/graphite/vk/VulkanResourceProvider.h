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
#include "src/gpu/graphite/DescriptorData.h"

#ifdef  SK_BUILD_FOR_ANDROID
extern "C" {
    typedef struct AHardwareBuffer AHardwareBuffer;
}
#endif

namespace skgpu::graphite {

class VulkanCommandBuffer;
class VulkanDescriptorSet;
class VulkanFramebuffer;
class VulkanGraphicsPipeline;
class VulkanRenderPass;
class VulkanSharedContext;
class VulkanSamplerYcbcrConversion;

class VulkanResourceProvider final : public ResourceProvider {
public:
    static constexpr size_t kIntrinsicConstantSize = sizeof(float) * 4;
    static constexpr size_t kLoadMSAAVertexBufferSize = sizeof(float) * 8; // 4 points of 2 floats

    VulkanResourceProvider(SharedContext* sharedContext,
                           SingleOwner*,
                           uint32_t recorderID,
                           size_t resourceBudget,
                           sk_sp<Buffer> intrinsicConstantUniformBuffer,
                           sk_sp<Buffer> loadMSAAVertexBuffer);

    ~VulkanResourceProvider() override;

    sk_sp<Texture> createWrappedTexture(const BackendTexture&) override;

    sk_sp<Buffer> refIntrinsicConstantBuffer() const;

    const Buffer* loadMSAAVertexBuffer() const;

    sk_sp<VulkanSamplerYcbcrConversion> findOrCreateCompatibleSamplerYcbcrConversion(
            const VulkanYcbcrConversionInfo& ycbcrInfo) const;

private:
    const VulkanSharedContext* vulkanSharedContext() const;

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
#ifdef SK_BUILD_FOR_ANDROID
    BackendTexture onCreateBackendTexture(AHardwareBuffer*,
                                          bool isRenderable,
                                          bool isProtectedContent,
                                          SkISize dimensions,
                                          bool fromAndroidWindow) const override;
#endif
    void onDeleteBackendTexture(const BackendTexture&) override;

    sk_sp<VulkanDescriptorSet> findOrCreateDescriptorSet(SkSpan<DescriptorData>);

    sk_sp<VulkanGraphicsPipeline> findOrCreateLoadMSAAPipeline(const RenderPassDesc&);

    // Find or create a compatible (needed when creating a framebuffer and graphics pipeline) or
    // full (needed when beginning a render pass from the command buffer) RenderPass.
    sk_sp<VulkanRenderPass> findOrCreateRenderPass(const RenderPassDesc&, bool compatibleOnly);

    // Use a predetermined RenderPass key for finding/creating a RenderPass to avoid recreating it
    sk_sp<VulkanRenderPass> findOrCreateRenderPassWithKnownKey(
            const RenderPassDesc&, bool compatibleOnly, const GraphiteResourceKey& rpKey);

    VkPipelineCache pipelineCache();

    friend class VulkanCommandBuffer;
    VkPipelineCache fPipelineCache = VK_NULL_HANDLE;

    // Each render pass will need buffer space to record rtAdjust information. To minimize costly
    // allocation calls and searching of the resource cache, we find & store a uniform buffer upon
    // resource provider creation. This way, render passes across all command buffers can simply
    // update the value within this buffer as needed.
    sk_sp<Buffer> fIntrinsicUniformBuffer;
    // Similary, use a shared buffer b/w all renderpasses to store vertices for loading MSAA from
    // resolve.
    sk_sp<Buffer> fLoadMSAAVertexBuffer;

    // The first value of the pair is a renderpass key. Graphics pipeline keys contain extra
    // information that we do not need for identifying unique pipelines.
    skia_private::TArray<std::pair<GraphiteResourceKey,
                         sk_sp<VulkanGraphicsPipeline>>> fLoadMSAAPipelines;
    // All of the following attributes are the same between all msaa load pipelines, so they only
    // need to be created once and can then be stored.
    VkShaderModule fMSAALoadVertShaderModule = VK_NULL_HANDLE;
    VkShaderModule fMSAALoadFragShaderModule = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo fMSAALoadShaderStageInfo[2];
    VkPipelineLayout fMSAALoadPipelineLayout = VK_NULL_HANDLE;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanResourceProvider_DEFINED
