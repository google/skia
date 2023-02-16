/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ComputeTask.h"

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"

namespace skgpu::graphite {

sk_sp<ComputeTask> ComputeTask::Make(DispatchGroupList dispatchGroups) {
    return sk_sp<ComputeTask>(new ComputeTask(std::move(dispatchGroups)));
}

ComputeTask::ComputeTask(DispatchGroupList dispatchGroups)
        : fDispatchGroups(std::move(dispatchGroups)) {}

ComputeTask::~ComputeTask() = default;

bool ComputeTask::prepareResources(ResourceProvider* provider, const RuntimeEffectDictionary*) {
    for (const auto& group : fDispatchGroups) {
        if (!group->prepareResources(provider)) {
            return false;
        }
    }
    return true;
}

bool ComputeTask::addCommands(Context*, CommandBuffer* commandBuffer, ReplayTargetData) {
    return commandBuffer->addComputePass(fDispatchGroups);
}

}  // namespace skgpu::graphite
