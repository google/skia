/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/TaskGraph.h"

namespace skgpu {

TaskGraph::TaskGraph() {}
TaskGraph::~TaskGraph() {}

void TaskGraph::add(sk_sp<Task> task) {
    fTasks.emplace_back(std::move(task));
}

void TaskGraph::addCommands(ResourceProvider* resourceProvider, CommandBuffer* commandBuffer) {
    for (const auto& task: fTasks) {
        task->addCommands(resourceProvider, commandBuffer);
    }
}

void TaskGraph::reset() {
    fTasks.clear();
}

} // namespace skgpu
