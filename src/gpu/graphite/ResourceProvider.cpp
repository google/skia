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
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/Texture.h"
#include "src/sksl/SkSLCompiler.h"

namespace skgpu::graphite {

// This is only used when tracing is enabled at compile time.
[[maybe_unused]] static std::string to_str(const SharedContext* ctx,
                                           const GraphicsPipelineDesc& gpDesc,
                                           const RenderPassDesc& rpDesc) {
    const ShaderCodeDictionary* dict = ctx->shaderCodeDictionary();
    const RenderStep* step = ctx->rendererProvider()->lookup(gpDesc.renderStepID());
    return GetPipelineLabel(dict, rpDesc, step, gpDesc.paintParamsID());
}

ResourceProvider::ResourceProvider(SharedContext* sharedContext,\
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
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {

    auto globalCache = fSharedContext->globalCache();
    UniqueKey pipelineKey = fSharedContext->caps()->makeGraphicsPipelineKey(pipelineDesc,
                                                                            renderPassDesc);

    uint32_t compilationID = 0;
    sk_sp<GraphicsPipeline> pipeline = globalCache->findGraphicsPipeline(pipelineKey,
                                                                         pipelineCreationFlags,
                                                                         &compilationID);
    if (pipeline && pipeline->didAsyncCompilationFail()) {
        // If the pipeline failed, remove it from the cache and fall through to retry
        globalCache->removeGraphicsPipeline(pipeline.get());
        pipeline.reset();
    }

    if (!pipeline) {
        // Haven't encountered this pipeline, so create a new one. Since pipelines are shared
        // across Recorders, we could theoretically create equivalent pipelines on different
        // threads. If this happens, GlobalCache returns the first-through-gate pipeline and we
        // discard the redundant pipeline. While this is wasted effort in the rare event of a race,
        // it allows pipeline creation to be performed without locking the global cache.
        // NOTE: The parameters to TRACE_EVENT are only evaluated inside an if-block when the
        // category is enabled.
        TRACE_EVENT1_ALWAYS(
                "skia.shaders", "createGraphicsPipeline", "desc",
                TRACE_STR_COPY(to_str(fSharedContext, pipelineDesc, renderPassDesc).c_str()));

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
        bool forPrecompile =
                SkToBool(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation);

        static const char* kNames[2] = { "BeginBuildN", "BeginBuildP" };
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[forPrecompile]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", pipelineKey.hash(),
                             "compilationID", compilationID);
#endif

        pipeline = this->createGraphicsPipeline(runtimeDict, pipelineKey,
                                                pipelineDesc, renderPassDesc,
                                                pipelineCreationFlags,
                                                compilationID);
        if (pipeline) {
            globalCache->invokePipelineCallback(fSharedContext, pipelineDesc, renderPassDesc);
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

sk_sp<Texture> ResourceProvider::findOrCreateShareableTexture(SkISize dimensions,
                                                              const TextureInfo& info,
                                                              std::string_view label) {
    return this->findOrCreateTexture(dimensions,
                                     info,
                                     std::move(label),
                                     Budgeted::kYes,
                                     Shareable::kYes);
}

sk_sp<Texture> ResourceProvider::findOrCreateNonShareableTexture(SkISize dimensions,
                                                                 const TextureInfo& info,
                                                                 std::string_view label,
                                                                 Budgeted budgeted) {
    return this->findOrCreateTexture(dimensions,
                                     info,
                                     std::move(label),
                                     budgeted,
                                     Shareable::kNo);
}

sk_sp<Texture> ResourceProvider::findOrCreateScratchTexture(
        SkISize dimensions,
        const TextureInfo& info,
        std::string_view label,
        const ResourceCache::ScratchResourceSet& unavailable) {
    return this->findOrCreateTexture(dimensions,
                                     info,
                                     std::move(label),
                                     Budgeted::kYes,
                                     Shareable::kScratch,
                                     &unavailable);
}

sk_sp<Texture> ResourceProvider::findOrCreateTexture(
        SkISize dimensions,
        const TextureInfo& info,
        std::string_view label,
        Budgeted budgeted,
        Shareable shareable,
        const ResourceCache::ScratchResourceSet* unavailable) {
    // If the resource is shareable it should be budgeted since it shouldn't be backing any client
    // owned object.
    SkASSERT(shareable == Shareable::kNo || budgeted == Budgeted::kYes);
    SkASSERT(shareable != Shareable::kScratch || SkToBool(unavailable));

    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    if (!info.isValid()) {
        // Checking for a valid TextureInfo here allows callers to consolidate error checking for
        // both TextureInfo and Texture creation to checking for a null returned Texture.
        return nullptr;
    }

    GraphiteResourceKey key;
    fSharedContext->caps()->buildKeyForTexture(dimensions, info, kType, &key);

    if (Resource* resource =
                fResourceCache->findAndRefResource(key, budgeted, shareable, unavailable)) {
        resource->setLabel(std::move(label));
        return sk_sp<Texture>(static_cast<Texture*>(resource));
    }

    auto tex = this->createTexture(dimensions, info);
    if (!tex) {
        return nullptr;
    }

    tex->setLabel(std::move(label));
    fResourceCache->insertResource(tex.get(), key, budgeted, shareable);

    return tex;
}

sk_sp<Texture> ResourceProvider::createWrappedTexture(const BackendTexture& backendTexture,
                                                      std::string_view label) {
    sk_sp<Texture> texture = this->onCreateWrappedTexture(backendTexture);
    if (texture) {
        texture->setLabel(std::move(label));
        SkASSERT(texture->ownership() == Ownership::kWrapped);
    }
    return texture;
}

sk_sp<Sampler> ResourceProvider::findOrCreateCompatibleSampler(const SamplerDesc& samplerDesc) {
    static constexpr Budgeted kBudgeted = Budgeted::kYes;
    static constexpr Shareable kShareable = Shareable::kYes;
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    {
        // The size of the returned span accurately captures the quantity of uint32s needed whether
        // the sampler is immutable or not. Each backend will already have encoded any specific
        // immutable sampler details into the SamplerDesc, so there is no need to delegate to Caps
        // to create a specific key.
        const SkSpan<const uint32_t>& samplerData = samplerDesc.asSpan();
        GraphiteResourceKey::Builder builder(&key, kType, samplerData.size());

        for (size_t i = 0; i < samplerData.size(); i++) {
            builder[i] = samplerData[i];
        }
    }

    if (Resource* resource = fResourceCache->findAndRefResource(key, kBudgeted, kShareable)) {
        return sk_sp<Sampler>(static_cast<Sampler*>(resource));
    }

    sk_sp<Sampler> sampler = this->createSampler(samplerDesc);
    if (!sampler) {
        return nullptr;
    }

    fResourceCache->insertResource(sampler.get(), key, kBudgeted, kShareable);
    return sampler;
}

sk_sp<Buffer> ResourceProvider::findOrCreateBuffer(size_t size,
                                                   BufferType type,
                                                   AccessPattern accessPattern,
                                                   std::string_view label) {
    static constexpr Budgeted kBudgeted = Budgeted::kYes;
    static constexpr Shareable kShareable = Shareable::kNo;
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    GraphiteResourceKey key;
    {
        // For the key we need ((sizeof(size_t) + (sizeof(uint32_t) - 1)) / (sizeof(uint32_t))
        // uint32_t's for the size and one uint32_t for the rest.
        static_assert(sizeof(uint32_t) == 4);
        static const int kSizeKeyNum32DataCnt = (sizeof(size_t) + 3) / 4;
        static const int kKeyNum32DataCnt =  kSizeKeyNum32DataCnt + 1;

        SkASSERT(static_cast<uint32_t>(type) < (1u << 4));
        SkASSERT(static_cast<uint32_t>(accessPattern) < (1u << 2));

        GraphiteResourceKey::Builder builder(&key, kType, kKeyNum32DataCnt);
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

    if (Resource* resource = fResourceCache->findAndRefResource(key, kBudgeted, kShareable)) {
        resource->setLabel(std::move(label));
        return sk_sp<Buffer>(static_cast<Buffer*>(resource));
    }
    auto buffer = this->createBuffer(size, type, accessPattern);
    if (!buffer) {
        return nullptr;
    }

    buffer->setLabel(std::move(label));
    fResourceCache->insertResource(buffer.get(), key, kBudgeted, kShareable);
    return buffer;
}

namespace {
bool dimensions_are_valid(const int maxTextureSize, const SkISize& dimensions) {
    if (dimensions.isEmpty() ||
        dimensions.width()  > maxTextureSize ||
        dimensions.height() > maxTextureSize) {
        SKGPU_LOG_W("Call to createBackendTexture has requested dimensions (%d, %d) larger than the"
                    " supported gpu max texture size: %d. Or the dimensions are empty.",
                    dimensions.fWidth, dimensions.fHeight, maxTextureSize);
        return false;
    }
    return true;
}
}

BackendTexture ResourceProvider::createBackendTexture(SkISize dimensions, const TextureInfo& info) {
    if (!dimensions_are_valid(fSharedContext->caps()->maxTextureSize(), dimensions)) {
        return {};
    }
    return this->onCreateBackendTexture(dimensions, info);
}

#ifdef SK_BUILD_FOR_ANDROID
BackendTexture ResourceProvider::createBackendTexture(AHardwareBuffer* hardwareBuffer,
                                                      bool isRenderable,
                                                      bool isProtectedContent,
                                                      SkISize dimensions,
                                                      bool fromAndroidWindow) const {
    if (!dimensions_are_valid(fSharedContext->caps()->maxTextureSize(), dimensions)) {
        return {};
    }
    return this->onCreateBackendTexture(hardwareBuffer,
                                        isRenderable,
                                        isProtectedContent,
                                        dimensions,
                                        fromAndroidWindow);
}

BackendTexture ResourceProvider::onCreateBackendTexture(AHardwareBuffer*,
                                                        bool isRenderable,
                                                        bool isProtectedContent,
                                                        SkISize dimensions,
                                                        bool fromAndroidWindow) const {
    return {};
}
#endif

void ResourceProvider::deleteBackendTexture(const BackendTexture& texture) {
    this->onDeleteBackendTexture(texture);
}

void ResourceProvider::freeGpuResources() {
    this->onFreeGpuResources();

    // TODO: Are there Resources that are ref'd by the ResourceProvider or its subclasses that need
    // be released? If we ever find that we're holding things directly on the ResourceProviders we
    // call down into the subclasses to allow them to release things.

    fResourceCache->purgeResources();
}

void ResourceProvider::purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime) {
    this->onPurgeResourcesNotUsedSince(purgeTime);
    fResourceCache->purgeResourcesNotUsedSince(purgeTime);
}

}  // namespace skgpu::graphite
