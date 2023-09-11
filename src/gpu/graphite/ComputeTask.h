/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputeTask_DEFINED
#define skgpu_graphite_ComputeTask_DEFINED

#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/Task.h"

#include <memory>

namespace skgpu::graphite {

class DispatchGroup;

/**
 * ComputeTask handles preparing and recording DispatchGroups into a series of compute dispatches
 * within a command buffer. It is guaranteed that dispatches within a DispatchGroup will be executed
 * sequentially.
 */
class ComputeTask final : public Task {
public:
    using DispatchGroupList = skia_private::TArray<std::unique_ptr<DispatchGroup>>;

    static sk_sp<ComputeTask> Make(DispatchGroupList dispatchGroups);

    ~ComputeTask() override;

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override;
    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    explicit ComputeTask(DispatchGroupList dispatchGroups);

    DispatchGroupList fDispatchGroups;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ComputeTask_DEFINED
