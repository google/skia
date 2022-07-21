/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlResourceProvider_DEFINED
#define skgpu_graphite_MtlResourceProvider_DEFINED

#include "include/private/SkTHash.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/ResourceProvider.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

class CommandBuffer;
class MtlGpu;

class MtlResourceProvider final : public ResourceProvider {
public:
    MtlResourceProvider(const Gpu* gpu, sk_sp<GlobalCache>, SingleOwner*);
    ~MtlResourceProvider() override {}

    sk_sp<Texture> createWrappedTexture(const BackendTexture&) override;

    // Finds or creates a compatible DepthStencilState based on the enum
    sk_cfp<id<MTLDepthStencilState>> findOrCreateCompatibleDepthStencilState(
            const DepthStencilSettings&);

private:
    const MtlGpu* mtlGpu();

    sk_sp<CommandBuffer> createCommandBuffer() override;
    sk_sp<GraphicsPipeline> onCreateGraphicsPipeline(const GraphicsPipelineDesc&,
                                                            const RenderPassDesc&) override;
    sk_sp<Texture> createTexture(SkISize, const TextureInfo&, SkBudgeted) override;
    sk_sp<Buffer> createBuffer(size_t size, BufferType type, PrioritizeGpuReads) override;

    sk_sp<Sampler> createSampler(const SkSamplingOptions&,
                                 SkTileMode xTileMode,
                                 SkTileMode yTileMode) override;

    SkTHashMap<DepthStencilSettings, sk_cfp<id<MTLDepthStencilState>>> fDepthStencilStates;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlResourceProvider_DEFINED
