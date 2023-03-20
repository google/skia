/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ClearBuffersTask.h"

#include "src/gpu/graphite/CommandBuffer.h"

namespace skgpu::graphite {

sk_sp<ClearBuffersTask> ClearBuffersTask::Make(std::vector<ClearBufferInfo> clearList) {
    return sk_sp<ClearBuffersTask>(new ClearBuffersTask(std::move(clearList)));
}

ClearBuffersTask::~ClearBuffersTask(){};

bool ClearBuffersTask::addCommands(Context*, CommandBuffer* commandBuffer, ReplayTargetData) {
    bool result = true;
    for (const auto& c : fClearList) {
        result &= commandBuffer->clearBuffer(c.fBuffer, c.fOffset, c.fSize);
    }
    return result;
}

}  // namespace skgpu::graphite
