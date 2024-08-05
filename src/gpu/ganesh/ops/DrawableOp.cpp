/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/DrawableOp.h"

#include "include/core/SkDrawable.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrOpsRenderPass.h"

#include <utility>

namespace skgpu::ganesh {

GrOp::Owner DrawableOp::Make(GrRecordingContext* context,
                             std::unique_ptr<SkDrawable::GpuDrawHandler> drawable,
                             const SkRect& bounds) {
    return GrOp::Make<DrawableOp>(context, std::move(drawable), bounds);
}

DrawableOp::DrawableOp(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable,
                       const SkRect& bounds)
        : GrOp(ClassID())
        , fDrawable(std::move(drawable)) {
    this->setBounds(bounds, HasAABloat::kNo, IsHairline::kNo);
}

void DrawableOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(state->opsRenderPass());
    state->opsRenderPass()->executeDrawable(std::move(fDrawable));
}

}  // namespace skgpu::ganesh
