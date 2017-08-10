/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilPathOp.h"

#include "GrGpu.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetPriv.h"

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

