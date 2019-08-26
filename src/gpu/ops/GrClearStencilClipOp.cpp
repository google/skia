/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrClearStencilClipOp.h"

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrRecordingContextPriv.h"

std::unique_ptr<GrOp> GrClearStencilClipOp::Make(GrRecordingContext* context,
                                                 const GrFixedClip& clip,
                                                 bool insideStencilMask,
                                                 GrRenderTargetProxy* proxy) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrClearStencilClipOp>(clip, insideStencilMask, proxy);
}

void GrClearStencilClipOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(state->opsRenderPass());
    state->opsRenderPass()->clearStencilClip(fClip, fInsideStencilMask);
}
