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
private:
    friend class PipelineManager; // for entire API and fPipeline

    PipelineCreationTask(const UniqueKey& pipelineKey,
                         const GraphicsPipelineDesc& graphicsPipelineDesc,
                         const RenderPassDesc& renderPassDesc,
                         SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags)
            : fPipelineKey(pipelineKey)
            , fGraphicsPipelineDesc(graphicsPipelineDesc)
            , fRenderPassDesc(renderPassDesc)
            , fPipelineCreationFlags(pipelineCreationFlags) {}

    const UniqueKey fPipelineKey;  // used to track this task in the PipelineManager
    const GraphicsPipelineDesc fGraphicsPipelineDesc;
    const RenderPassDesc fRenderPassDesc;
    const SkEnumBitMask<PipelineCreationFlags> fPipelineCreationFlags;

    // Once completed, this task will have filled in 'fPipeline' (if compilation succeeded).
    // This also serves to lock the pipeline in the cache.
    sk_sp<GraphicsPipeline> fPipeline;

    std::atomic<bool> fCompleted = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PipelineCreationTask_DEFINED
