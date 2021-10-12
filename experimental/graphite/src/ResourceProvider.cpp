/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ResourceProvider.h"

#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/RenderPipeline.h"
#include "experimental/graphite/src/Texture.h"

namespace skgpu {

ResourceProvider::ResourceProvider(const Gpu* gpu) : fGpu(gpu) {
    fRenderPipelineCache.reset(new RenderPipelineCache(this));
}

ResourceProvider::~ResourceProvider() {
    fRenderPipelineCache.release();
}

RenderPipeline* ResourceProvider::findOrCreateRenderPipeline(const RenderPipelineDesc& desc) {
    return fRenderPipelineCache->refPipeline(desc);
}

////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceProvider::RenderPipelineCache::Entry {
    Entry(std::unique_ptr<RenderPipeline> pipeline)
            : fPipeline(std::move(pipeline)) {}

    std::unique_ptr<RenderPipeline> fPipeline;
};

ResourceProvider::RenderPipelineCache::RenderPipelineCache(ResourceProvider* resourceProvider)
    : fMap(16) // TODO: find a good value for this
    , fResourceProvider(resourceProvider) {}

ResourceProvider::RenderPipelineCache::~RenderPipelineCache() {
    SkASSERT(0 == fMap.count());
}

void ResourceProvider::RenderPipelineCache::release() {
    fMap.reset();
}

RenderPipeline* ResourceProvider::RenderPipelineCache::refPipeline(
        const RenderPipelineDesc& desc) {
    std::unique_ptr<Entry>* entry = fMap.find(desc);

    if (!entry) {
        auto pipeline = fResourceProvider->onCreateRenderPipeline(desc);
        if (!pipeline) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(std::move(pipeline))));
    }
    return (*entry)->fPipeline.get();
}

sk_sp<Texture> ResourceProvider::findOrCreateTexture(SkISize dimensions, const TextureInfo& info) {
    return this->createTexture(dimensions, info);
}

} // namespace skgpu
