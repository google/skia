/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/task/TaskList.h"

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
                                  const RuntimeEffectDictionary* runtimeDict) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "# tasks", fTasks.size());
    return this->visitTasks([&](Task* task) {
        return task->prepareResources(resourceProvider, runtimeDict);
    });
}

Status TaskList::addCommands(Context* context,
                             CommandBuffer* commandBuffer,
                             Task::ReplayTargetData replayData) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "# tasks", fTasks.size());
    return this->visitTasks([&](Task* task) {
        return task->addCommands(context, commandBuffer, replayData);
    });
}

} // namespace skgpu::graphite
