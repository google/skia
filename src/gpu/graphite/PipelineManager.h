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

namespace skgpu {
class UniqueKey;
}

class SkExecutor;

namespace skgpu::graphite {

class GraphicsPipeline;
class GraphicsPipelineDesc;
class GraphicsPipelineHandle;
enum class PipelineCreationFlags : uint8_t;
class PipelineCreationTask;
class RuntimeEffectDictionary;
class SharedContext;
struct RenderPassDesc;

class PipelineManager {
public:
    PipelineManager(SkExecutor* executor);
    ~PipelineManager();

    // If an existing Pipeline is found, it is just wrapped in a Handle and returned.
    // Otherwise, a compilation task is created and queued up for execution.
    // If no Executor is provided the compilations will occur synchronously, in-line.
    GraphicsPipelineHandle createHandle(
            SharedContext*,
            sk_sp<const RuntimeEffectDictionary> runtimeDict,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            SkEnumBitMask<PipelineCreationFlags>);

    sk_sp<GraphicsPipeline> resolveHandle(const GraphicsPipelineHandle&);

    // Wait for any in-flight tasks to complete. Additionally, disable the addition of any
    // more threaded tasks.
    void shutDown() SK_EXCLUDES(fSpinLock);

#if defined(GPU_TEST_UTILS)
    void wait_TestOnly() SK_EXCLUDES(fSpinLock);

    struct Stats {
        // The number of times we find a pre-existing task for a Pipeline
        int fNumPreemptivelyFoundTasks = 0;
        int fNumTasksCreated = 0;
    };

    Stats getStats() const SK_EXCLUDES(fSpinLock);
#endif

private:
    enum class Priority { kHigh = 0, kLow = 1 };

    sk_sp<PipelineCreationTask> findOrCreateTask(
            SharedContext*,
            sk_sp<const RuntimeEffectDictionary>,
            const UniqueKey& pipelineKey,
            const GraphicsPipelineDesc&,
            const RenderPassDesc&,
            Priority) SK_EXCLUDES(fSpinLock);

    void addTaskToWorkList(SharedContext*,
                           sk_sp<PipelineCreationTask>,
                           Priority) SK_EXCLUDES(fSpinLock);

    void removeTask(const sk_sp<PipelineCreationTask>&) SK_EXCLUDES(fSpinLock);

    void potentiallyWaitOn(const sk_sp<PipelineCreationTask>&) SK_EXCLUDES(fSpinLock);

    sk_sp<PipelineCreationTask> getWork(bool inclInProgress) SK_EXCLUDES(fSpinLock);
    void wait() SK_EXCLUDES(fSpinLock);

    // Returns true if compilation occurred; false otherwise.
    // All callers must hold a ref on the PipelineCreationTask.
    static bool InlineCompile(const sk_sp<PipelineCreationTask>&) SK_EXCLUDES(fSpinLock);

    struct Traits {
        static const UniqueKey& GetKey(const sk_sp<PipelineCreationTask>&);
        static uint32_t Hash(const UniqueKey& pipelineKey);
    };
    using TaskMap = skia_private::THashTable<sk_sp<PipelineCreationTask>, UniqueKey, Traits>;

    mutable SkSpinlock fSpinLock;

    TaskMap fActiveTasks SK_GUARDED_BY(fSpinLock);

#if defined(GPU_TEST_UTILS)
    Stats fStats SK_GUARDED_BY(fSpinLock);
#endif

    // The executor is obtained from ContextOptions. It is up to the client to ensure it
    // exists past the Context's destruction. The Context does call PipelineManager::shutDown
    // from its destructor to end its use.
    SkExecutor* fExecutor SK_GUARDED_BY(fSpinLock) = nullptr;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PipelineManager_DEFINED
