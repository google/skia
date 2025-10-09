/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/TaskList.h"

#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ScratchResourceManager.h"

#include <cstdint>

namespace skgpu::graphite {

using Status = Task::Status;

template <typename Fn>
Status TaskList::visitTasks(Fn fn) {
    int discardCount = 0;
    for (sk_sp<Task>& task: fTasks) {
        if (!task) {
            discardCount++;
            continue; // Skip over discarded tasks
        }

        Status status = fn(task.get());
        if (status == Status::kFail) {
            return Status::kFail;
        } else if (status == Status::kDiscard) {
            task.reset();
            discardCount++;
        }
    }

    return discardCount == fTasks.size() ? Status::kDiscard : Status::kSuccess;
}

Status TaskList::prepareResources(ResourceProvider* resourceProvider,
                                  ScratchResourceManager* scratchManager,
                                  sk_sp<const RuntimeEffectDictionary> runtimeDict) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "# tasks", fTasks.size());
    scratchManager->pushScope();
    Status status = this->visitTasks([&](Task* task) {
        return task->prepareResources(resourceProvider, scratchManager, runtimeDict);
    });
    scratchManager->popScope();
    return status;
}

Status TaskList::addCommands(Context* context,
                             CommandBuffer* commandBuffer,
                             Task::ReplayTargetData replayData) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "# tasks", fTasks.size());
    return this->visitTasks([&](Task* task) {
        return task->addCommands(context, commandBuffer, replayData);
    });
}

bool TaskList::visitPipelines(const std::function<bool(const GraphicsPipeline*)>& visitor) {
    Status status = this->visitTasks([&](Task* task) {
        return task->visitPipelines(visitor) ? Status::kSuccess : Status::kFail;
    });
    // Map back to simple bool (treat kDiscard as true too, no pipelines to visit means all
    // pipelines were visited).
    return status != Status::kFail;
}

bool TaskList::visitProxies(const std::function<bool(const TextureProxy*)>& visitor,
                            bool readsOnly) {
    Status status = this->visitTasks([&](Task* task) {
        return task->visitProxies(visitor, readsOnly) ? Status::kSuccess : Status::kFail;
    });
    // Map back to simple bool (treat kDiscard as true too, no pipelines to visit means all
    // pipelines were visited).
    return status != Status::kFail;
}

#if defined(SK_DUMP_TASKS)
void TaskList::visit(const std::function<void(const Task* task, bool isLast)>& visitor) const {
    // Find the last non-null task so we know when to draw the corner branch.
    const Task* lastNonNullTask = nullptr;
    for (int i = fTasks.size() - 1; i >= 0; --i) {
        if (fTasks[i]) {
            lastNonNullTask = fTasks[i].get();
            break;
        }
    }

    for (const sk_sp<Task>& task : fTasks) {
        if (task) {
            visitor(task.get(), task.get() == lastNonNullTask);
        }
    }
}
#endif

} // namespace skgpu::graphite
