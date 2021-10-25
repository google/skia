/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/RenderPassTask.h"

#include "experimental/graphite/src/DrawPass.h"

namespace skgpu {

sk_sp<RenderPassTask> RenderPassTask::Make(std::vector<std::unique_ptr<DrawPass>> passes) {
    return sk_sp<RenderPassTask>(new RenderPassTask(std::move(passes)));
}

RenderPassTask::RenderPassTask(std::vector<std::unique_ptr<DrawPass>> passes)
        : fDrawPasses(std::move(passes)) {}

RenderPassTask::~RenderPassTask() = default;

} // namespace skgpu
