/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PipelineManager_DEFINED
#define skgpu_graphite_PipelineManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkEnumBitMask.h"
#include "src/core/SkSpinlock.h"
#include "src/core/SkTHash.h"

#include <condition_variable>
#include <mutex>

namespace skgpu {
class UniqueKey;
}

class SkExecutor;
class SkTaskGroup;

namespace skgpu::graphite {

class GraphicsPipeline;
class GraphicsPipelineDesc;
class GraphicsPipelineHandle;
enum class PipelineCreationFlags : uint8_t;
class PipelineCreationTask;
struct RenderPassDesc;
class RuntimeEffectDictionary;
class SharedContext;

class PipelineManager {
public:
    PipelineManager(SkExecutor* executor);
    ~PipelineManager();

    GraphicsPipelineHandle createHandle(
            SharedContext*,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>);

    void startPipelineCreationTask(SharedContext*,
                                   sk_sp<const RuntimeEffectDictionary>,
                                   const GraphicsPipelineHandle&);

    sk_sp<GraphicsPipeline> resolveHandle(const GraphicsPipelineHandle&);

    // Wait for any in-flight tasks to complete. Additionally, disable the addition of any
    // more threaded tasks.
    void shutDown();

#if defined(GPU_TEST_UTILS)
    void wait_TestOnly();

    struct Stats {
        // The number of times we find a pre-existing task for a Pipeline
        int fNumPreemptivelyFoundTasks = 0;
        int fNumTasksCreated = 0;
    };

    Stats getStats() const SK_EXCLUDES(fSpinLock);
#endif

private:
    mutable SkSpinlock fSpinLock;

    sk_sp<PipelineCreationTask> findOrCreateTask(
            const UniqueKey& pipelineKey,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>) SK_EXCLUDES(fSpinLock);

    void removeTask(PipelineCreationTask*) SK_EXCLUDES(fSpinLock);

    struct Traits {
        static const UniqueKey& GetKey(const sk_sp<PipelineCreationTask>&);
        static uint32_t Hash(const UniqueKey& pipelineKey);
    };
    using TaskMap = skia_private::THashTable<sk_sp<PipelineCreationTask>, UniqueKey, Traits>;

    TaskMap fActiveTasks SK_GUARDED_BY(fSpinLock);

#if defined(GPU_TEST_UTILS)
    Stats fStats SK_GUARDED_BY(fSpinLock);
#endif

    std::unique_ptr<SkTaskGroup> fTaskGroup SK_GUARDED_BY(fSpinLock);

    void signalCompleted(PipelineCreationTask*);
    void potentiallyWaitOn(PipelineCreationTask*);

    // We have the mutex and condition_variable here to limit the number of
    // mutexes/semaphores we need for synchronizing access to the pipelines.
    // The Context thread is the only place that resolves handles so we will only
    // ever be waiting on at most one pipeline at a time and no other thread will
    // need to block on waiting for a different pipeline. This means we don't need
    // to add a condition_variable to every PipelineCreationTask.
    std::mutex fMutex;
    std::condition_variable fConditionVariable; // SK_GUARDED_BY(fMutex)
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PipelineManager_DEFINED
