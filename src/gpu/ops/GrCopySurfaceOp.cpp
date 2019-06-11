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
#include "src/gpu/geometry/GrRect.h"

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
    if (!GrClipSrcRectAndDstPoint(dstProxy->isize(), srcProxy->isize(), srcRect, dstPoint,
                                  &clippedSrcRect, &clippedDstPoint)) {
        return nullptr;
    }
    if (GrPixelConfigIsCompressed(dstProxy->config())) {
        return nullptr;
    }

    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrCopySurfaceOp>(srcProxy, dstProxy, clippedSrcRect, clippedDstPoint);
}

void GrCopySurfaceOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    SkASSERT(fSrc.get()->isInstantiated());

    // If we are using approx surfaces we may need to adjust our srcRect or dstPoint if the origin
    // is bottom left.
    GrSurfaceProxy* src = fSrc.get();
    if (src->origin() == kBottomLeft_GrSurfaceOrigin) {
        GrSurfaceProxy* dst = fDst.get();
        SkASSERT(dst->isInstantiated());
        if (src->height() != src->peekSurface()->height()) {
            fSrcRect.offset(0, src->peekSurface()->height() - src->height());
        }
        if (dst->height() != dst->peekSurface()->height()) {
            fDstPoint.fY = fDstPoint.fY + (dst->peekSurface()->height() - dst->height());
        }
    }

    state->commandBuffer()->copy(fSrc.get()->peekSurface(), fSrcRect, fDstPoint);
}
