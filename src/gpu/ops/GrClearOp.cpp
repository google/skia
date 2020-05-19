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
                                           const SkPMColor4f& color,
                                           GrSurfaceProxy* dstProxy) {
    const SkIRect rect = SkIRect::MakeSize(dstProxy->dimensions());
    if (scissor.enabled() && !SkIRect::Intersects(scissor.rect(), rect)) {
        return nullptr;
    }

    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrClearOp>(scissor, color, dstProxy);
}

std::unique_ptr<GrClearOp> GrClearOp::Make(GrRecordingContext* context,
                                           const SkIRect& rect,
                                           const SkPMColor4f& color,
                                           bool fullScreen) {
    SkASSERT(fullScreen || !rect.isEmpty());

    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrClearOp>(rect, color, fullScreen);
}

GrClearOp::GrClearOp(const GrScissorState& scissor, const SkPMColor4f& color, GrSurfaceProxy* proxy)
        : INHERITED(ClassID())
        , fScissor(scissor)
        , fColor(color) {
    const SkIRect rtRect = SkIRect::MakeSize(proxy->dimensions());
    if (fScissor.enabled()) {
        // Don't let scissors extend outside the RT. This may improve op combining.
        if (!fScissor.intersect(rtRect)) {
            SkASSERT(0);  // should be caught upstream
            fScissor.set(SkIRect::MakeEmpty());
        }

        if (proxy->isFunctionallyExact() && fScissor.rect() == rtRect) {
            fScissor.setDisabled();
        }
    }
    this->setBounds(SkRect::Make(fScissor.enabled() ? fScissor.rect() : rtRect),
                    HasAABloat::kNo, IsHairline::kNo);
}

void GrClearOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(state->opsRenderPass());
    state->opsRenderPass()->clear(fScissor, fColor);
}
