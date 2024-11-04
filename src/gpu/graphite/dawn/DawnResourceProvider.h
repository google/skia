/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnResourceProvider_DEFINED
#define skgpu_graphite_DawnResourceProvider_DEFINED

#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

class DawnGraphicsPipeline;
class DawnSampler;
class DawnSharedContext;
class DawnTexture;
class DawnBuffer;
class DawnCommandBuffer;

class DawnResourceProvider final : public ResourceProvider {
public:
    template <size_t NumEntries>
    using BindGroupKey = FixedSizeKey<2 * NumEntries>;

    static constexpr size_t kNumUniformEntries = 4;

    DawnResourceProvider(SharedContext* sharedContext,
                         SingleOwner*,
                         uint32_t recorderID,
                         size_t resourceBudget);
    ~DawnResourceProvider() override;

    sk_sp<DawnTexture> findOrCreateDiscardableMSAALoadTexture(SkISize dimensions,
                                                              const TextureInfo& msaaInfo);

    wgpu::RenderPipeline findOrCreateBlitWithDrawPipeline(const RenderPassDesc& renderPassDesc);

    sk_sp<DawnBuffer> findOrCreateDawnBuffer(size_t size,
                                             BufferType type,
                                             AccessPattern,
                                             std::string_view label);

    const wgpu::BindGroupLayout& getOrCreateUniformBuffersBindGroupLayout();
    const wgpu::BindGroupLayout& getOrCreateSingleTextureSamplerBindGroupLayout();

    // Find the cached bind group or create a new one based on the bound buffers and their
    // binding sizes (boundBuffersAndSizes) for these uniforms (in order):
    // - Intrinsic constants.
    // - Render step uniforms.
    // - Paint uniforms.
    const wgpu::BindGroup& findOrCreateUniformBuffersBindGroup(
            const std::array<std::pair<const DawnBuffer*, uint32_t>, kNumUniformEntries>&
                    boundBuffersAndSizes);

    // Find or create a bind group containing the given sampler & texture.
    const wgpu::BindGroup& findOrCreateSingleTextureSamplerBindGroup(const DawnSampler* sampler,
                                                                     const DawnTexture* texture);

    // Find the cached bind buffer info, or create a new one for the given intrinsic values.
    BindBufferInfo findOrCreateIntrinsicBindBufferInfo(DawnCommandBuffer* cb,
                                                       UniformDataBlock intrinsicValues);

private:
    sk_sp<GraphicsPipeline> createGraphicsPipeline(const RuntimeEffectDictionary*,
                                                   const GraphicsPipelineDesc&,
                                                   const RenderPassDesc&,
                                                   SkEnumBitMask<PipelineCreationFlags>) override;
    sk_sp<ComputePipeline> createComputePipeline(const ComputePipelineDesc&) override;

    sk_sp<Texture> createTexture(SkISize, const TextureInfo&, skgpu::Budgeted) override;
    sk_sp<Buffer> createBuffer(size_t size, BufferType type, AccessPattern) override;

    sk_sp<Texture> onCreateWrappedTexture(const BackendTexture&) override;

    sk_sp<Sampler> createSampler(const SamplerDesc&) override;

    BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) override;
    void onDeleteBackendTexture(const BackendTexture&) override;

    const wgpu::Buffer& getOrCreateNullBuffer();

    DawnSharedContext* dawnSharedContext() const;

    void onFreeGpuResources() override;
    void onPurgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime) override;

    skia_private::THashMap<uint32_t, wgpu::RenderPipeline> fBlitWithDrawPipelines;

    wgpu::BindGroupLayout fUniformBuffersBindGroupLayout;
    wgpu::BindGroupLayout fSingleTextureSamplerBindGroupLayout;

    wgpu::Buffer fNullBuffer;

    template <size_t NumEntries>
    using BindGroupCache = SkLRUCache<BindGroupKey<NumEntries>,
                                      wgpu::BindGroup,
                                      typename BindGroupKey<NumEntries>::Hash>;

    BindGroupCache<kNumUniformEntries> fUniformBufferBindGroupCache;
    BindGroupCache<1> fSingleTextureSamplerBindGroups;

    class IntrinsicBuffer;
    class IntrinsicConstantsManager;
    std::unique_ptr<IntrinsicConstantsManager> fIntrinsicConstantsManager;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DawnResourceProvider_DEFINED
