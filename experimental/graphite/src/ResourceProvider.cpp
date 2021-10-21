/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ResourceProvider.h"

#include "experimental/graphite/src/Buffer.h"
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

sk_sp<RenderPipeline> ResourceProvider::findOrCreateRenderPipeline(const RenderPipelineDesc& desc) {
    return fRenderPipelineCache->refPipeline(desc);
}

////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceProvider::RenderPipelineCache::Entry {
    Entry(sk_sp<RenderPipeline> pipeline)
            : fPipeline(std::move(pipeline)) {}

    sk_sp<RenderPipeline> fPipeline;
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

sk_sp<RenderPipeline> ResourceProvider::RenderPipelineCache::refPipeline(
        const RenderPipelineDesc& desc) {
    std::unique_ptr<Entry>* entry = fMap.find(desc);

    if (!entry) {
        auto pipeline = fResourceProvider->onCreateRenderPipeline(desc);
        if (!pipeline) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(std::move(pipeline))));
    }
    return (*entry)->fPipeline;
}

sk_sp<Texture> ResourceProvider::findOrCreateTexture(SkISize dimensions, const TextureInfo& info) {
    return this->createTexture(dimensions, info);
}

sk_sp<Buffer> ResourceProvider::findOrCreateBuffer(size_t size,
                                                   BufferType type,
                                                   PrioritizeGpuReads prioritizeGpuReads) {
    return this->createBuffer(size, type, prioritizeGpuReads);
}


} // namespace skgpu
