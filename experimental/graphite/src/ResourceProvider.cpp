/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ResourceProvider.h"

#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/GraphicsPipeline.h"
#include "experimental/graphite/src/Texture.h"

namespace skgpu {

ResourceProvider::ResourceProvider(const Gpu* gpu) : fGpu(gpu) {
    fGraphicsPipelineCache.reset(new GraphicsPipelineCache(this));
}

ResourceProvider::~ResourceProvider() {
    fGraphicsPipelineCache.release();
}

sk_sp<GraphicsPipeline> ResourceProvider::findOrCreateGraphicsPipeline(
        const GraphicsPipelineDesc& desc) {
    return fGraphicsPipelineCache->refPipeline(desc);
}

////////////////////////////////////////////////////////////////////////////////////////////////

struct ResourceProvider::GraphicsPipelineCache::Entry {
    Entry(sk_sp<GraphicsPipeline> pipeline) : fPipeline(std::move(pipeline)) {}

    sk_sp<GraphicsPipeline> fPipeline;
};

ResourceProvider::GraphicsPipelineCache::GraphicsPipelineCache(ResourceProvider* resourceProvider)
    : fMap(16) // TODO: find a good value for this
    , fResourceProvider(resourceProvider) {}

ResourceProvider::GraphicsPipelineCache::~GraphicsPipelineCache() {
    SkASSERT(0 == fMap.count());
}

void ResourceProvider::GraphicsPipelineCache::release() {
    fMap.reset();
}

sk_sp<GraphicsPipeline> ResourceProvider::GraphicsPipelineCache::refPipeline(
        const GraphicsPipelineDesc& desc) {
    std::unique_ptr<Entry>* entry = fMap.find(desc);

    if (!entry) {
        auto pipeline = fResourceProvider->onCreateGraphicsPipeline(desc);
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
