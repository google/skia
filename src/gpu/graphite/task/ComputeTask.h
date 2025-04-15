/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_task_ComputeTask_DEFINED
#define skgpu_graphite_task_ComputeTask_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"
#include "src/gpu/graphite/task/Task.h"

#include <memory>

namespace skgpu::graphite {

class CommandBuffer;
class Context;
class ResourceProvider;
class RuntimeEffectDictionary;
class ScratchResourceManager;

/**
 * ComputeTask handles preparing and recording DispatchGroups into a series of compute dispatches
 * within a command buffer. It is guaranteed that dispatches within a DispatchGroup will be executed
 * sequentially.
 */
class ComputeTask final : public Task {
public:
    using DispatchGroupList = skia_private::STArray<1, std::unique_ptr<DispatchGroup>>;

    static sk_sp<ComputeTask> Make(DispatchGroupList dispatchGroups);

    ~ComputeTask() override;

    Status prepareResources(ResourceProvider*,
                            ScratchResourceManager*,
                            const RuntimeEffectDictionary*) override;
    Status addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    explicit ComputeTask(DispatchGroupList dispatchGroups);

    DispatchGroupList fDispatchGroups;

    // Every element of this list is a task that must execute before the DispatchGroup stored at the
    // same array index. Child tasks are allowed to be a nullptr to represent NOP (i.e. the
    // corresponding DispatchGroup doesn't have any pre-tasks).
    skia_private::TArray<sk_sp<Task>> fChildTasks;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_task_ComputeTask_DEFINED
