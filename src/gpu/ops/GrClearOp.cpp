/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClearOp.h"

#include "GrGpuCommandBuffer.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "GrProxyProvider.h"

std::unique_ptr<GrClearOp> GrClearOp::Make(GrContext* context,
                                           const GrFixedClip& clip,
                                           GrColor color,
                                           GrSurfaceProxy* dstProxy) {
    const SkIRect rect = SkIRect::MakeWH(dstProxy->width(), dstProxy->height());
    if (clip.scissorEnabled() && !SkIRect::Intersects(clip.scissorRect(), rect)) {
        return nullptr;
    }

    GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

    return pool->allocate<GrClearOp>(clip, color, dstProxy);
}

std::unique_ptr<GrClearOp> GrClearOp::Make(GrContext* context,
                                           const SkIRect& rect,
                                           GrColor color,
                                           bool fullScreen) {
    SkASSERT(fullScreen || !rect.isEmpty());

    GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

    return pool->allocate<GrClearOp>(rect, color, fullScreen);
}

GrClearOp::GrClearOp(const GrFixedClip& clip, GrColor color, GrSurfaceProxy* proxy)
        : INHERITED(ClassID())
        , fClip(clip)
        , fColor(color) {
    const SkIRect rtRect = SkIRect::MakeWH(proxy->width(), proxy->height());
    if (fClip.scissorEnabled()) {
        // Don't let scissors extend outside the RT. This may improve op combining.
        if (!fClip.intersect(rtRect)) {
            SkASSERT(0);  // should be caught upstream
            fClip = GrFixedClip(SkIRect::MakeEmpty());
        }

        if (GrProxyProvider::IsFunctionallyExact(proxy) && fClip.scissorRect() == rtRect) {
            fClip.disableScissor();
        }
    }
    this->setBounds(SkRect::Make(fClip.scissorEnabled() ? fClip.scissorRect() : rtRect),
                    HasAABloat::kNo, IsZeroArea::kNo);
}

void GrClearOp::onExecute(GrOpFlushState* state) {
    SkASSERT(state->rtCommandBuffer());
    state->rtCommandBuffer()->clear(fClip, fColor);
}
