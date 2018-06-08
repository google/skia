/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClearStencilClipOp.h"

#include "GrGpuCommandBuffer.h"
#include "GrMemoryPool.h"

std::unique_ptr<GrOp> GrClearStencilClipOp::Make(GrContext* context,
                                                 const GrFixedClip& clip,
                                                 bool insideStencilMask,
                                                 GrRenderTargetProxy* proxy) {
    GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

    return pool->allocate<GrClearStencilClipOp>(clip, insideStencilMask, proxy);
}

void GrClearStencilClipOp::onExecute(GrOpFlushState* state) {
    SkASSERT(state->rtCommandBuffer());
    state->rtCommandBuffer()->clearStencilClip(fClip, fInsideStencilMask);
}
