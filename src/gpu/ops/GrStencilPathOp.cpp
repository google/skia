/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilPathOp.h"

#include "GrGpu.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "GrRenderTargetPriv.h"

std::unique_ptr<GrOp> GrStencilPathOp::Make(GrRecordingContext* context,
                                            const SkMatrix& viewMatrix,
                                            bool useHWAA,
                                            GrPathRendering::FillType fillType,
                                            bool hasStencilClip,
                                            const GrScissorState& scissor,
                                            const GrPath* path) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrStencilPathOp>(viewMatrix, useHWAA, fillType,
                                           hasStencilClip, scissor, path);
}

void GrStencilPathOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    GrRenderTarget* rt = state->drawOpArgs().renderTarget();
    SkASSERT(rt);

    int numStencilBits = rt->renderTargetPriv().numStencilBits();
    GrStencilSettings stencil(GrPathRendering::GetStencilPassSettings(fFillType),
                              fHasStencilClip, numStencilBits);

    GrPathRendering::StencilPathArgs args(fUseHWAA, state->drawOpArgs().fProxy,
                                          &fViewMatrix, &fScissor, &stencil);
    state->gpu()->pathRendering()->stencilPath(args, fPath.get());
}
