/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawableOp.h"

#include "GrGpuCommandBuffer.h"
#include "GrOpFlushState.h"
#include "SkDrawable.h"

GrDrawableOp::GrDrawableOp(SkDrawable* drawable,
                           const SkMatrix& matrix,
                           sk_sp<GrSemaphore> semaphoreToWaitOn,
                           sk_sp<GrSemaphore> semaphoreToSignal)
        : INHERITED(ClassID())
        , fDrawable(sk_ref_sp(drawable))
        , fMatrix(matrix)
        , fSemaphoreToWaitOn(std::move(semaphoreToWaitOn))
        , fSemaphoreToSignal(std::move(semaphoreToSignal)) {
        this->setBounds(drawable->getBounds(), HasAABloat::kNo, IsZeroArea::kNo);
}

void GrDrawableOp::onExecute(GrOpFlushState* state) {
    SkASSERT(state->commandBuffer());
    SkASSERT(state->drawOpArgs().fRenderTarget);
    state->commandBuffer()->executeDrawable(state->drawOpArgs().fRenderTarget,
                                            fDrawable.get(), fMatrix,
                                            fSemaphoreToWaitOn.get(),
                                            fSemaphoreToSignal.get());
}

