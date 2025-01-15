/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GlobalCache.h"

#include "include/private/base/SkTArray.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/Resource.h"

namespace {

uint32_t next_compilation_id() {
    static std::atomic<uint32_t> nextId{0};
    // Not worried about overflow since we don't expect that many GraphicsPipelines.
    // Even if it wraps around to 0, this is solely for debug logging.
    return nextId.fetch_add(1, std::memory_order_relaxed);
}

#if defined(GPU_TEST_UTILS)
// TODO(jamesgk): get rid of this special case once we've got color space transform shader
// specialization more under control
constexpr int kGlobalGraphicsPipelineCacheSizeLimit = 2048;
constexpr int kGlobalComputePipelineCacheSizeLimit = 256;

#else
// TODO: find a good value for these limits
constexpr int kGlobalGraphicsPipelineCacheSizeLimit = 256;
constexpr int kGlobalComputePipelineCacheSizeLimit = 256;
#endif

} // anonymous namespce

namespace skgpu::graphite {

GlobalCache::GlobalCache()
        : fGraphicsPipelineCache(kGlobalGraphicsPipelineCacheSizeLimit)
        , fComputePipelineCache(kGlobalComputePipelineCacheSizeLimit) {}

GlobalCache::~GlobalCache() {
    // These should have been cleared out earlier by deleteResources().
    SkDEBUGCODE(SkAutoSpinlock lock{ fSpinLock });
    SkASSERT(fGraphicsPipelineCache.count() == 0);
    SkASSERT(fComputePipelineCache.count() == 0);
    SkASSERT(fStaticResource.empty());
}

void GlobalCache::deleteResources() {
    SkAutoSpinlock lock{ fSpinLock };

    fGraphicsPipelineCache.reset();
    fComputePipelineCache.reset();
    fStaticResource.clear();
}

void GlobalCache::LogPurge(const UniqueKey& key, sk_sp<GraphicsPipeline>* p) {
#if defined(SK_PIPELINE_LIFETIME_LOGGING)
    // A "Bad" Purge is one where the Pipeline was never retrieved from the Cache (i.e., unused
    // overgeneration).
    static const char* kNames[2][2] = { { "BadPurgedN", "BadPurgedP" },
                                        { "PurgedN",    "PurgedP"} };

    TRACE_EVENT_INSTANT2("skia.gpu",
                         TRACE_STR_STATIC(kNames[(*p)->wasUsed()][(*p)->fromPrecompile()]),
                         TRACE_EVENT_SCOPE_THREAD,
                         "key", key.hash(),
                         "compilationID", (*p)->getPipelineInfo().fCompilationID);
#endif
}

sk_sp<GraphicsPipeline> GlobalCache::findGraphicsPipeline(
        const UniqueKey& key,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t *compilationID) {

    [[maybe_unused]] bool forPrecompile =
            SkToBool(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation);

    SkAutoSpinlock lock{fSpinLock};

    sk_sp<GraphicsPipeline>* entry = fGraphicsPipelineCache.find(key);
    if (entry) {
#if defined(GPU_TEST_UTILS)
        ++fStats.fGraphicsCacheHits;
#endif

        (*entry)->updateAccessTime();
        (*entry)->markUsed();

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
        static const char* kNames[2] = { "CacheHitForN", "CacheHitForP" };
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[forPrecompile]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             "compilationID", (*entry)->getPipelineInfo().fCompilationID);
#endif

        return *entry;
    } else {
#if defined(GPU_TEST_UTILS)
        ++fStats.fGraphicsCacheMisses;
#endif

        if (compilationID) {
            // This is a cache miss so we know the next step is going to be a Pipeline
            // creation. Create the compilationID here so we can use it in the "CacheMissFor"
            // trace event.
            *compilationID = next_compilation_id();

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
            static const char* kNames[2] = { "CacheMissForN", "CacheMissForP" };
            TRACE_EVENT_INSTANT2("skia.gpu",
                                 TRACE_STR_STATIC(kNames[forPrecompile]),
                                 TRACE_EVENT_SCOPE_THREAD,
                                 "key", key.hash(),
                                 "compilationID", *compilationID);
#endif
        }

        return nullptr;
    }
}

#if SK_HISTOGRAMS_ENABLED
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
//
// LINT.IfChange(PipelineCreationRace)
enum class PipelineCreationRace {
    // The <First>Over<Second> enum names mean the first type of compilation won a compilation race
    // over the second type of compilation and ended up in the cache.
    kNormalOverNormal                 = 0, // can happen w/ multiple Recorders on different threads
    kNormalOverPrecompilation         = 1,
    kPrecompilationOverNormal         = 2,
    kPrecompilationOverPrecompilation = 3, // can happen with multiple threaded precompilation calls

    kMaxValue = kPrecompilationOverPrecompilation,
};
// LINT.ThenChange(//tools/metrics/histograms/enums.xml:SkiaPipelineCreationRace)

[[maybe_unused]] static constexpr int kPipelineCreationRaceCount =
        static_cast<int>(PipelineCreationRace::kMaxValue) + 1;
