/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnResourceProvider_DEFINED
#define skgpu_graphite_DawnResourceProvider_DEFINED

#include "src/core/SkTHash.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

class DawnSharedContext;
class DawnTexture;

class DawnResourceProvider final : public ResourceProvider {
public:
    DawnResourceProvider(SharedContext* sharedContext,
                         SingleOwner*,
                         uint32_t recorderID,
                         size_t resourceBudget);
    ~DawnResourceProvider() override;

    sk_sp<Texture> createWrappedTexture(const BackendTexture&) override;

    sk_sp<DawnTexture> findOrCreateDiscardableMSAALoadTexture(SkISize dimensions,
                                                              const TextureInfo& msaaInfo);

    wgpu::RenderPipeline findOrCreateBlitWithDrawPipeline(const RenderPassDesc& renderPassDesc);

private:
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
    void onDeleteBackendTexture(BackendTexture&) override;

    const DawnSharedContext* dawnSharedContext() const;

    skia_private::THashMap<uint64_t, wgpu::RenderPipeline> fBlitWithDrawPipelines;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnResourceProvider_DEFINED
