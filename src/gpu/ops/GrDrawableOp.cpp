/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawableOp.h"

#include "SkDrawable.h"

GrDrawableOp::GrDrawableOP(SkDrawable* drawable, const SkMatrix& matrix)
        : INHERITED(ClassID()), fDrawable(sk_ref_sp(drawable), fMatrix(matrix) {
}

void GrDrawable::onExecute(GrOpFlushState* state, const SkRect& bounds) {
    SkASSERT(state->commandBuffer());
    state->commandBuffer()->executeDrawable(fDrawable, fMatrix);
}

