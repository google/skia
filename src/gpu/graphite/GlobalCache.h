/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GlobalCache_DEFINED
#define skgpu_graphite_GlobalCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/GraphicsPipeline.h"


#include <functional>

namespace skgpu::graphite {

class ComputePipeline;
class GraphicsPipeline;
class Resource;
class ShaderCodeDictionary;

/**
 * GlobalCache holds GPU resources that should be shared by every Recorder. The common requirement
 * of these resources are they are static/read-only, have long lifetimes, and are likely to be used
 * by multiple Recorders. The canonical example of this are pipelines.
 *
 * GlobalCache is thread safe, but intentionally splits queries and storing operations so that they
 * are not atomic. The pattern is to query for a resource, which has a high likelihood of a cache
 * hit. If it's not found, the Recorder creates the resource on its own, without locking the
 * GlobalCache. After the resource is created, it is added to the GlobalCache, atomically returning
 * the winning Resource in the event of a race between Recorders for the same UniqueKey.
 */
class GlobalCache {
public:
    GlobalCache();
    ~GlobalCache();

    void deleteResources();

    // Find a cached GraphicsPipeline that matches the associated key.
    sk_sp<GraphicsPipeline> findGraphicsPipeline(
        const UniqueKey&,
        SkEnumBitMask<PipelineCreationFlags> = PipelineCreationFlags::kNone,
        uint32_t* compilationID = nullptr) SK_EXCLUDES(fSpinLock);

    // Associate the given pipeline with the key. If the key has already had a separate pipeline
    // associated with the key, that pipeline is returned and the passed-in pipeline is discarded.
    // Otherwise, the passed-in pipeline is held by the GlobalCache and also returned back.
    sk_sp<GraphicsPipeline> addGraphicsPipeline(const UniqueKey&,
                                                sk_sp<GraphicsPipeline>) SK_EXCLUDES(fSpinLock);
    // Remove the GraphicsPipeline from the cache, if possible. This does nothing if the pipeline
    // is not held in the cache. This removes based on actual pipeline object, not by key. When
    // pipeline compilation has transient failures, it is possible for multiple GraphicsPipelines to
    // be created that have the same key.
    void removeGraphicsPipeline(const GraphicsPipeline*) SK_EXCLUDES(fSpinLock);

    void purgePipelinesNotUsedSince(
            StdSteadyClock::time_point purgeTime) SK_EXCLUDES(fSpinLock);

    void reportPrecompileStats() SK_EXCLUDES(fSpinLock);
    void reportCacheStats() SK_EXCLUDES(fSpinLock);

#if defined(GPU_TEST_UTILS)
    int numGraphicsPipelines() const SK_EXCLUDES(fSpinLock);
    void resetGraphicsPipelines() SK_EXCLUDES(fSpinLock);
    void forEachGraphicsPipeline(
            const std::function<void(const UniqueKey&, const GraphicsPipeline*)>& fn)
            SK_EXCLUDES(fSpinLock);
    uint16_t getEpoch() const SK_EXCLUDES(fSpinLock);
    void forceNextEpochOverflow() SK_EXCLUDES(fSpinLock);
#endif

    struct PipelineStats {
#if defined(GPU_TEST_UTILS)
        int fGraphicsCacheHits = 0;
        int fGraphicsCacheMisses = 0;
        int fGraphicsCacheAdditions = 0;
        int fGraphicsRaces = 0;
        int fGraphicsPurges = 0;
#endif
        // Normally compiled Pipelines that were skipped bc of a preexisting Precompiled Pipeline
        uint32_t fNormalPreemptedByPrecompile = 0;
        // Precompiled Pipelines that made it into the cache
        uint32_t fUnpreemptedPrecompilePipelines = 0;
        // Precompiled Pipelines that were purged from the cache prior to use
        uint32_t fPurgedUnusedPrecompiledPipelines = 0;
        // The number of Pipelines requested since the last call to reportCacheStats
        uint32_t fPipelineUsesInEpoch = 0;
    };

    PipelineStats getStats() const SK_EXCLUDES(fSpinLock);

