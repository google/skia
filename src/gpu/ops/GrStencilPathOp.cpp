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
    SkASSERT(state->drawOpArgs().renderTarget());

    GrPathRendering::StencilPathArgs args(fUseHWAA, state->drawOpArgs().renderTarget(),
                                          state->drawOpArgs().fProxy->origin(),
                                          &fViewMatrix, &fScissor, &fStencil);
    state->gpu()->pathRendering()->stencilPath(args, fPath.get());
}

