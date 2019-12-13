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
                                           const GrFixedClip& clip,
                                           const SkPMColor4f& color,
                                           GrSurfaceProxy* dstProxy) {
    const SkIRect rect = SkIRect::MakeSize(dstProxy->dimensions());
    if (clip.scissorEnabled() && !SkIRect::Intersects(clip.scissorRect(), rect)) {
        return nullptr;
    }

    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrClearOp>(clip, color, dstProxy);
}

std::unique_ptr<GrClearOp> GrClearOp::Make(GrRecordingContext* context,
                                           const SkIRect& rect,
                                           const SkPMColor4f& color,
                                           bool fullScreen) {
    SkASSERT(fullScreen || !rect.isEmpty());

    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrClearOp>(rect, color, fullScreen);
}

GrClearOp::GrClearOp(const GrFixedClip& clip, const SkPMColor4f& color, GrSurfaceProxy* proxy)
        : INHERITED(ClassID())
        , fClip(clip)
        , fColor(color) {
    const SkIRect rtRect = SkIRect::MakeSize(proxy->dimensions());
    if (fClip.scissorEnabled()) {
        // Don't let scissors extend outside the RT. This may improve op combining.
        if (!fClip.intersect(rtRect)) {
            SkASSERT(0);  // should be caught upstream
            fClip = GrFixedClip(SkIRect::MakeEmpty());
        }

        if (proxy->isFunctionallyExact() && fClip.scissorRect() == rtRect) {
            fClip.disableScissor();
        }
    }
    this->setBounds(SkRect::Make(fClip.scissorEnabled() ? fClip.scissorRect() : rtRect),
                    HasAABloat::kNo, IsHairline::kNo);
}

void GrClearOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(state->opsRenderPass());
    state->opsRenderPass()->clear(fClip, fColor);
}
