/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/SynchronizeToCpuTask.h"

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"

namespace skgpu::graphite {

sk_sp<SynchronizeToCpuTask> SynchronizeToCpuTask::Make(sk_sp<Buffer> buffer) {
    return sk_sp<SynchronizeToCpuTask>(new SynchronizeToCpuTask(std::move(buffer)));
}

SynchronizeToCpuTask::~SynchronizeToCpuTask() {}

bool SynchronizeToCpuTask::addCommands(Context*, CommandBuffer* commandBuffer, ReplayTargetData) {
    return commandBuffer->synchronizeBufferToCpu(std::move(fBuffer));
}

}  // namespace skgpu::graphite
