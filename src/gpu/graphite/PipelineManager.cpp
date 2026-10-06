/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PipelineManager.h"

#include "include/core/SkExecutor.h"
#include "include/private/SkLog.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphicsPipelineHandle.h"
#include "src/gpu/graphite/PipelineCreationTask.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/SharedContext.h"

// The threaded PipelineManager is set up to work with a executor with two worklists.
// All work from the first work list must be completed before work from the second list
// is begun. In-line compiles will be put in the high priority list while precompiles
// will be put in the low priority work list. If a single work list executor is provided
// everything will collapse to just being interleaved on that single work list.
// Note that the number of worklists is orthogonal to the number of threads.
static constexpr int kHighPriorityWorkList = 0;
static constexpr int kLowPriorityWorkList = 1;

namespace skgpu::graphite {

// Directly accessing the two variants is thread safe because a given GraphicsPipelineHandle
// does not switch between a task or pipeline when the task completes. The 'fCompleted' check
// is atomic and, if the task is completed, the 'fPipeline' access is thread safe. There is,
// obviously, an inherent race wrt the 'fCompleted' access. Callers must either ensure that
// the Handle has already been resolved or accept some raciness in the response.
sk_sp<GraphicsPipeline> GraphicsPipelineHandle::pipelineOrNull() const {
    if (std::holds_alternative<sk_sp<GraphicsPipeline>>(fTaskOrPipeline)) {
        return std::get<sk_sp<GraphicsPipeline>>(fTaskOrPipeline);
    }
    const sk_sp<PipelineCreationTask>& task =
            std::get<sk_sp<PipelineCreationTask>>(fTaskOrPipeline);
    if (!task->fCompleted.load(std::memory_order_acquire)) {
        return nullptr;
    }
    return task->fPipeline;
}

GraphicsPipelineHandle::GraphicsPipelineHandle(sk_sp<PipelineCreationTask> task)
    : fTaskOrPipeline(std::move(task)) {}

GraphicsPipelineHandle::GraphicsPipelineHandle(sk_sp<GraphicsPipeline> pipeline)
    : fTaskOrPipeline(std::move(pipeline)) {}

PipelineManager::PipelineManager(SkExecutor* executor) : fExecutor(executor) {}

PipelineManager::~PipelineManager() {
    // The executor should've been detached in Context's destructor (via shutDown()).
    {
        SkDEBUGCODE(SkAutoSpinlock lock{fSpinLock};)
        SkASSERT(!fExecutor);
        SkASSERT(fActiveTasks.count() == 0);
    }
}

const UniqueKey& PipelineManager::Traits::GetKey(const sk_sp<PipelineCreationTask>& task) {
    return task->fPipelineKey;
}

uint32_t PipelineManager::Traits::Hash(const UniqueKey& pipelineKey) {
    return pipelineKey.hash();
}

GraphicsPipelineHandle PipelineManager::createHandle(
        SharedContext* sharedContext,
        sk_sp<const RuntimeEffectDictionary> runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags) {
    GlobalCache* globalCache = sharedContext->globalCache();
    const Caps* caps = sharedContext->caps();

    bool forPrecompile =
        SkToBool(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation);
    Priority curPriority = forPrecompile ? Priority::kLow : Priority::kHigh;

    UniqueKey pipelineKey = caps->makeGraphicsPipelineKey(pipelineDesc, renderPassDesc);

    sk_sp<GraphicsPipeline> pipeline = globalCache->findGraphicsPipeline(
            pipelineKey,
            pipelineCreationFlags);
    if (pipeline) {
        return GraphicsPipelineHandle(std::move(pipeline));
    }

    // Although 'findGraphicsPipeline' didn't find a GraphicsPipeline, there could be a race.
    // In 'findOrCreateTask' we will, thread-safely, check if there is a task in flight to
    // create the Pipeline and, failing that, create one. If the race had occurred and
    // there is actually a matching GraphicsPipeline in the GlobalCache then it will be found
    // in 'compileTask'.
    sk_sp<PipelineCreationTask> task = this->findOrCreateTask(sharedContext,
                                                              std::move(runtimeDict),
                                                              pipelineKey,
                                                              pipelineDesc,
                                                              renderPassDesc,
                                                              curPriority);

    bool shouldAddToWorkList = false;

    if (curPriority == Priority::kHigh && task->isLowPriority()) {
        // If we found an active task for the current Pipeline we know, modulo thread races,
        // that it hasn't completed yet (since it, then, wouldn't be in the active task list).
        // If it was initially added as low priority, but turned out to be high priority,
        // re-add it as a high priority task.
        shouldAddToWorkList = !task->fIsHighPriority.exchange(true);
    } else {
        // Tasks are only removed from the TaskList when they are complete. This means that
        // an in-flight task can be found and resubmitted for compilation. The 'fInWorkList'
        // guard ensures we don't resubmit the same task over and over (modulo the one-off
        // switch from low to high priority above).
        shouldAddToWorkList = !task->fInWorkList.exchange(true);
    }

    if (shouldAddToWorkList) {
        this->addTaskToWorkList(sharedContext, task, curPriority);
    }

    return GraphicsPipelineHandle(std::move(task));
}

bool PipelineManager::InlineCompile(const sk_sp<PipelineCreationTask>& task) {
    SkASSERT(task);

    // Since there might be threaded contention to execute the compilation for the same
    // task (e.g., if a low priority compile got duplicated as a high priority compile
    // or an immediate compile was required), we check the 'fStarted' atomic so only
    // one does the work.
    if (task->fStarted.exchange(true)) {
        // If we got here it means some other thread beat us to it so don't compile
        // the pipeline.
        return false;
    }

    SkASSERT(task->fSharedContext);

    SharedContext* sharedContext = task->fSharedContext;
    PipelineManager* pipelineManager = sharedContext->pipelineManager();

    // This is a bit racy but the exact correctness of the actual cause for the compilation
    // isn't crucial. In essence, this tries to give the SharedContext a best guess about
    // the driver behind the compilation. The exact race is if a precompile compilation
    // was usurped by a normal compilation but we still report a precompile compilation
    // to the SharedContext.
    PipelineCreationFlags flags = task->isLowPriority()
                                      ? PipelineCreationFlags::kForPrecompilation
                                      : PipelineCreationFlags::kNone;

    task->fPipeline = sharedContext->findOrCreateGraphicsPipeline(
            task->fRuntimeDict.get(),
            task->fPipelineKey,
            task->fGraphicsPipelineDesc,
            task->fRenderPassDesc,
            flags);

    if (!task->fPipeline) {
        SKIA_LOG_W("Failed to create GraphicsPipeline!");
    }

    task->fSharedContext = nullptr;

    task->fCompleted.store(true, std::memory_order_release);
    // The Context thread is the only place that resolves handles so we will only
    // ever be waiting on a task on a single thread.
    task->fCompleted.notify_one();

    pipelineManager->removeTask(task);
    return true;
}

void PipelineManager::addTaskToWorkList(SharedContext* sharedContext,
                                        sk_sp<PipelineCreationTask> task,
                                        Priority priority) {
    {
        SkAutoSpinlock lock{fSpinLock};

        if (fExecutor) {
            int workList = priority == Priority::kLow ? kLowPriorityWorkList
                                                      : kHighPriorityWorkList;

            fExecutor->add([task]{ InlineCompile(task); } , workList);
            return;
        }
    }

    // Non-SkExecutor fallback. Note that, if multiple Recorders are recording in parallel on
    // multiple threads (w/ no SkExecutor supplied) there could still be a compilation race
    // here. In that case all the thread-safety mechanisms (e.g., 'fStarted', 'fCompleted')
    // will kick in to eliminate duplicate work. This does mean, as in the SkExecutor case,
    // that the task's Pipeline need not be resolved at the end of 'compileTask'. That is,
    // after all, the purview of 'resolveHandle'.
    InlineCompile(task);
}

sk_sp<GraphicsPipeline> PipelineManager::resolveHandle(const GraphicsPipelineHandle& handle) {
    if (std::holds_alternative<sk_sp<GraphicsPipeline>>(handle.fTaskOrPipeline)) {
        return std::get<sk_sp<GraphicsPipeline>>(handle.fTaskOrPipeline);
    }

    // Since 'fTaskOrPipeline' doesn't hold a pipeline the pipeline must not have existed when
    // the handle was created so a compilation task must've been created to compile it
    const sk_sp<PipelineCreationTask>& task =
            std::get<sk_sp<PipelineCreationTask>>(handle.fTaskOrPipeline);

    // For the non-threaded PipelineManager, the GraphicsPipeline will have been compiled in-line
    // so will already have been completed. For the threaded PipelineManager, resolveHandle
    // should only ever be called from the main thread (on which Context::insertRecording is
    // called) so only one thread should ever be waiting.
    this->potentiallyWaitOn(task);
    return task->fPipeline;
}

sk_sp<PipelineCreationTask> PipelineManager::getWork(bool inclInProgress) {
    SkAutoSpinlock lock{fSpinLock};

    if (fActiveTasks.count() == 0) {
        return nullptr;
    }

    using TaskMapIter = typename TaskMap::template Iter<sk_sp<PipelineCreationTask>>;

    TaskMapIter end = TaskMapIter::MakeEnd(&fActiveTasks);
    for (TaskMapIter it = TaskMapIter::MakeBegin(&fActiveTasks); it != end; ++it) {
        if (inclInProgress || !(*it)->fStarted) {
            return *it;
        }
    }

    return nullptr;
}

void PipelineManager::wait() {
    // First, try to steal any work that hasn't yet been started and run it locally.
    while (sk_sp<PipelineCreationTask> task = this->getWork(/* inclInProgress= */ false)) {
        InlineCompile(task);
    }
    // Then wait on any work that is in progress but hasn't been completed.
    while (sk_sp<PipelineCreationTask> task = this->getWork(/* inclInProgress= */ true)) {
        this->potentiallyWaitOn(task);
    }
}

void PipelineManager::shutDown() {
    // We null out 'fExecutor' so no more threaded work can be added after this point.
    {
        SkAutoSpinlock lock{fSpinLock};
        fExecutor = nullptr;
    }
    // We have to wait for the remaining tasks to complete bc they rely on the existence
    // of the SharedContext and the PipelineManager (this).
    // TODO(robertphillips) We could discard any unstarted tasks but would need a way to
    // have them still remove themselves from the task list.
    this->wait();

    {
        SkDEBUGCODE(SkAutoSpinlock lock{fSpinLock};)
        SkASSERT(!fExecutor);
        SkASSERT(fActiveTasks.count() == 0);
    }
}

#if defined(GPU_TEST_UTILS)
void PipelineManager::wait_TestOnly() {
    // This isn't thread safe (since some other thread could be adding more work in parallel) but,
    // hopefully, the unit tests know what they're doing (i.e., don't be doing other threaded
    // work when using this method).
    this->wait();
}

PipelineManager::Stats PipelineManager::getStats() const {
    SkAutoSpinlock lock{fSpinLock};

    return fStats;
}
#endif

sk_sp<PipelineCreationTask> PipelineManager::findOrCreateTask(
        SharedContext* sharedContext,
        sk_sp<const RuntimeEffectDictionary> runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        Priority priority) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<PipelineCreationTask>* task = fActiveTasks.find(pipelineKey);
    if (task) {
#if defined(GPU_TEST_UTILS)
        fStats.fNumPreemptivelyFoundTasks++;
#endif
        return *task;
    }

#if defined(GPU_TEST_UTILS)
    fStats.fNumTasksCreated++;
#endif

    sk_sp<PipelineCreationTask> newTask = sk_sp<PipelineCreationTask>(
            new PipelineCreationTask(sharedContext,
                                     std::move(runtimeDict),
                                     pipelineKey,
                                     pipelineDesc,
                                     renderPassDesc,
                                     priority == Priority::kHigh));
    fActiveTasks.set(newTask);
    return newTask;
}

void PipelineManager::removeTask(const sk_sp<PipelineCreationTask>& task) {
    SkAutoSpinlock lock{fSpinLock};

    fActiveTasks.remove(task->fPipelineKey);
}

void PipelineManager::potentiallyWaitOn(const sk_sp<PipelineCreationTask>& task) {
    // If we can preempt some thread that is scheduled to compile this Pipeline, do so rather
    // than waiting.
    if (InlineCompile(task)) {
        return;
    }

    task->fCompleted.wait(false, std::memory_order_acquire);
}

} // namespace skgpu::graphite
