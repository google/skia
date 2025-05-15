/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanResourceProvider_DEFINED
#define skgpu_graphite_VulkanResourceProvider_DEFINED

#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"

#include "include/gpu/vk/VulkanTypes.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/DescriptorData.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"

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
class VulkanSharedContext;
class VulkanTexture;
class VulkanYcbcrConversion;

class VulkanResourceProvider final : public ResourceProvider {
public:
    static constexpr size_t kIntrinsicConstantSize = sizeof(float) * 8; // float4 + 2xfloat2
    // Intrinsic constant rtAdjust value is needed by the vertex shader. Dst copy bounds are needed
    // in the frag shader.
    static constexpr VkShaderStageFlagBits kIntrinsicConstantStageFlags =
            VkShaderStageFlagBits(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    static constexpr size_t kLoadMSAAPushConstantSize = sizeof(float) * 4;
    static constexpr VkShaderStageFlagBits kLoadMSAAPushConstantStageFlags =
            VK_SHADER_STAGE_VERTEX_BIT;


    using UniformBindGroupKey = FixedSizeKey<2 * VulkanGraphicsPipeline::kNumUniformBuffers>;

    VulkanResourceProvider(SharedContext* sharedContext,
                           SingleOwner*,
                           uint32_t recorderID,
                           size_t resourceBudget);

    ~VulkanResourceProvider() override;

    sk_sp<VulkanYcbcrConversion> findOrCreateCompatibleYcbcrConversion(
            const VulkanYcbcrConversionInfo& ycbcrInfo) const;

    sk_sp<VulkanDescriptorSet> findOrCreateDescriptorSet(SkSpan<DescriptorData>);

    sk_sp<VulkanDescriptorSet> findOrCreateUniformBuffersDescriptorSet(
            SkSpan<DescriptorData> requestedDescriptors,
            SkSpan<BindBufferInfo> bindUniformBufferInfo);

    sk_sp<VulkanGraphicsPipeline> findOrCreateLoadMSAAPipeline(const RenderPassDesc&);

    // Find or create a compatible (needed when creating a framebuffer and graphics pipeline) or
    // full (needed when beginning a render pass from the command buffer) RenderPass.
    sk_sp<VulkanRenderPass> findOrCreateRenderPass(const RenderPassDesc&, bool compatibleOnly);

    VkPipelineCache pipelineCache();

    VkPipelineLayout mockPipelineLayout() const { return fMockPipelineLayout; }

    sk_sp<VulkanFramebuffer> findOrCreateFramebuffer(const VulkanSharedContext*,
                                                     VulkanTexture* colorTexture,
                                                     VulkanTexture* resolveTexture,
                                                     VulkanTexture* depthStencilTexture,
                                                     const RenderPassDesc&,
                                                     const VulkanRenderPass&,
                                                     const int width,
                                                     const int height);

private:
    const VulkanSharedContext* vulkanSharedContext() const;

    sk_sp<GraphicsPipeline> createGraphicsPipeline(const RuntimeEffectDictionary*,
                                                   const UniqueKey&,
                                                   const GraphicsPipelineDesc&,
                                                   const RenderPassDesc&,
                                                   SkEnumBitMask<PipelineCreationFlags>,
                                                   uint32_t compilationID) override;
    sk_sp<ComputePipeline> createComputePipeline(const ComputePipelineDesc&) override;

    sk_sp<Texture> createTexture(SkISize, const TextureInfo&) override;
    sk_sp<Texture> onCreateWrappedTexture(const BackendTexture&) override;
    sk_sp<Buffer> createBuffer(size_t size, BufferType type, AccessPattern) override;
    sk_sp<Sampler> createSampler(const SamplerDesc&) override;

    BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) override;
#ifdef SK_BUILD_FOR_ANDROID
    BackendTexture onCreateBackendTexture(AHardwareBuffer*,
                                          bool isRenderable,
                                          bool isProtectedContent,
                                          SkISize dimensions,
                                          bool fromAndroidWindow) const override;
#endif
    void onDeleteBackendTexture(const BackendTexture&) override;

    VkPipelineCache fPipelineCache = VK_NULL_HANDLE;

    // Certain operations only need to occur once per renderpass (updating push constants and, if
    // necessary, binding the dst texture as an input attachment). It is useful to have a
    // mock pipeline layout that has compatible push constant parameters and input attachment
    // descriptor set with all other real pipeline layouts such that it can be reused across command
    // buffers to perform these operations even before we bind any pipelines.
    VkPipelineLayout fMockPipelineLayout;

    // The first value of the pair is a compatible-only renderpass metadata for the render pass.
    skia_private::TArray<std::pair<VulkanRenderPass::Metadata,
                                   sk_sp<VulkanGraphicsPipeline>>> fLoadMSAAPipelines;
    // The shader modules and pipeline layout can be shared for all loadMSAA pipelines.
    std::unique_ptr<VulkanProgramInfo> fLoadMSAAProgram;

    SkLRUCache<UniformBindGroupKey, sk_sp<VulkanDescriptorSet>,
               UniformBindGroupKey::Hash> fUniformBufferDescSetCache;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanResourceProvider_DEFINED
