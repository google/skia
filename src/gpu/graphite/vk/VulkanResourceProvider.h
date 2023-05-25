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
class VulkanSharedContext;

class VulkanResourceProvider final : public ResourceProvider {
public:
    static VkDescriptorSetLayout DescTypeAndCountToVkDescSetLayout(const VulkanSharedContext* ctxt,
                                                                   SkSpan<DescTypeAndCount>);

    VulkanResourceProvider(SharedContext* sharedContext, SingleOwner*, uint32_t recorderID);
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

    BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) override;
    void onDeleteBackendTexture(BackendTexture&) override {}

    VulkanDescriptorSet* findOrCreateDescriptorSet(SkSpan<DescTypeAndCount>);

    friend class VulkanCommandBuffer;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanResourceProvider_DEFINED
