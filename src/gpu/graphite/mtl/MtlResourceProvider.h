/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlResourceProvider_DEFINED
#define skgpu_graphite_MtlResourceProvider_DEFINED

#include "src/core/SkTHash.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/mtl/MtlGraphicsPipeline.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

class CommandBuffer;
class MtlSharedContext;

class MtlResourceProvider final : public ResourceProvider {
public:
    MtlResourceProvider(SharedContext* sharedContext,
                        SingleOwner*,
                        uint32_t recorderID,
                        size_t resourceBudget);
    ~MtlResourceProvider() override {}

    sk_sp<MtlGraphicsPipeline> findOrCreateLoadMSAAPipeline(const RenderPassDesc&);

    sk_cfp<id<MTLDepthStencilState>> findOrCreateCompatibleDepthStencilState(
            const DepthStencilSettings&);

private:
    const MtlSharedContext* mtlSharedContext();

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
    void onDeleteBackendTexture(const BackendTexture&) override;

    skia_private::THashMap<DepthStencilSettings, sk_cfp<id<MTLDepthStencilState>>>
            fDepthStencilStates;
    skia_private::THashMap<uint32_t, sk_sp<MtlGraphicsPipeline>> fLoadMSAAPipelines;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlResourceProvider_DEFINED
