/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrStencilPathOp.h"

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetPriv.h"

std::unique_ptr<GrOp> GrStencilPathOp::Make(GrRecordingContext* context,
                                            const SkMatrix& viewMatrix,
                                            bool useHWAA,
                                            bool hasStencilClip,
                                            const GrScissorState& scissor,
                                            sk_sp<const GrPath> path) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrStencilPathOp>(viewMatrix, useHWAA,
                                           hasStencilClip, scissor, std::move(path));
}

void GrStencilPathOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    GrRenderTarget* rt = state->drawOpArgs().proxy()->peekRenderTarget();
    SkASSERT(rt);

    int numStencilBits = rt->renderTargetPriv().numStencilBits();
    GrStencilSettings stencil(GrPathRendering::GetStencilPassSettings(fPath->getFillType()),
                              fHasStencilClip, numStencilBits);

    GrPathRendering::StencilPathArgs args(fUseHWAA, state->drawOpArgs().proxy(),
                                          state->drawOpArgs().origin(),
                                          &fViewMatrix, &fScissor, &stencil);
    state->gpu()->pathRendering()->stencilPath(args, fPath.get());
}