    // Find and add operations for ComputePipelines, with the same pattern as GraphicsPipelines.
    sk_sp<ComputePipeline> findComputePipeline(const UniqueKey&) SK_EXCLUDES(fSpinLock);
    sk_sp<ComputePipeline> addComputePipeline(const UniqueKey&,
                                              sk_sp<ComputePipeline>) SK_EXCLUDES(fSpinLock);

    // The GlobalCache holds a ref on the given Resource until the cache is destroyed, keeping it
    // alive for the lifetime of the SharedContext. This should be used only for Resources that are
    // immutable after initialization so that anyone can use the resource without synchronization
    // or reference tracking.
    void addStaticResource(sk_sp<Resource>) SK_EXCLUDES(fSpinLock);

    using PipelineCallbackContext = void*;
    using PipelineCallback = void (*)(PipelineCallbackContext context, sk_sp<SkData> pipelineData);
    void setPipelineCallback(PipelineCallback, PipelineCallbackContext) SK_EXCLUDES(fSpinLock);

    void invokePipelineCallback(SharedContext*,
                                const GraphicsPipelineDesc&,
                                const RenderPassDesc&);

#if defined(GPU_TEST_UTILS)
    struct StaticVertexCopyRanges {
        uint32_t fOffset;
        size_t fUnalignedSize;
        size_t fSize;
        size_t fRequiredAlignment;
    };
    void testingOnly_SetStaticVertexInfo(skia_private::TArray<StaticVertexCopyRanges>,
                                         const Buffer*) SK_EXCLUDES(fSpinLock);
    SkSpan<const StaticVertexCopyRanges> getStaticVertexCopyRanges() const SK_EXCLUDES(fSpinLock);
    sk_sp<Buffer> getStaticVertexBuffer() SK_EXCLUDES(fSpinLock);
#endif
private:
    struct KeyHash {
        uint32_t operator()(const UniqueKey& key) const { return key.hash(); }
    };

    static void LogPurge(void* context, const UniqueKey& key, sk_sp<GraphicsPipeline>* p);
    struct PurgeCB {
        void operator()(void* context, const UniqueKey& k, sk_sp<GraphicsPipeline>* p) const {
            LogPurge(context, k, p);
        }
    };

    using GraphicsPipelineCache = SkLRUCache<UniqueKey, sk_sp<GraphicsPipeline>, KeyHash, PurgeCB>;
    using ComputePipelineCache  = SkLRUCache<UniqueKey, sk_sp<ComputePipeline>,  KeyHash>;

    // TODO: can we do something better given this should have write-seldom/read-often behavior?
    mutable SkSpinlock fSpinLock;

    // GraphicsPipelines and ComputePipelines are expensive to create, likely to be used by multiple
    // Recorders, and are ideally pre-compiled on process startup so thread write-contention is
    // expected to be low. For these reasons we store pipelines globally instead of per-Recorder.
    GraphicsPipelineCache fGraphicsPipelineCache SK_GUARDED_BY(fSpinLock);
    ComputePipelineCache  fComputePipelineCache  SK_GUARDED_BY(fSpinLock);

    skia_private::TArray<sk_sp<Resource>> fStaticResource SK_GUARDED_BY(fSpinLock);

    PipelineCallback fPipelineCallback SK_GUARDED_BY(fSpinLock) = nullptr;
    PipelineCallbackContext fPipelineCallbackContext SK_GUARDED_BY(fSpinLock) = nullptr;

    PipelineStats fStats SK_GUARDED_BY(fSpinLock);

    // An epoch is the span of time between calls to PrecompileContext::reportPipelineStats.
    // Every Pipeline will be marked with the epoch in which it was created and then updated
    // for each epoch in which it was used.
    uint16_t fEpochCounter SK_GUARDED_BY(fSpinLock) = 1;

#if defined(GPU_TEST_UTILS)
    skia_private::TArray<StaticVertexCopyRanges> fStaticVertexInfo SK_GUARDED_BY(fSpinLock);
    const Buffer* fStaticVertexBuffer SK_GUARDED_BY(fSpinLock);
#endif
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_GlobalCache_DEFINED
