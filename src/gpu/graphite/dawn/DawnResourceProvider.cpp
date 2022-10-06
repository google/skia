/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnResourceProvider.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"

namespace skgpu::graphite {

DawnResourceProvider::DawnResourceProvider(SharedContext* sharedContext,
                                           SingleOwner* singleOwner)
        : ResourceProvider(sharedContext, singleOwner) {}

DawnResourceProvider::~DawnResourceProvider() = default;

sk_sp<Texture> DawnResourceProvider::createWrappedTexture(const BackendTexture&) {
    return nullptr;
}

sk_sp<GraphicsPipeline> DawnResourceProvider::createGraphicsPipeline(
        const SkRuntimeEffectDictionary*,
        const GraphicsPipelineDesc&,
        const RenderPassDesc&) {
    return nullptr;
}

sk_sp<ComputePipeline> DawnResourceProvider::createComputePipeline(const ComputePipelineDesc&) {
    return nullptr;
}

sk_sp<Texture> DawnResourceProvider::createTexture(SkISize, const TextureInfo&, SkBudgeted) {
    return nullptr;
}

sk_sp<Buffer> DawnResourceProvider::createBuffer(size_t size,
                                                 BufferType type,
                                                 PrioritizeGpuReads) {
    return nullptr;
}

sk_sp<Sampler> DawnResourceProvider::createSampler(const SkSamplingOptions&,
                                                   SkTileMode xTileMode,
                                                   SkTileMode yTileMode) {
    return nullptr;
}

BackendTexture DawnResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                              const TextureInfo&) {
    return {};
}

} // namespace skgpu::graphite
