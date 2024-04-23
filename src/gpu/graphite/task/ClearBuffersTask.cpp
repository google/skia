/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/ClearBuffersTask.h"

#include "src/gpu/graphite/CommandBuffer.h"

namespace skgpu::graphite {

sk_sp<ClearBuffersTask> ClearBuffersTask::Make(skia_private::TArray<ClearBufferInfo> clearList) {
    return sk_sp<ClearBuffersTask>(new ClearBuffersTask(std::move(clearList)));
}

ClearBuffersTask::~ClearBuffersTask(){};

Task::Status ClearBuffersTask::addCommands(Context*,
                                           CommandBuffer* commandBuffer,
                                           ReplayTargetData) {
    bool result = true;
    for (const auto& c : fClearList) {
        result &= commandBuffer->clearBuffer(c.fBuffer, c.fOffset, c.fSize);
    }
    return result ? Status::kSuccess : Status::kFail;
}

}  // namespace skgpu::graphite
