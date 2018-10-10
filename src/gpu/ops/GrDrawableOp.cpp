/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawableOp.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpuCommandBuffer.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "SkDrawable.h"

std::unique_ptr<GrDrawableOp> GrDrawableOp::Make(GrContext* context, SkDrawable* drawable,
                                                 const SkMatrix& matrix) {
    GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();
    return pool->allocate<GrDrawableOp>(drawable, matrix);
}

GrDrawableOp::GrDrawableOp(SkDrawable* drawable,
                           const SkMatrix& matrix)
        : INHERITED(ClassID())
        , fDrawable(sk_ref_sp(drawable))
        , fMatrix(matrix) {
        this->setBounds(drawable->getBounds(), HasAABloat::kNo, IsZeroArea::kNo);
}

void GrDrawableOp::onExecute(GrOpFlushState* state) {
    SkASSERT(state->commandBuffer());
    state->rtCommandBuffer()->executeDrawable(fDrawable.get(), fMatrix);
}

