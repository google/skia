/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/GpuWorkSubmission.h"

#include "experimental/graphite/src/CommandBuffer.h"

namespace skgpu::graphite {

GpuWorkSubmission::GpuWorkSubmission(sk_sp<CommandBuffer> cmdBuffer)
        : fCommandBuffer(std::move(cmdBuffer)) {}

GpuWorkSubmission::~GpuWorkSubmission() {
    fCommandBuffer->callFinishedProcs(/*success=*/true);
}

} // namespace skgpu::graphite

