/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/TaskGraph.h"

namespace skgpu::graphite {

TaskGraph::TaskGraph() {}
TaskGraph::~TaskGraph() {}

void TaskGraph::add(sk_sp<Task> task) {
    fTasks.emplace_back(std::move(task));
}

void TaskGraph::prepend(sk_sp<Task> task) {
    fTasks.emplace(fTasks.begin(), std::move(task));
}

bool TaskGraph::prepareResources(ResourceProvider* resourceProvider,
                                 const RuntimeEffectDictionary* runtimeDict) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "# tasks", fTasks.size());
    for (const auto& task: fTasks) {
        if (!task->prepareResources(resourceProvider, runtimeDict)) {
            return false;
        }
    }

    return true;
}

bool TaskGraph::addCommands(Context* context,
                            CommandBuffer* commandBuffer,
                            Task::ReplayTargetData replayData) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "# tasks", fTasks.size());

    for (const auto& task: fTasks) {
        if (!task->addCommands(context, commandBuffer, replayData)) {
            return false;
        }
    }

    return true;
}

void TaskGraph::reset() {
    fTasks.clear();
}

} // namespace skgpu::graphite
