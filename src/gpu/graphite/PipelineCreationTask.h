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

namespace skgpu::graphite {

// Once completed, the PipelineTask will lock the created Pipeline in the cache (via 'fPipeline')
// until the PipelineTask is deleted.
// Note that this is not a task in the sense of the Task class. It is a task in the sense that
// it is a unit of work possibly delegated to a thread. PipelineCreationTasks are also only
// known and handled by the PipelineManager vs being added to TaskLists (as Task-derived classes
// are).
class PipelineCreationTask : public SkRefCnt {
#if defined(GPU_TEST_UTILS)
public:
    int32_t id() const { return fID; }
#endif

private:
    friend class PipelineManager; // for entire API and fPipeline
    friend class GraphicsPipelineHandle; // for fPipeline in pipelineOrNull()

    PipelineCreationTask(const UniqueKey& pipelineKey,
                         const GraphicsPipelineDesc& graphicsPipelineDesc,
                         const RenderPassDesc& renderPassDesc,
                         SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags)
            : fPipelineKey(pipelineKey)
            , fGraphicsPipelineDesc(graphicsPipelineDesc)
            , fRenderPassDesc(renderPassDesc)
            , fPipelineCreationFlags(pipelineCreationFlags) {
#if defined(GPU_TEST_UTILS)
        static std::atomic<int32_t> sID{0};
        fID = sID++;
#endif
    }

    bool forPrecompile() const {
        return SkToBool(fPipelineCreationFlags & PipelineCreationFlags::kForPrecompilation);
    }

    const UniqueKey fPipelineKey;  // used to track this task in the PipelineManager
    const GraphicsPipelineDesc fGraphicsPipelineDesc;
    const RenderPassDesc fRenderPassDesc;
    const SkEnumBitMask<PipelineCreationFlags> fPipelineCreationFlags;

    // Once completed, this task will have filled in 'fPipeline' (if compilation succeeded).
    // This also serves to lock the pipeline in the cache.
    sk_sp<GraphicsPipeline> fPipeline;

    // This flag boils down to this task having been placed into a work list.
    std::atomic<bool> fInProgress{false};
    // Ideally, in C++-20, we would just wait on 'fCompleted' rather than using the
    // mutex/condition_variable pattern (in PipelineManager). This is atomic bc it is still used
    // outside the mutex in GraphicsPipelineHandle::pipelineOrNull.
    std::atomic<bool> fCompleted{false};

#if defined(GPU_TEST_UTILS)
    int32_t fID;
#endif
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PipelineCreationTask_DEFINED
