/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrBufferUpdateRenderTask.h"

#include "include/core/SkData.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"

#include <utility>

sk_sp<GrRenderTask> GrBufferUpdateRenderTask::Make(sk_sp<SkData> src,
                                                   sk_sp<GrGpuBuffer> dst,
                                                   size_t dstOffset) {
    return sk_sp<GrRenderTask>(new GrBufferUpdateRenderTask(std::move(src),
                                                            std::move(dst),
                                                            dstOffset));
}

GrBufferUpdateRenderTask::GrBufferUpdateRenderTask(sk_sp<SkData> src,
                                                   sk_sp<GrGpuBuffer> dst,
                                                   size_t dstOffset)
        : fSrc(std::move(src))
        , fDst(std::move(dst))
        , fDstOffset(dstOffset) {
    this->setFlag(kBlocksReordering_Flag);
}

GrBufferUpdateRenderTask::~GrBufferUpdateRenderTask() = default;

bool GrBufferUpdateRenderTask::onExecute(GrOpFlushState* flushState) {
    return fDst->updateData(fSrc->data(), fDstOffset, fSrc->size(), /*preserve=*/true);
}
