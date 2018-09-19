/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrStencilPathOp.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRenderTargetPriv.h"

std::unique_ptr<GrOp> GrStencilPathOp::Make(GrContext* context,
                                            const SkMatrix& viewMatrix,
                                            bool useHWAA,
                                            GrPathRendering::FillType fillType,
                                            bool hasStencilClip,
                                            const GrScissorState& scissor,
                                            const GrPath* path) {
    GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

    return pool->allocate<GrStencilPathOp>(viewMatrix, useHWAA, fillType,
                                           hasStencilClip, scissor, path);
}

void GrStencilPathOp::onExecute(GrOpFlushState* state) {
    GrRenderTarget* rt = state->drawOpArgs().renderTarget();
    SkASSERT(rt);

    int numStencilBits = rt->renderTargetPriv().numStencilBits();
    GrStencilSettings stencil(GrPathRendering::GetStencilPassSettings(fFillType),
                              fHasStencilClip, numStencilBits);

    GrPathRendering::StencilPathArgs args(fUseHWAA, state->drawOpArgs().fProxy,
                                          &fViewMatrix, &fScissor, &stencil);
    state->gpu()->pathRendering()->stencilPath(args, fPath.get());
}

