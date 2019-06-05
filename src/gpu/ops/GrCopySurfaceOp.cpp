/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrCopySurfaceOp.h"

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"

// returns true if the read/written rect intersects the src/dst and false if not.
static bool clip_src_rect_and_dst_point(const GrSurfaceProxy* dst,
                                        const GrSurfaceProxy* src,
                                        const SkIRect& srcRect,
                                        const SkIPoint& dstPoint,
                                        SkIRect* clippedSrcRect,
                                        SkIPoint* clippedDstPoint) {
    *clippedSrcRect = srcRect;
    *clippedDstPoint = dstPoint;

    // clip the left edge to src and dst bounds, adjusting dstPoint if necessary
    if (clippedSrcRect->fLeft < 0) {
        clippedDstPoint->fX -= clippedSrcRect->fLeft;
        clippedSrcRect->fLeft = 0;
    }
    if (clippedDstPoint->fX < 0) {
        clippedSrcRect->fLeft -= clippedDstPoint->fX;
        clippedDstPoint->fX = 0;
    }

    // clip the top edge to src and dst bounds, adjusting dstPoint if necessary
    if (clippedSrcRect->fTop < 0) {
        clippedDstPoint->fY -= clippedSrcRect->fTop;
        clippedSrcRect->fTop = 0;
    }
    if (clippedDstPoint->fY < 0) {
        clippedSrcRect->fTop -= clippedDstPoint->fY;
        clippedDstPoint->fY = 0;
    }

    // clip the right edge to the src and dst bounds.
    if (clippedSrcRect->fRight > src->width()) {
        clippedSrcRect->fRight = src->width();
    }
    if (clippedDstPoint->fX + clippedSrcRect->width() > dst->width()) {
        clippedSrcRect->fRight = clippedSrcRect->fLeft + dst->width() - clippedDstPoint->fX;
    }

    // clip the bottom edge to the src and dst bounds.
    if (clippedSrcRect->fBottom > src->height()) {
        clippedSrcRect->fBottom = src->height();
    }
    if (clippedDstPoint->fY + clippedSrcRect->height() > dst->height()) {
        clippedSrcRect->fBottom = clippedSrcRect->fTop + dst->height() - clippedDstPoint->fY;
    }

    // The above clipping steps may have inverted the rect if it didn't intersect either the src or
    // dst bounds.
    return !clippedSrcRect->isEmpty();
}

std::unique_ptr<GrOp> GrCopySurfaceOp::Make(GrRecordingContext* context,
                                            GrSurfaceProxy* dstProxy,
                                            GrSurfaceProxy* srcProxy,
                                            const SkIRect& srcRect,
                                            const SkIPoint& dstPoint) {
    SkASSERT(dstProxy);
    SkASSERT(srcProxy);
    SkIRect clippedSrcRect;
    SkIPoint clippedDstPoint;
    // If the rect is outside the srcProxy or dstProxy then we've already succeeded.
    if (!clip_src_rect_and_dst_point(dstProxy, srcProxy, srcRect, dstPoint,
                                     &clippedSrcRect, &clippedDstPoint)) {
        return nullptr;
    }
    if (GrPixelConfigIsCompressed(dstProxy->config())) {
        return nullptr;
    }

    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrCopySurfaceOp>(srcProxy, clippedSrcRect, clippedDstPoint);
}

void GrCopySurfaceOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(fSrc.get()->isInstantiated());

    state->commandBuffer()->copy(fSrc.get()->peekSurface(), fSrc.get()->origin(), fSrcRect,
                                 fDstPoint);
}
