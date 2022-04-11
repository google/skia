/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ResourceProvider.h"

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/Gpu.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"

namespace skgpu::graphite {

ResourceProvider::ResourceProvider(const Gpu* gpu,
                                   sk_sp<GlobalCache> globalCache,
                                   SingleOwner* singleOwner)
        : fGpu(gpu)
        , fResourceCache(ResourceCache::Make(singleOwner))
        , fGlobalCache(std::move(globalCache)) {
    SkASSERT(fResourceCache);
    fGraphicsPipelineCache.reset(new GraphicsPipelineCache(this));
}

ResourceProvider::~ResourceProvider() {
    fGraphicsPipelineCache.release();
    fResourceCache->shutdown();
}

sk_sp<GraphicsPipeline> ResourceProvider::findOrCreateGraphicsPipeline(
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    return fGraphicsPipelineCache->refPipeline(fGpu->caps(), pipelineDesc, renderPassDesc);
}

SkShaderCodeDictionary* ResourceProvider::shaderCodeDictionary() const {
    return fGlobalCache->shaderCodeDictionary();
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
        const Caps* caps,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    UniqueKey pipelineKey = caps->makeGraphicsPipelineKey(pipelineDesc, renderPassDesc);

	std::unique_ptr<Entry>* entry = fMap.find(pipelineKey);

    if (!entry) {
        auto pipeline = fResourceProvider->onCreateGraphicsPipeline(pipelineDesc, renderPassDesc);
        if (!pipeline) {
            return nullptr;
        }
        entry = fMap.insert(pipelineKey, std::unique_ptr<Entry>(new Entry(std::move(pipeline))));
    }
    return (*entry)->fPipeline;
}

sk_sp<Texture> ResourceProvider::findOrCreateScratchTexture(SkISize dimensions,
                                                            const TextureInfo& info) {
    SkASSERT(info.isValid());

    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    // Scratch textures are not shareable
    fGpu->caps()->buildKeyForTexture(dimensions, info, kType, Shareable::kNo, &key);

    return this->findOrCreateTextureWithKey(dimensions, info, key);
}

sk_sp<Texture> ResourceProvider::findOrCreateDepthStencilAttachment(SkISize dimensions,
                                                                    const TextureInfo& info) {
    SkASSERT(info.isValid());

    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    // We always make depth and stencil attachments shareable. Between any render pass the values
    // are reset. Thus it is safe to be used by multiple different render passes without worry of
    // stomping on each other's data.
    fGpu->caps()->buildKeyForTexture(dimensions, info, kType, Shareable::kYes, &key);

    return this->findOrCreateTextureWithKey(dimensions, info, key);
}

sk_sp<Texture> ResourceProvider::findOrCreateDiscardableMSAAAttachment(SkISize dimensions,
                                                                       const TextureInfo& info) {
    SkASSERT(info.isValid());

    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    // We always make discardable msaa attachments shareable. Between any render pass we discard
    // the values of the MSAA texture. Thus it is safe to be used by multiple different render
    // passes without worry of stomping on each other's data. It is the callings code responsiblity
    // to populate the discardable MSAA texture with data at the start of the render pass.
    fGpu->caps()->buildKeyForTexture(dimensions, info, kType, Shareable::kYes, &key);

    return this->findOrCreateTextureWithKey(dimensions, info, key);
}

sk_sp<Texture> ResourceProvider::findOrCreateTextureWithKey(SkISize dimensions,
                                                            const TextureInfo& info,
                                                            const GraphiteResourceKey& key) {
    if (Resource* resource = fResourceCache->findAndRefResource(key)) {
        return sk_sp<Texture>(static_cast<Texture*>(resource));
    }

    auto tex = this->createTexture(dimensions, info);
    if (!tex) {
        return nullptr;
    }

    tex->setKey(key);
    fResourceCache->insertResource(tex.get());

    return tex;
}

sk_sp<Sampler> ResourceProvider::findOrCreateCompatibleSampler(const SkSamplingOptions& smplOptions,
                                                               SkTileMode xTileMode,
                                                               SkTileMode yTileMode) {
    return this->createSampler(smplOptions, xTileMode, yTileMode);
}

sk_sp<Buffer> ResourceProvider::findOrCreateBuffer(size_t size,
                                                   BufferType type,
                                                   PrioritizeGpuReads prioritizeGpuReads) {
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    {
        // For the key we need ((sizeof(size_t) + (sizeof(uint32_t) - 1)) / (sizeof(uint32_t))
        // uint32_t's for the size and one uint32_t for the rest.
        static_assert(sizeof(uint32_t) == 4);
        static const int kSizeKeyNum32DataCnt = (sizeof(size_t) + 3) / 4;
        static const int kKeyNum32DataCnt =  kSizeKeyNum32DataCnt + 1;

        SkASSERT(static_cast<uint32_t>(type)               < (1u << 3));
        SkASSERT(static_cast<uint32_t>(prioritizeGpuReads) < (1u << 1));

        GraphiteResourceKey::Builder builder(&key, kType, kKeyNum32DataCnt, Shareable::kNo);
        builder[0] = (static_cast<uint32_t>(type)               << 0) |
                     (static_cast<uint32_t>(prioritizeGpuReads) << 3);
        size_t szKey = size;
        for (int i = 0; i < kSizeKeyNum32DataCnt; ++i) {
            builder[i + 1] = (uint32_t) szKey;
            szKey = szKey >> 32;
        }
    }

    if (Resource* resource = fResourceCache->findAndRefResource(key)) {
        return sk_sp<Buffer>(static_cast<Buffer*>(resource));
    }
    auto buffer = this->createBuffer(size, type, prioritizeGpuReads);
    if (!buffer) {
        return nullptr;
    }

    buffer->setKey(key);
    fResourceCache->insertResource(buffer.get());
    return buffer;
}


} // namespace skgpu::graphite
