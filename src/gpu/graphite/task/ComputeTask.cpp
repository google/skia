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

Task::Status ComputeTask::prepareResources(ResourceProvider* provider,
                                           ScratchResourceManager* scratchManager,
                                           const RuntimeEffectDictionary* rtd) {
    for (auto& child : fChildTasks) {
        if (child) {
            Status status = child->prepareResources(provider, scratchManager, rtd);
            if (status == Status::kFail) {
                return Status::kFail;
            } else if (status == Status::kDiscard) {
                child.reset();
            }
        }
    }
    for (const auto& group : fDispatchGroups) {
        // TODO: Allow ComputeTasks to instantiate with scratch textures and return them.
        if (!group->prepareResources(provider)) {
            return Status::kFail;
        }
    }
    return Status::kSuccess;
}

Task::Status ComputeTask::addCommands(Context* ctx,
                                      CommandBuffer* commandBuffer,
                                      ReplayTargetData rtd) {
    if (fDispatchGroups.empty()) {
        return Status::kDiscard;
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
                    return Status::kFail;
                }
                currentSpanPtr = &fDispatchGroups[i];
                currentSpanSize = 0u;
            }

            Status status = child->addCommands(ctx, commandBuffer, rtd);
            if (status == Status::kFail) {
                return Status::kFail;
            } else if (status == Status::kDiscard) {
                fChildTasks[i].reset();
            }
        }
        currentSpanSize++;
    }
    return (currentSpanSize == 0u ||
            commandBuffer->addComputePass({currentSpanPtr, currentSpanSize})) ? Status::kSuccess
                                                                              : Status::kFail;
}

}  // namespace skgpu::graphite
