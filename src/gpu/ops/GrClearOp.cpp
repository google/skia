/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrClearOp.h"

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"

std::unique_ptr<GrClearOp> GrClearOp::Make(GrRecordingContext* context,
                                           const GrScissorState& scissor,
                                           const SkPMColor4f& color) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();
    return pool->allocate<GrClearOp>(scissor, color);
}

GrClearOp::GrClearOp(const GrScissorState& scissor, const SkPMColor4f& color)
        : INHERITED(ClassID())
        , fScissor(scissor)
        , fColor(color) {
    this->setBounds(SkRect::Make(scissor.rect()), HasAABloat::kNo, IsHairline::kNo);
}

void GrClearOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(state->opsRenderPass());
    state->opsRenderPass()->clear(fScissor, fColor);
}
