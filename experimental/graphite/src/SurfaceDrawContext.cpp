/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/SurfaceDrawContext.h"

#include "experimental/graphite/src/SDCTask.h"

namespace skgpu {

sk_sp<SurfaceDrawContext> SurfaceDrawContext::Make(const SkImageInfo& ii) {
    sk_sp<SDCTask> task = SDCTask::Make();
    if (!task) {
        return nullptr;
    }

    return sk_sp<SurfaceDrawContext>(new SurfaceDrawContext(ii, std::move(task)));
}

SurfaceDrawContext::SurfaceDrawContext(const SkImageInfo& ii, sk_sp<SDCTask> task)
    : fImageInfo(ii)
    , fTask(std::move(task)) {
}

SurfaceDrawContext::~SurfaceDrawContext() {}

} // namespace skgpu
