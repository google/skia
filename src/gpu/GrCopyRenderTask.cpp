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
                                           const GrSurfaceProxyView& dstView,
                                           const SkIPoint& dstPoint,
                                           const GrCaps* caps) {
    SkASSERT(dstProxy);
    SkASSERT(srcProxy);
    SkIRect clippedSrcRect;
    SkIPoint clippedDstPoint;
    GrSurfaceProxy* dstProxy = dstView.asSurfaceProxy();
    // If the rect is outside the srcProxy or dstProxy then we've already succeeded.
    if (!GrClipSrcRectAndDstPoint(dstProxy->isize(), srcProxy->isize(), srcRect, dstPoint,
                                  &clippedSrcRect, &clippedDstPoint)) {
        return nullptr;
    }

    if (caps->isFormatCompressed(dstProxy->backendFormat())) {
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
            std::move(srcProxy), clippedSrcRect, dstView, clippedDstPoint));
    return task;
}

GrCopyRenderTask::GrCopyRenderTask(sk_sp<GrSurfaceProxy> srcProxy,
                                   const SkIRect& srcRect,
                                   const GrSurfaceProxyView& dstView,
                                   const SkIPoint& dstPoint)
        : GrRenderTask(dstView)
        , fSrcProxy(std::move(srcProxy))
        , fSrcRect(srcRect)
        , fDstPoint(dstPoint) {
    fTargetView.asSurfaceProxy()->setLastRenderTask(this);
}

void GrCopyRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // This renderTask doesn't have "normal" ops. In this case we still need to add an interval (so
    // fEndOfOpsTaskOpIndices will remain in sync), so we create a fake op# to capture the fact that
    // we read fSrcProxy and copy to fTarget.
    alloc->addInterval(fSrcProxy.get(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->addInterval(fTargetView.asSurfaceProxy(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->incOps();
}

bool GrCopyRenderTask::onExecute(GrOpFlushState* flushState) {
    GrSurfaceProxy* proxy = fTargetView.asSurfaceProxy();
    if (!fSrcProxy->isInstantiated() || !proxy->isInstantiated()) {
        return false;
    }
    GrSurface* srcSurface = fSrcProxy->peekSurface();
    GrSurface* dstSurface = proxy->peekSurface();
    if (fSrcProxy->origin() == kBottomLeft_GrSurfaceOrigin) {
        if (fSrcProxy->height() != srcSurface->height()) {
            fSrcRect.offset(0, srcSurface->height() - fSrcProxy->height());
        }
        if (proxy->height() != dstSurface->height()) {
            fDstPoint.fY = fDstPoint.fY + (dstSurface->height() - proxy->height());
        }
    }
    return flushState->gpu()->copySurface(dstSurface, srcSurface, fSrcRect, fDstPoint);
}

