/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_TaskList_DEFINED
#define skgpu_graphite_task_TaskList_DEFINED

#include "src/gpu/graphite/task/Task.h"

#include <vector>

namespace skgpu::graphite {

class CommandBuffer;
class Context;
class ResourceProvider;

class TaskList {
public:
    TaskList() = default;

    void add(sk_sp<Task> task)     { fTasks.emplace_back(std::move(task)); }
    void prepend(sk_sp<Task> task) { fTasks.emplace(fTasks.begin(), std::move(task)); }
    void reset() { fTasks.clear(); }

    bool hasTasks() const { return !fTasks.empty(); }

    // Returns true on success; false on failure
    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*);
    bool addCommands(Context*, CommandBuffer*, Task::ReplayTargetData);

private:
    // TODO(b/238767759): Switch to TArray, but that doesn't support inserting at the front.
    // Prepending tasks can be avoided if we restructure how tasks are moved from Recorder to
    // Recording.
    std::vector<sk_sp<Task>> fTasks;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_task_TaskList_DEFINED
