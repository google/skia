/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawableOp.h"

#include "GrOpFlushState.h"
#include "SkDrawable.h"

GrDrawableOp::GrDrawableOp(SkDrawable* drawable,
                           const SkMatrix& matrix,
                           sk_sp<GrSemaphore> signalSemaphore,
                           sk_sp<GrSemaphore> waitSemaphore)
        : INHERITED(ClassID())
        , fDrawable(sk_ref_sp(drawable))
        , fMatrix(matrix)
        , fSignalSemaphore(std::move(signalSemaphore))
        , fWaitSemaphore(std::move(waitSemaphore)) {
}

void GrDrawableOp::onExecute(GrOpFlushState* state) {
    SkASSERT(state->commandBuffer());
    state->commandBuffer()->executeDrawable(fDrawable.get(), fMatrix,
                                            fSignalSemaphore.get(),
                                            fWaitSemaphore.get());
}

