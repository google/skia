/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilPathOp.h"

#include "GrGpu.h"
#include "GrOpFlushState.h"

void GrStencilPathOp::onExecute(GrOpFlushState* state) {
    SkASSERT(state->drawOpArgs().fRenderTarget);

    GrPathRendering::StencilPathArgs args(fUseHWAA, state->drawOpArgs().fRenderTarget,
                                          &fViewMatrix, &fScissor, &fStencil);
    state->gpu()->pathRendering()->stencilPath(args, fPath.get());
}

