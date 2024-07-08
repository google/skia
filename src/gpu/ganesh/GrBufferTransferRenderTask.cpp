/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/GrBufferTransferRenderTask.h"

#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrOpFlushState.h"

#include <utility>

sk_sp<GrRenderTask> GrBufferTransferRenderTask::Make(sk_sp<GrGpuBuffer> src,
                                                     size_t srcOffset,
                                                     sk_sp<GrGpuBuffer> dst,
                                                     size_t dstOffset,
                                                     size_t size) {
    return sk_sp<GrRenderTask>(new GrBufferTransferRenderTask(std::move(src),
                                                              srcOffset,
                                                              std::move(dst),
                                                              dstOffset,
                                                              size));
}

GrBufferTransferRenderTask::GrBufferTransferRenderTask(sk_sp<GrGpuBuffer> src,
                                                       size_t srcOffset,
                                                       sk_sp<GrGpuBuffer> dst,
                                                       size_t dstOffset,
                                                       size_t size)
        : fSrc(std::move(src))
        , fDst(std::move(dst))
        , fSrcOffset(srcOffset)
        , fDstOffset(dstOffset)
        , fSize(size) {
    this->setFlag(kBlocksReordering_Flag);
}

GrBufferTransferRenderTask::~GrBufferTransferRenderTask() = default;

bool GrBufferTransferRenderTask::onExecute(GrOpFlushState* flushState) {
    return flushState->gpu()->transferFromBufferToBuffer(fSrc,
                                                         fSrcOffset,
                                                         fDst,
                                                         fDstOffset,
                                                         fSize);
}
