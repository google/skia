/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/TaskGraph.h"

namespace skgpu::graphite {

TaskGraph::TaskGraph() {}
TaskGraph::~TaskGraph() {}

void TaskGraph::add(sk_sp<Task> task) {
    fTasks.emplace_back(std::move(task));
}

bool TaskGraph::addCommands(ResourceProvider* resourceProvider, CommandBuffer* commandBuffer) {
    for (const auto& task: fTasks) {
        if (!task->addCommands(resourceProvider, commandBuffer)) {
            return false;
        }
    }

    return true;
}

void TaskGraph::reset() {
    fTasks.clear();
}

} // namespace skgpu::graphite
