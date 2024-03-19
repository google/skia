/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/ComputeTask.h"

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/compute/DispatchGroup.h"

namespace skgpu::graphite {

sk_sp<ComputeTask> ComputeTask::Make(DispatchGroupList dispatchGroups) {
    return sk_sp<ComputeTask>(new ComputeTask(std::move(dispatchGroups)));
}

ComputeTask::ComputeTask(DispatchGroupList dispatchGroups)
        : fDispatchGroups(std::move(dispatchGroups)), fChildTasks(fDispatchGroups.size()) {
    for (auto& group : fDispatchGroups) {
        fChildTasks.push_back(group->snapChildTask());
    }
}

ComputeTask::~ComputeTask() = default;

bool ComputeTask::prepareResources(ResourceProvider* provider, const RuntimeEffectDictionary* rtd) {
    for (const auto& child : fChildTasks) {
        if (child) {
            child->prepareResources(provider, rtd);
        }
    }
    for (const auto& group : fDispatchGroups) {
        if (!group->prepareResources(provider)) {
            return false;
        }
    }
    return true;
}

bool ComputeTask::addCommands(Context* ctx, CommandBuffer* commandBuffer, ReplayTargetData rtd) {
    if (fDispatchGroups.empty()) {
        return true;
    }
    SkASSERT(fDispatchGroups.size() == fChildTasks.size());
    const std::unique_ptr<DispatchGroup>* currentSpanPtr = &fDispatchGroups[0];
    size_t currentSpanSize = 0u;
    for (int i = 0; i < fDispatchGroups.size(); ++i) {
        // If the next DispatchGroup has a dependent task, then encode the accumulated span as a
        // compute pass now. CommandBuffer encodes each compute pass with a separate encoder, so
        // the dependent task can use a non-compute encoder if needed.
        Task* child = fChildTasks[i].get();
        if (child) {
            if (currentSpanSize > 0u) {
                if (!commandBuffer->addComputePass({currentSpanPtr, currentSpanSize})) {
                    return false;
                }
                currentSpanPtr = &fDispatchGroups[i];
                currentSpanSize = 0u;
            }
            if (!child->addCommands(ctx, commandBuffer, rtd)) {
                return false;
            }
        }
        currentSpanSize++;
    }
    return currentSpanSize == 0u ||
           commandBuffer->addComputePass({currentSpanPtr, currentSpanSize});
}

}  // namespace skgpu::graphite
