/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpuCommandBuffer.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/ops/GrTransferFromOp.h"

std::unique_ptr<GrOp> GrTransferFromOp::Make(GrRecordingContext* context,
                                             const SkIRect& srcRect,
                                             GrColorType dstColorType,
                                             sk_sp<GrGpuBuffer> dstBuffer,
                                             size_t dstOffset) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();
    return pool->allocate<GrTransferFromOp>(srcRect, dstColorType, std::move(dstBuffer), dstOffset);
}

void GrTransferFromOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    state->commandBuffer()->transferFrom(fSrcRect, fDstColorType, fDstBuffer.get(), fDstOffset);
}
