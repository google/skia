/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrCopyRenderTask.h"

#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceAllocator.h"

sk_sp<GrRenderTask> GrCopyRenderTask::Make(sk_sp<GrSurfaceProxy> srcProxy,
                                           const SkIRect& srcRect,
                                           sk_sp<GrSurfaceProxy> dstProxy,
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

    SkASSERT(dstProxy->origin() == srcProxy->origin());
    if (srcProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
        int rectHeight = clippedSrcRect.height();
        clippedSrcRect.fTop = srcProxy->height() - clippedSrcRect.fBottom;
        clippedSrcRect.fBottom = clippedSrcRect.fTop + rectHeight;
        clippedDstPoint.fY = dstProxy->height() - clippedDstPoint.fY - rectHeight;
    }

    sk_sp<GrCopyRenderTask> task(new GrCopyRenderTask(
            std::move(srcProxy), clippedSrcRect, std::move(dstProxy), clippedDstPoint));
    return task;
}

GrCopyRenderTask::GrCopyRenderTask(sk_sp<GrSurfaceProxy> srcProxy,
                                   const SkIRect& srcRect,
                                   sk_sp<GrSurfaceProxy> dstProxy,
                                   const SkIPoint& dstPoint)
        : GrRenderTask(std::move(dstProxy))
        , fSrcProxy(std::move(srcProxy))
        , fSrcRect(srcRect)
        , fDstPoint(dstPoint) {
    fTarget->setLastRenderTask(this);
}

void GrCopyRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // This renderTask doesn't have "normal" ops. In this case we still need to add an interval (so
    // fEndOfOpsTaskOpIndices will remain in sync), so we create a fake op# to capture the fact that
    // we read fSrcProxy and copy to fTarget.
    alloc->addInterval(fSrcProxy.get(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->addInterval(fTarget.get(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->incOps();
}

bool GrCopyRenderTask::onExecute(GrOpFlushState* flushState) {
    if (!fSrcProxy->isInstantiated() || !fTarget->isInstantiated()) {
        return false;
    }
    GrSurface* srcSurface = fSrcProxy->peekSurface();
    GrSurface* dstSurface = fTarget->peekSurface();
    if (fSrcProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
        if (fSrcProxy->height() != srcSurface->height()) {
            fSrcRect.offset(0, srcSurface->height() - fSrcProxy->height());
        }
        if (fTarget->height() != dstSurface->height()) {
            fDstPoint.fY = fDstPoint.fY + (dstSurface->height() - fTarget->height());
        }
    }
    return flushState->gpu()->copySurface(dstSurface, srcSurface, fSrcRect, fDstPoint);
}

