/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ResourceProvider_DEFINED
#define skgpu_graphite_ResourceProvider_DEFINED

#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/ResourceTypes.h"

struct AHardwareBuffer;
struct SkSamplingOptions;
class SkTraceMemoryDump;

namespace skgpu {
class SingleOwner;
}

namespace SkSL {
    class Compiler;
}

namespace skgpu::graphite {

class BackendTexture;
class Buffer;
class Caps;
class ComputePipeline;
class ComputePipelineDesc;
class GlobalCache;
class GraphicsPipelineDesc;
class GraphicsPipelineHandle;
class GraphiteResourceKey;
class ResourceCache;
class RuntimeEffectDictionary;
class ShaderCodeDictionary;
class Sampler;
class SharedContext;
class Texture;
class TextureInfo;

class ResourceProvider {
public:
    virtual ~ResourceProvider();

    GraphicsPipelineHandle createGraphicsPipelineHandle(
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>);
    void startPipelineCreationTask(sk_sp<const RuntimeEffectDictionary>,
                                   const GraphicsPipelineHandle&);
    sk_sp<GraphicsPipeline> resolveHandle(const GraphicsPipelineHandle&);

    sk_sp<GraphicsPipeline> findGraphicsPipeline(
            const UniqueKey& pipelineKey,
            SkEnumBitMask<PipelineCreationFlags>,
            uint32_t *compilationID = nullptr);

    // The runtime effect dictionary provides a link between SkCodeSnippetIds referenced in the
    // paint key and the current SkRuntimeEffect that provides the SkSL for that id.
    sk_sp<GraphicsPipeline> findOrCreateGraphicsPipeline(
            const RuntimeEffectDictionary*,
            const UniqueKey& pipelineKey,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>);

    sk_sp<ComputePipeline> findOrCreateComputePipeline(const ComputePipelineDesc&);

    sk_sp<Texture> findOrCreateShareableTexture(SkISize,
                                                const TextureInfo&,
                                                std::string_view label);

    sk_sp<Texture> findOrCreateNonShareableTexture(SkISize,
                                                   const TextureInfo&,
                                                   std::string_view label,
                                                   Budgeted);
    sk_sp<Texture> findOrCreateScratchTexture(SkISize,
                                              const TextureInfo&,
                                              std::string_view label,
                                              const ResourceCache::ScratchResourceSet& unavailable);

    sk_sp<Texture> createWrappedTexture(const BackendTexture&, std::string_view label);

    sk_sp<Buffer> findOrCreateBuffer(size_t size,
                                     BufferType type,
                                     AccessPattern,
                                     std::string_view label);

    sk_sp<Sampler> findOrCreateCompatibleSampler(const SamplerDesc&);

    BackendTexture createBackendTexture(SkISize dimensions, const TextureInfo&);
    void deleteBackendTexture(const BackendTexture&);

    ProxyCache* proxyCache() { return fResourceCache->proxyCache(); }

    void setResourceCacheLimit(size_t bytes) { return fResourceCache->setMaxBudget(bytes); }
    size_t getResourceCacheLimit() const { return fResourceCache->getMaxBudget(); }
    size_t getResourceCacheCurrentBudgetedBytes() const {
        return fResourceCache->currentBudgetedBytes();
    }
    size_t getResourceCacheCurrentPurgeableBytes() const {
        return fResourceCache->currentPurgeableBytes();
    }

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
        fResourceCache->dumpMemoryStatistics(traceMemoryDump);
    }

    void freeGpuResources();
    void purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime);
    void forceProcessReturnedResources() { fResourceCache->forceProcessReturnedResources(); }

#if defined(GPU_TEST_UTILS)
    ResourceCache* resourceCache() { return fResourceCache.get(); }
    const SharedContext* sharedContext() { return fSharedContext; }
#endif

    const Caps* caps() const;

#ifdef SK_BUILD_FOR_ANDROID
    virtual BackendTexture createBackendTexture(AHardwareBuffer*,
                                                bool isRenderable,
                                                bool isProtectedContent,
                                                SkISize dimensions,
                                                bool fromAndroidWindow) const;
#endif

protected:
    ResourceProvider(SharedContext* sharedContext,
                     SingleOwner* singleOwner,
                     uint32_t recorderID,
                     size_t resourceBudget);

    SharedContext* fSharedContext;
    // Each ResourceProvider owns one local cache; for some resources it also refers out to the
    // global cache of the SharedContext, which is assumed to outlive the ResourceProvider.
    sk_sp<ResourceCache> fResourceCache;

private:
    virtual sk_sp<GraphicsPipeline> createGraphicsPipeline(
            const RuntimeEffectDictionary*,
            const UniqueKey&,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>,
            uint32_t compilationID) = 0;
    virtual sk_sp<ComputePipeline> createComputePipeline(const ComputePipelineDesc&) = 0;
    virtual sk_sp<Texture> createTexture(SkISize, const TextureInfo&) = 0;
    virtual sk_sp<Buffer> createBuffer(size_t size, BufferType type, AccessPattern) = 0;
    virtual sk_sp<Sampler> createSampler(const SamplerDesc&) = 0;

    sk_sp<Texture> findOrCreateTexture(SkISize dimensions,
                                       const TextureInfo& info,
                                       std::string_view label,
                                       Budgeted,
                                       Shareable,
                                       const ResourceCache::ScratchResourceSet* = nullptr);

    virtual sk_sp<Texture> onCreateWrappedTexture(const BackendTexture&) = 0;

    virtual BackendTexture onCreateBackendTexture(SkISize dimensions, const TextureInfo&) = 0;
#ifdef SK_BUILD_FOR_ANDROID
    virtual BackendTexture onCreateBackendTexture(AHardwareBuffer*,
                                                  bool isRenderable,
                                                  bool isProtectedContent,
                                                  SkISize dimensions,
                                                  bool fromAndroidWindow) const;
#endif
    virtual void onDeleteBackendTexture(const BackendTexture&) = 0;

    virtual void onFreeGpuResources() {}
    virtual void onPurgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime) {}
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceProvider_DEFINED
