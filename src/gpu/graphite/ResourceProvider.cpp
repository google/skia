/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ResourceProvider.h"

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/Texture.h"
#include "src/sksl/SkSLCompiler.h"

namespace skgpu::graphite {

ResourceProvider::ResourceProvider(SharedContext* sharedContext,
                                   SingleOwner* singleOwner,
                                   uint32_t recorderID,
                                   size_t resourceBudget)
        : fSharedContext(sharedContext)
        , fResourceCache(ResourceCache::Make(singleOwner, recorderID, resourceBudget)) {}

ResourceProvider::~ResourceProvider() {
    fResourceCache->shutdown();
}

sk_sp<GraphicsPipeline> ResourceProvider::findOrCreateGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    auto globalCache = fSharedContext->globalCache();
    UniqueKey pipelineKey = fSharedContext->caps()->makeGraphicsPipelineKey(pipelineDesc,
                                                                            renderPassDesc);
    sk_sp<GraphicsPipeline> pipeline = globalCache->findGraphicsPipeline(pipelineKey);
    if (!pipeline) {
        // Haven't encountered this pipeline, so create a new one. Since pipelines are shared
        // across Recorders, we could theoretically create equivalent pipelines on different
        // threads. If this happens, GlobalCache returns the first-through-gate pipeline and we
        // discard the redundant pipeline. While this is wasted effort in the rare event of a race,
        // it allows pipeline creation to be performed without locking the global cache.
        pipeline = this->createGraphicsPipeline(runtimeDict, pipelineDesc, renderPassDesc);
        if (pipeline) {
            // TODO: Should we store a null pipeline if we failed to create one so that subsequent
            // usage immediately sees that the pipeline cannot be created, vs. retrying every time?
            pipeline = globalCache->addGraphicsPipeline(pipelineKey, std::move(pipeline));
        }
    }
    return pipeline;
}

sk_sp<ComputePipeline> ResourceProvider::findOrCreateComputePipeline(
        const ComputePipelineDesc& pipelineDesc) {
    auto globalCache = fSharedContext->globalCache();
    UniqueKey pipelineKey = fSharedContext->caps()->makeComputePipelineKey(pipelineDesc);
    sk_sp<ComputePipeline> pipeline = globalCache->findComputePipeline(pipelineKey);
    if (!pipeline) {
        pipeline = this->createComputePipeline(pipelineDesc);
        if (pipeline) {
            pipeline = globalCache->addComputePipeline(pipelineKey, std::move(pipeline));
        }
    }
    return pipeline;
}

////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<Texture> ResourceProvider::findOrCreateScratchTexture(SkISize dimensions,
                                                            const TextureInfo& info,
                                                            skgpu::Budgeted budgeted) {
    SkASSERT(info.isValid());

    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    // Scratch textures are not shareable
    fSharedContext->caps()->buildKeyForTexture(dimensions, info, kType, Shareable::kNo, &key);

    return this->findOrCreateTextureWithKey(dimensions, info, key, budgeted);
}

sk_sp<Texture> ResourceProvider::findOrCreateDepthStencilAttachment(SkISize dimensions,
                                                                    const TextureInfo& info) {
    SkASSERT(info.isValid());

    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    // We always make depth and stencil attachments shareable. Between any render pass the values
    // are reset. Thus it is safe to be used by multiple different render passes without worry of
    // stomping on each other's data.
    fSharedContext->caps()->buildKeyForTexture(dimensions, info, kType, Shareable::kYes, &key);

    return this->findOrCreateTextureWithKey(dimensions, info, key, skgpu::Budgeted::kYes);
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
    fSharedContext->caps()->buildKeyForTexture(dimensions, info, kType, Shareable::kYes, &key);

    return this->findOrCreateTextureWithKey(dimensions, info, key, skgpu::Budgeted::kYes);
}

sk_sp<Texture> ResourceProvider::findOrCreateTextureWithKey(SkISize dimensions,
                                                            const TextureInfo& info,
                                                            const GraphiteResourceKey& key,
                                                            skgpu::Budgeted budgeted) {
    // If the resource is shareable it should be budgeted since it shouldn't be backing any client
    // owned object.
    SkASSERT(key.shareable() == Shareable::kNo || budgeted == skgpu::Budgeted::kYes);

    if (Resource* resource = fResourceCache->findAndRefResource(key, budgeted)) {
        return sk_sp<Texture>(static_cast<Texture*>(resource));
    }

    auto tex = this->createTexture(dimensions, info, budgeted);
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
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    {
        constexpr int kNumTileModeBits   = SkNextLog2_portable(int(SkTileMode::kLastTileMode)+1);
        constexpr int kNumFilterModeBits = SkNextLog2_portable(int(SkFilterMode::kLast)+1);
        constexpr int kNumMipmapModeBits = SkNextLog2_portable(int(SkMipmapMode::kLast)+1);

        constexpr int kTileModeXShift  = 0;
        constexpr int kTileModeYShift  = kTileModeXShift  + kNumTileModeBits;
        constexpr int kFilterModeShift = kTileModeYShift  + kNumTileModeBits;
        constexpr int kMipmapModeShift = kFilterModeShift + kNumFilterModeBits;

        static_assert(kMipmapModeShift + kNumMipmapModeBits <= 32);

        // For the key we need only one uint32_t.
        // TODO: add aniso value when used
        static_assert(sizeof(uint32_t) == 4);

        GraphiteResourceKey::Builder builder(&key, kType, 1, Shareable::kYes);
        uint32_t myKey =
        (static_cast<uint32_t>(xTileMode) << kTileModeXShift) |
                     (static_cast<uint32_t>(yTileMode) << kTileModeYShift) |
                     (static_cast<uint32_t>(smplOptions.filter) << kFilterModeShift) |
                     (static_cast<uint32_t>(smplOptions.mipmap) << kMipmapModeShift);
        builder[0] = myKey;
    }

    skgpu::Budgeted budgeted = skgpu::Budgeted::kYes;
    if (Resource* resource = fResourceCache->findAndRefResource(key, budgeted)) {
        return sk_sp<Sampler>(static_cast<Sampler*>(resource));
    }
    sk_sp<Sampler> sampler = this->createSampler(smplOptions, xTileMode, yTileMode);
    if (!sampler) {
        return nullptr;
    }

    sampler->setKey(key);
    fResourceCache->insertResource(sampler.get());
    return sampler;
}

