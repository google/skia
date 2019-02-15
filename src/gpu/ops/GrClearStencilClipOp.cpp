/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClearStencilClipOp.h"

#include "GrGpuCommandBuffer.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"

std::unique_ptr<GrOp> GrClearStencilClipOp::Make(GrRecordingContext* context,
                                                 const GrFixedClip& clip,
                                                 bool insideStencilMask,
                                                 GrRenderTargetProxy* proxy) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrClearStencilClipOp>(clip, insideStencilMask, proxy);
}

void GrClearStencilClipOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(state->rtCommandBuffer());
    state->rtCommandBuffer()->clearStencilClip(fClip, fInsideStencilMask);
}
