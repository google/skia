/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/task/TaskList.h"

namespace skgpu::graphite {

bool TaskList::prepareResources(ResourceProvider* resourceProvider,
                                 const RuntimeEffectDictionary* runtimeDict) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "# tasks", fTasks.size());
    for (const auto& task: fTasks) {
        if (!task->prepareResources(resourceProvider, runtimeDict)) {
            return false;
        }
    }

    return true;
}

bool TaskList::addCommands(Context* context,
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

} // namespace skgpu::graphite
