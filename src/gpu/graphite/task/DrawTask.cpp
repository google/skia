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

bool DrawTask::prepareResources(ResourceProvider* resourceProvider,
                                const RuntimeEffectDictionary* rteDict) {
    return fChildTasks.prepareResources(resourceProvider, rteDict);
}

bool DrawTask::addCommands(Context* ctx,
                           CommandBuffer* commandBuffer,
                           ReplayTargetData replayTarget) {
    return fChildTasks.addCommands(ctx, commandBuffer, replayTarget);
}

} // namespace skgpu::graphite
