/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GpuWorkSubmission.h"

#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/QueueManager.h"

namespace skgpu::graphite {

GpuWorkSubmission::GpuWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                                     QueueManager* queueManager)
        : fCommandBuffer(std::move(cmdBuffer))
        , fQueueManager(queueManager) {}

GpuWorkSubmission::~GpuWorkSubmission() {
    fCommandBuffer->callFinishedProcs(/*success=*/true);
    fCommandBuffer->resetCommandBuffer();
    fQueueManager->returnCommandBuffer(std::move(fCommandBuffer));
}

} // namespace skgpu::graphite

