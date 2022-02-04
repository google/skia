/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/RecorderPriv.h"

#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/TaskGraph.h"

namespace skgpu {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(fRecorder->singleOwner())

ResourceProvider* RecorderPriv::resourceProvider() const {
    return fRecorder->fResourceProvider.get();
}

UniformCache* RecorderPriv::uniformCache() const {
    return fRecorder->fUniformCache.get();
}

const Caps* RecorderPriv::caps() const {
    return fRecorder->fGpu->caps();
}

DrawBufferManager* RecorderPriv::drawBufferManager() const {
    return fRecorder->fDrawBufferManager.get();
}

void RecorderPriv::add(sk_sp<Task> task) {
    ASSERT_SINGLE_OWNER
    fRecorder->fGraph->add(std::move(task));
}

} // namespace skgpu
