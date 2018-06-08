/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilPathOp.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetPriv.h"

std::unique_ptr<GrOp> GrStencilPathOp::Make(GrContext* context,
                                            const SkMatrix& viewMatrix,
                                            bool useHWAA,
                                            GrPathRendering::FillType fillType,
                                            bool hasStencilClip,
                                            const GrScissorState& scissor,
                                            const GrPath* path) {
    // $$
    GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

#if 0
    char* mem = (char*) pool->allocate(sizeof(GrStencilPathOp));
    return std::unique_ptr<GrOp>(new (mem) GrStencilPathOp(viewMatrix, useHWAA, fillType,
                                                            hasStencilClip, scissor, path));
#else
    return pool->allocate<GrStencilPathOp>(viewMatrix, useHWAA, fillType,
                                           hasStencilClip, scissor, path);
#endif
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

