/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/DrawTask.h"

#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

DrawTask::DrawTask(sk_sp<TextureProxy> target) : fTarget(std::move(target)) {}

DrawTask::~DrawTask() = default;

Task::Status DrawTask::prepareResources(ResourceProvider* resourceProvider,
                                        ScratchResourceManager* scratchManager,
                                        const RuntimeEffectDictionary* rteDict) {
    // TODO: Add pending listeners to decrement pending read counts of the target and return the
    // target to the scratch manager when it reaches 0.
    return fChildTasks.prepareResources(resourceProvider, scratchManager, rteDict);
}

Task::Status DrawTask::addCommands(Context* ctx,
                                   CommandBuffer* commandBuffer,
                                   ReplayTargetData replayTarget) {
    return fChildTasks.addCommands(ctx, commandBuffer, replayTarget);
}

} // namespace skgpu::graphite
