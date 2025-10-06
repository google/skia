/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SharedContext_DEFINED
#define skgpu_graphite_SharedContext_DEFINED

#include <memory>
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/base/SingleOwner.h"

#include "include/gpu/graphite/GraphiteTypes.h"
#include "src/capture/SkCaptureManager.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/PipelineManager.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

class SkCaptureManager;
class SkExecutor;

namespace skgpu {
class SingleOwner;
}

namespace skgpu::graphite {

class BackendTexture;
class Caps;
class CommandBuffer;
class Context;
class PersistentPipelineStorage;
class RendererProvider;
class ResourceProvider;
class TextureInfo;
class ThreadSafeResourceProvider;

class SharedContext : public SkRefCnt {
public:
    ~SharedContext() override;

    /**
     * Gets the capabilities of the draw target.
     */
    const Caps* caps() const { return fCaps.get(); }

    BackendApi backend() const { return fBackend; }
    Protected isProtected() const;

    GlobalCache* globalCache() { return &fGlobalCache; }
    const GlobalCache* globalCache() const { return &fGlobalCache; }

    PipelineManager* pipelineManager() { return &fPipelineManager; }

    const RendererProvider* rendererProvider() const { return fRendererProvider.get(); }

    ShaderCodeDictionary* shaderCodeDictionary() { return &fShaderDictionary; }
    const ShaderCodeDictionary* shaderCodeDictionary() const { return &fShaderDictionary; }

    virtual std::unique_ptr<ResourceProvider> makeResourceProvider(SingleOwner*,
                                                                   uint32_t recorderID,
                                                                   size_t resourceBudget) = 0;

    // The runtime effect dictionary provides a link between SkCodeSnippetIds referenced in the
    // paint key and the current SkRuntimeEffect that provides the SkSL for that id.
    sk_sp<GraphicsPipeline> findOrCreateGraphicsPipeline(
            const RuntimeEffectDictionary*,
            const UniqueKey& pipelineKey,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>);

    // Called by Context::isContextLost(). Returns true if the backend-specific SharedContext has
    // gotten into an unrecoverable, lost state.
    virtual bool isDeviceLost() const { return false; }

    virtual void deviceTick(Context*) {}

    virtual void syncPipelineData(PersistentPipelineStorage*, size_t maxSize) {}

    SkCaptureManager* captureManager() { return fCaptureManager.get(); }

#if defined(SK_DEBUG)
    size_t getResourceCacheLimit() const;
    size_t getResourceCacheCurrentBudgetedBytes() const;
    size_t getResourceCacheCurrentPurgeableBytes() const;
#endif

    void dumpMemoryStatistics(SkTraceMemoryDump*) const;
    void freeGpuResources();
    void purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime);
    void forceProcessReturnedResources();

protected:
    SharedContext(std::unique_ptr<const Caps>,
                  BackendApi,
                  SkExecutor* executor,
                  SkSpan<sk_sp<SkRuntimeEffect>> userDefinedKnownRuntimeEffects);

    // All the resources in the ThreadSafeResourceProvider should be 0-sized.
    static constexpr size_t kThreadedSafeResourceBudget = 256;

    // This SingleOwner is for ResourceProvider wrapped by the fThreadSafeResourceProvider.
    // It can't be in that class bc each backend-specific ShareContext must make its own
    // backend-specific wrapped ResourceProvider in its constructor.
    mutable SingleOwner fSingleOwner;
    std::unique_ptr<ThreadSafeResourceProvider> fThreadSafeResourceProvider;

private:
    friend class Context; // for setRendererProvider() and setCaptureManager()

    virtual sk_sp<GraphicsPipeline> createGraphicsPipeline(
            const RuntimeEffectDictionary*,
            const UniqueKey&,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>,
            uint32_t compilationID) = 0;

    // Must be created out-of-band to allow RenderSteps to use a QueueManager.
    void setRendererProvider(std::unique_ptr<RendererProvider> rendererProvider);

    void setCaptureManager(sk_sp<SkCaptureManager> captureManager);

    std::unique_ptr<const Caps> fCaps; // Provided by backend subclass

    BackendApi fBackend;
    GlobalCache fGlobalCache;
    PipelineManager fPipelineManager;
    std::unique_ptr<RendererProvider> fRendererProvider;
    ShaderCodeDictionary fShaderDictionary;
    sk_sp<SkCaptureManager> fCaptureManager;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_SharedContext_DEFINED
