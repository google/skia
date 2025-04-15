/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_DrawTask_DEFINED
#define skgpu_graphite_task_DrawTask_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/ScratchResourceManager.h"
#include "src/gpu/graphite/task/Task.h"
#include "src/gpu/graphite/task/TaskList.h"

#include <utility>

namespace skgpu::graphite {

class TextureProxy;
class CommandBuffer;
class Context;
class ResourceProvider;
class RuntimeEffectDictionary;

/**
 * DrawTask is a collection of subtasks that are executed in order to produce some intended
 * image in the DrawTask's target. As such, at least one of its subtasks will either be a
 * RenderPassTask, ComputeTask or CopyXToTextureTask that directly modify the target.
*/
class DrawTask final : public Task, private ScratchResourceManager::PendingUseListener {
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

    void onUseCompleted(ScratchResourceManager*) override;

    sk_sp<TextureProxy> fTarget;
    TaskList fChildTasks;

    // Once there is one DrawTask for a scratch device, whether or not the target is instantaited
    // will be equivalent to whether or not prepareResources() has been called already if the task
    // is referenced multiple times in a Recording. Right now, however, a scratch device can still
    // produce several DrawTasks (in which case they will see an instantiated proxy so should still
    // prepare their own resources instead of discarding themselves).
    bool fPrepared = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_task_DrawTask_DEFINED
