/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_DrawTask_DEFINED
#define skgpu_graphite_task_DrawTask_DEFINED

#include "src/gpu/graphite/task/Task.h"
#include "src/gpu/graphite/task/TaskList.h"

namespace skgpu::graphite {

class TextureProxy;

/**
 * DrawTask is a collection of subtasks that are executed in order to produce some intended
 * image in the DrawTask's target. As such, at least one of its subtasks will either be a
 * RenderPassTask, ComputeTask or CopyXToTextureTask that directly modify the target.
*/
class DrawTask final : public Task {
public:
    explicit DrawTask(sk_sp<TextureProxy> target);
    ~DrawTask() override;

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager*,
                            const RuntimeEffectDictionary*) override;

    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    friend class DrawContext; // for "addTask"

    // DrawTask is modified directly by DrawContext for efficiency, but its task list will be
    // fixed once DrawContext snaps the task.
    void addTask(sk_sp<Task> task) { fChildTasks.add(std::move(task)); }
    bool hasTasks() const { return fChildTasks.hasTasks(); }

    sk_sp<TextureProxy> fTarget;
    TaskList fChildTasks;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_task_DrawTask_DEFINED