sk_sp<Buffer> ResourceProvider::findOrCreateBuffer(size_t size,
                                                   BufferType type,
                                                   AccessPattern accessPattern) {
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

#ifdef SK_DEBUG
    // The size should already be aligned.
    size_t minAlignment = 1;
    if (type == BufferType::kStorage || type == BufferType::kIndirect ||
        type == BufferType::kVertexStorage || type == BufferType::kIndexStorage) {
        minAlignment = std::max(fSharedContext->caps()->requiredStorageBufferAlignment(),
                                minAlignment);
    } else if (type == BufferType::kUniform) {
        minAlignment = std::max(fSharedContext->caps()->requiredUniformBufferAlignment(),
                                minAlignment);
    } else if (type == BufferType::kXferCpuToGpu || type == BufferType::kXferGpuToCpu) {
        minAlignment = std::max(fSharedContext->caps()->requiredTransferBufferAlignment(),
                                minAlignment);
    }
    SkASSERT(size % minAlignment == 0);
#endif

    GraphiteResourceKey key;
    {
        // For the key we need ((sizeof(size_t) + (sizeof(uint32_t) - 1)) / (sizeof(uint32_t))
        // uint32_t's for the size and one uint32_t for the rest.
        static_assert(sizeof(uint32_t) == 4);
        static const int kSizeKeyNum32DataCnt = (sizeof(size_t) + 3) / 4;
        static const int kKeyNum32DataCnt =  kSizeKeyNum32DataCnt + 1;

        SkASSERT(static_cast<uint32_t>(type) < (1u << 4));
        SkASSERT(static_cast<uint32_t>(accessPattern) < (1u << 1));

        GraphiteResourceKey::Builder builder(&key, kType, kKeyNum32DataCnt, Shareable::kNo);
        builder[0] = (static_cast<uint32_t>(type) << 0) |
                     (static_cast<uint32_t>(accessPattern) << 4);
        size_t szKey = size;
        for (int i = 0; i < kSizeKeyNum32DataCnt; ++i) {
            builder[i + 1] = (uint32_t) szKey;

            // If size_t is 4 bytes, we cannot do a shift of 32 or else we get a warning/error that
            // shift amount is >= width of the type.
            if constexpr(kSizeKeyNum32DataCnt > 1) {
                szKey = szKey >> 32;
            }
        }
    }

    skgpu::Budgeted budgeted = skgpu::Budgeted::kYes;
    if (Resource* resource = fResourceCache->findAndRefResource(key, budgeted)) {
        return sk_sp<Buffer>(static_cast<Buffer*>(resource));
    }
    auto buffer = this->createBuffer(size, type, accessPattern);
    if (!buffer) {
        return nullptr;
    }

    buffer->setKey(key);
    fResourceCache->insertResource(buffer.get());
    return buffer;
}

BackendTexture ResourceProvider::createBackendTexture(SkISize dimensions, const TextureInfo& info) {
    const auto maxTextureSize = fSharedContext->caps()->maxTextureSize();
    if (dimensions.isEmpty() ||
        dimensions.width()  > maxTextureSize ||
        dimensions.height() > maxTextureSize) {
        SKGPU_LOG_W("call to createBackendTexture has requested dimensions (%d, %d) larger than the"
                    " supported gpu max texture size: %d. Or the dimensions are empty.",
                    dimensions.fWidth, dimensions.fHeight, maxTextureSize);
        return {};
    }

    return this->onCreateBackendTexture(dimensions, info);
}

void ResourceProvider::deleteBackendTexture(BackendTexture& texture) {
    this->onDeleteBackendTexture(texture);
    // Invalidate the texture;
    texture = BackendTexture();
}

void ResourceProvider::freeGpuResources() {
    // TODO: Are there Resources that are ref'd by the ResourceProvider or its subclasses that need
    // be released? If we ever find that we're holding things directly on the ResourceProviders we
    // call down into the subclasses to allow them to release things.

    fResourceCache->purgeResources();
}

void ResourceProvider::purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime) {
    fResourceCache->purgeResourcesNotUsedSince(purgeTime);
}

}  // namespace skgpu::graphite
