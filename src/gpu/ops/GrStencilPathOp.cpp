/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrStencilPathOp.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTarget.h"

GrOp::Owner GrStencilPathOp::Make(GrRecordingContext* context,
                                  const SkMatrix& viewMatrix,
                                  bool useHWAA,
                                  bool hasStencilClip,
                                  const GrScissorState& scissor,
                                  sk_sp<const GrPath> path) {
    return GrOp::Make<GrStencilPathOp>(context, viewMatrix, useHWAA,
                                       hasStencilClip, scissor, std::move(path));
}

void GrStencilPathOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    GrRenderTarget* rt = state->drawOpArgs().rtProxy()->peekRenderTarget();
    SkASSERT(rt);

    int numStencilBits = rt->numStencilBits();
    GrStencilSettings stencil(GrPathRendering::GetStencilPassSettings(fPath->getFillType()),
                              fHasStencilClip, numStencilBits);

    GrPathRendering::StencilPathArgs args(fUseHWAA, state->drawOpArgs().rtProxy(),
                                          state->drawOpArgs().writeView().origin(),
                                          &fViewMatrix, &fScissor, &stencil);
    state->gpu()->pathRendering()->stencilPath(args, fPath.get());
}
