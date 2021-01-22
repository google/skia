/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrDrawableOp.h"

#include "include/core/SkDrawable.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrRecordingContextPriv.h"

GrOp::Owner GrDrawableOp::Make(
        GrRecordingContext* context, std::unique_ptr<SkDrawable::GpuDrawHandler> drawable,
        const SkRect& bounds) {
    return GrOp::Make<GrDrawableOp>(context, std::move(drawable), bounds);
}

GrDrawableOp::GrDrawableOp(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable,
                           const SkRect& bounds)
        : INHERITED(ClassID())
        , fDrawable(std::move(drawable)) {
    this->setBounds(bounds, HasAABloat::kNo, IsHairline::kNo);
}

void GrDrawableOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(state->opsRenderPass());
    state->opsRenderPass()->executeDrawable(std::move(fDrawable));
}
