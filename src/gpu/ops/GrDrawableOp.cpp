/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawableOp.h"

#include "GrGpuCommandBuffer.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "SkDrawable.h"

std::unique_ptr<GrDrawableOp> GrDrawableOp::Make(
        GrRecordingContext* context, std::unique_ptr<SkDrawable::GpuDrawHandler> drawable,
        const SkRect& bounds) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();
    return pool->allocate<GrDrawableOp>(std::move(drawable), bounds);
}

GrDrawableOp::GrDrawableOp(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable,
                           const SkRect& bounds)
        : INHERITED(ClassID())
        , fDrawable(std::move(drawable)) {
        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
}

void GrDrawableOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(state->commandBuffer());
    state->rtCommandBuffer()->executeDrawable(std::move(fDrawable));
}
