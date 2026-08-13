/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PipelineCreationTask_DEFINED
#define skgpu_graphite_PipelineCreationTask_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/RenderPassDesc.h"

#include <atomic>

namespace skgpu::graphite {

class RuntimeEffectDictionary;

// A PipelineCreationTask serves two purposes:
//   Initially it captures the need to compile a Pipeline. In this mode it will appear in
//      the PipelineManager's active tasks list and be wrapped in PipelineHandles.
//   Once executed, the Task will get a pointer to the Pipeline it helped compile and
//      hang around in the PipelineHandles (via a ref) to resolve the Handles to Pipelines)
// Note that, when the PipelineManager is threaded, the same task can appear in two work lists.
//   Once in the low priority list if it was initially kicked off via precompile
//   Once in the high priority list if the low priority task was found by a normal compilation
// Only one of the two 'compileTask' lambdas will be allowed to actually compile the pipeline -
// guarded by the 'fStarted' atomic.

// Once completed, the PipelineTask will lock the created Pipeline in the cache (via 'fPipeline')
// until the PipelineTask is deleted.
// Note that this is not a task in the sense of the Task class. It is a task in the sense that
// it is a unit of work possibly delegated to a thread. PipelineCreationTasks are also only
// known and handled by the PipelineManager vs being added to TaskLists (as Task-derived classes
// are).
class PipelineCreationTask : public SkRefCnt {
public:
    bool isLowPriority() const {
        return !fIsHighPriority.load(std::memory_order_relaxed);
    }

private:
    friend class PipelineManager; // for entire API and fPipeline
    friend class GraphicsPipelineHandle; // for fPipeline in pipelineOrNull()

    PipelineCreationTask(sk_sp<const RuntimeEffectDictionary> runtimeDict,
                         const UniqueKey& pipelineKey,
                         const GraphicsPipelineDesc& graphicsPipelineDesc,
                         const RenderPassDesc& renderPassDesc,
                         bool isHighPriority)
            : fRuntimeDict(std::move(runtimeDict))
            , fPipelineKey(pipelineKey)
            , fGraphicsPipelineDesc(graphicsPipelineDesc)
            , fRenderPassDesc(renderPassDesc)
            , fIsHighPriority(isHighPriority) {
    }

    const sk_sp<const RuntimeEffectDictionary> fRuntimeDict;
    const UniqueKey fPipelineKey;  // used to track this task in the PipelineManager
    const GraphicsPipelineDesc fGraphicsPipelineDesc;
    const RenderPassDesc fRenderPassDesc;

    // Once completed, this task will have filled in 'fPipeline' (if compilation succeeded).
    // This also serves to lock the pipeline in the cache.
    sk_sp<GraphicsPipeline> fPipeline;

    // This flag boils down to this task having been placed into a work list. It could be
    // in two at once.
    std::atomic<bool> fInWorkList{false};

    std::atomic<bool> fIsHighPriority{false};

    std::atomic<bool> fStarted{false};
    // Ideally, in C++-20, we would just wait on 'fCompleted' rather than using the
    // mutex/condition_variable pattern (in PipelineManager). This is atomic bc it is still used
    // outside the mutex in GraphicsPipelineHandle::pipelineOrNull.
    std::atomic<bool> fCompleted{false};
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PipelineCreationTask_DEFINED
