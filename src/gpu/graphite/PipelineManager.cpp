/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PipelineManager.h"

#include "src/core/SkTaskGroup.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphicsPipelineHandle.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PipelineCreationTask.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/SharedContext.h"

namespace skgpu::graphite {

GraphicsPipelineHandle::GraphicsPipelineHandle(sk_sp<PipelineCreationTask> task)
    : fTaskOrPipeline(std::move(task)) {}

GraphicsPipelineHandle::GraphicsPipelineHandle(sk_sp<GraphicsPipeline> pipeline)
    : fTaskOrPipeline(std::move(pipeline)) {}

PipelineManager::PipelineManager() {}
PipelineManager::~PipelineManager() {}

const UniqueKey& PipelineManager::Traits::GetKey(const sk_sp<PipelineCreationTask>& task) {
    return task->fPipelineKey;
}

uint32_t PipelineManager::Traits::Hash(const UniqueKey& pipelineKey) {
    return pipelineKey.hash();
}

GraphicsPipelineHandle PipelineManager::createHandle(
        SharedContext* sharedContext,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    GlobalCache* globalCache = sharedContext->globalCache();
    const Caps* caps = sharedContext->caps();

    UniqueKey pipelineKey = caps->makeGraphicsPipelineKey(pipelineDesc, renderPassDesc);

    if (sk_sp<PipelineCreationTask> task = this->findTask(pipelineKey)) {
        // There is a task in progress to compile this Pipeline so it can't be ready yet (i.e.,
        // it isn't in the Pipeline Cache).
        return GraphicsPipelineHandle(std::move(task));
    }

    sk_sp<GraphicsPipeline> pipeline = globalCache->findGraphicsPipeline(
            pipelineKey,
            pipelineCreationFlags);
    if (pipeline) {
        return GraphicsPipelineHandle(std::move(pipeline));
    }

    sk_sp<PipelineCreationTask> task = this->findOrCreateTask(pipelineKey,
                                                              pipelineDesc,
                                                              renderPassDesc,
                                                              pipelineCreationFlags);
    return GraphicsPipelineHandle(std::move(task));
}

void PipelineManager::startPipelineCreationTask(SharedContext* sharedContext,
                                                sk_sp<const RuntimeEffectDictionary> runtimeDict,
                                                const GraphicsPipelineHandle& handle) {
    if (std::holds_alternative<sk_sp<GraphicsPipeline>>(handle.fTaskOrPipeline)) {
        return;
    }

    sk_sp<PipelineCreationTask> task =
            std::get<sk_sp<PipelineCreationTask>>(handle.fTaskOrPipeline);

    sk_sp<GraphicsPipeline> pipeline = sharedContext->findOrCreateGraphicsPipeline(
            runtimeDict.get(),
            task->fPipelineKey,
            task->fGraphicsPipelineDesc,
            task->fRenderPassDesc,
            task->fPipelineCreationFlags);

    if (!pipeline) {
        SKGPU_LOG_W("Failed to create GraphicsPipeline!");
    }

    if (!task->fCompleted.exchange(true)) {
        task->fPipeline = pipeline;
        this->removeTask(task.get());
    }
}

sk_sp<GraphicsPipeline> PipelineManager::resolveHandle(const GraphicsPipelineHandle& handle) {
    if (std::holds_alternative<sk_sp<GraphicsPipeline>>(handle.fTaskOrPipeline)) {
        return std::get<sk_sp<GraphicsPipeline>>(handle.fTaskOrPipeline);
    }

    // Since 'fTaskOrPipeline' doesn't hold a pipeline the pipeline must not have existed when
    // the handle was created so a compilation task must've been created to compile it
    sk_sp<PipelineCreationTask> task =
            std::get<sk_sp<PipelineCreationTask>>(handle.fTaskOrPipeline);

    // For the non-threaded version of the PipelineManager, whenever a thread gets here it
    // will already have blindly executed the task (in DrawPass::prepareResources).
    SkASSERT(task->fCompleted);
    return task->fPipeline;
}

#if defined(GPU_TEST_UTILS)
PipelineManager::Stats PipelineManager::getStats() const {
    SkAutoSpinlock lock{fSpinLock};

    return fStats;
}
#endif

sk_sp<PipelineCreationTask> PipelineManager::findTask(const UniqueKey& pipelineKey) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<PipelineCreationTask> task = fActiveTasks.findOrNull(pipelineKey);

#if defined(GPU_TEST_UTILS)
    if (task) {
        fStats.fNumPreemptivelyFoundTasks++;
    }
#endif

    return task;
}

sk_sp<PipelineCreationTask> PipelineManager::findOrCreateTask(
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<PipelineCreationTask>* task = fActiveTasks.find(pipelineKey);
    if (task) {
        // There is a race in createHandle from when we first check for a task; then, failing that,
        // check for an existing pipeline; then, failing that, try to create a new task. Thus,
        // we can sometimes find our task here.
#if defined(GPU_TEST_UTILS)
        fStats.fNumTaskCreationRaces++;
#endif
        return *task;
    }

#if defined(GPU_TEST_UTILS)
    fStats.fNumTasksCreated++;
#endif

    sk_sp<PipelineCreationTask> newTask = sk_sp<PipelineCreationTask>(
            new PipelineCreationTask(pipelineKey,
                                     pipelineDesc,
                                     renderPassDesc,
                                     pipelineCreationFlags));
    fActiveTasks.set(newTask);
    return newTask;
}

void PipelineManager::removeTask(PipelineCreationTask* task) {
    SkAutoSpinlock lock{fSpinLock};

    // TODO(robertphillips): this guard is only necessary in the non-threaded version of
    // the PipelineManager
    if (fActiveTasks.findOrNull(task->fPipelineKey)) {
        fActiveTasks.remove(task->fPipelineKey);
    }
}

} // namespace skgpu::graphite