#endif // SK_HISTOGRAMS_ENABLED

sk_sp<GraphicsPipeline> GlobalCache::addGraphicsPipeline(const UniqueKey& key,
                                                         sk_sp<GraphicsPipeline> pipeline) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<GraphicsPipeline>* entry = fGraphicsPipelineCache.find(key);
    if (!entry) {
        // No equivalent pipeline was stored in the cache between a previous call to
        // findGraphicsPipeline() that returned null (triggering the pipeline creation) and this
        // later adding to the cache.
        entry = fGraphicsPipelineCache.insert(key, std::move(pipeline));

#if defined(GPU_TEST_UTILS)
        ++fStats.fGraphicsCacheAdditions;
#endif

        // Precompile Pipelines are only marked as used when they get a cache hit in
        // findGraphicsPipeline
        if (!(*entry)->fromPrecompile()) {
            (*entry)->updateAccessTime();
            (*entry)->markUsed();
        }

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
        static const char* kNames[2] = { "AddedN", "AddedP" };
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[(*entry)->fromPrecompile()]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             "compilationID", (*entry)->getPipelineInfo().fCompilationID);
#endif
    } else {
#if defined(GPU_TEST_UTILS)
        // else there was a race creating the same pipeline and this thread lost, so return
        // the winner
        ++fStats.fGraphicsRaces;
#endif

        [[maybe_unused]] int race = (*entry)->fromPrecompile() * 2 + pipeline->fromPrecompile();

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
        static const char* kNames[4] = {
                "NWonRaceOverN",
                "NWonRaceOverP",
                "PWonRaceOverN",
                "PWonRaceOverP"
        };
        TRACE_EVENT_INSTANT2("skia.gpu",
                             TRACE_STR_STATIC(kNames[race]),
                             TRACE_EVENT_SCOPE_THREAD,
                             "key", key.hash(),
                             // The losing compilation
                             "compilationID", pipeline->getPipelineInfo().fCompilationID);
#endif

#if SK_HISTOGRAMS_ENABLED
        SK_HISTOGRAM_ENUMERATION("Graphite.PipelineCreationRace",
                                 race,
                                 kPipelineCreationRaceCount);
#endif
    }
    return *entry;
}


void GlobalCache::purgePipelinesNotUsedSince(StdSteadyClock::time_point purgeTime) {
    SkAutoSpinlock lock{fSpinLock};

    skia_private::TArray<skgpu::UniqueKey> toRemove;

    // This is probably fine for now but is looping from most-recently-used to least-recently-used.
    // It seems like a reverse loop with an early out could be more efficient.
    fGraphicsPipelineCache.foreach([&toRemove, purgeTime](const UniqueKey* key,
                                                          const sk_sp<GraphicsPipeline>* pipeline) {
        if ((*pipeline)->lastAccessTime() < purgeTime) {
            toRemove.push_back(*key);
        }
    });

    for (const skgpu::UniqueKey& k : toRemove) {
#if defined(GPU_TEST_UTILS)
        ++fStats.fGraphicsPurges;
#endif
        fGraphicsPipelineCache.remove(k);
    }

    // TODO: add purging of Compute Pipelines (b/389073204)
}

#if defined(GPU_TEST_UTILS)
int GlobalCache::numGraphicsPipelines() const {
    SkAutoSpinlock lock{fSpinLock};

    return fGraphicsPipelineCache.count();
}

void GlobalCache::resetGraphicsPipelines() {
    SkAutoSpinlock lock{fSpinLock};

    fGraphicsPipelineCache.reset();
}

void GlobalCache::forEachGraphicsPipeline(
        const std::function<void(const UniqueKey&, const GraphicsPipeline*)>& fn) {
    SkAutoSpinlock lock{fSpinLock};

    fGraphicsPipelineCache.foreach([&](const UniqueKey* k, const sk_sp<GraphicsPipeline>* v) {
        fn(*k, v->get());
    });
}

GlobalCache::PipelineStats GlobalCache::getStats() const {
    SkAutoSpinlock lock{fSpinLock};

    return fStats;
}
#endif // defined(GPU_TEST_UTILS)

sk_sp<ComputePipeline> GlobalCache::findComputePipeline(const UniqueKey& key) {
    SkAutoSpinlock lock{fSpinLock};
    sk_sp<ComputePipeline>* entry = fComputePipelineCache.find(key);
    return entry ? *entry : nullptr;
}

sk_sp<ComputePipeline> GlobalCache::addComputePipeline(const UniqueKey& key,
                                                       sk_sp<ComputePipeline> pipeline) {
    SkAutoSpinlock lock{fSpinLock};
    sk_sp<ComputePipeline>* entry = fComputePipelineCache.find(key);
    if (!entry) {
        entry = fComputePipelineCache.insert(key, std::move(pipeline));
    }
    return *entry;
}

void GlobalCache::addStaticResource(sk_sp<Resource> resource) {
    SkAutoSpinlock lock{fSpinLock};
    fStaticResource.push_back(std::move(resource));
}

} // namespace skgpu::graphite
