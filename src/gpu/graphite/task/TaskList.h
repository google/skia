/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_TaskList_DEFINED
#define skgpu_graphite_task_TaskList_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/task/Task.h"

#include <functional>
#include <utility>

namespace skgpu::graphite {

class CommandBuffer;
class Context;
class GraphicsPipeline;
class ResourceProvider;
class RuntimeEffectDictionary;
class ScratchResourceManager;
class TextureProxy;

class TaskList {
public:
    TaskList() = default;

    void add(TaskList&& tasks) { fTasks.move_back(tasks.fTasks); }
    void add(sk_sp<Task> task) { fTasks.emplace_back(std::move(task)); }
    void reset() { fTasks.clear(); }

    int size() const { return fTasks.size(); }
    bool hasTasks() const { return !fTasks.empty(); }

    // Returns kSuccess if no child task failed and at least one child didn't return kDiscard.
    // Returns kDiscard if all children were discarded.
    // Returns kFail if any child failed.
    // Automatically removes tasks from its list if they return kDiscard.
    Task::Status prepareResources(ResourceProvider*,
                                  ScratchResourceManager*,
                                  sk_sp<const RuntimeEffectDictionary>);
    Task::Status addCommands(Context*, CommandBuffer*, Task::ReplayTargetData);

    bool visitPipelines(const std::function<bool(const GraphicsPipeline*)>& visitor);

    bool visitProxies(const std::function<bool(const TextureProxy*)>& visitor, bool readsOnly);

    SK_DUMP_TASKS_CODE(
            void visit(const std::function<void(const Task* task, bool isLast)>& visitor) const;)
private:
    template <typename Fn> // (Task*)->Status
    Task::Status visitTasks(Fn);

    skia_private::TArray<sk_sp<Task>> fTasks;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_task_TaskList_DEFINED
