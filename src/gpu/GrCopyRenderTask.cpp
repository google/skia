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

sk_sp<GrRenderTask> GrCopyRenderTask::Make(GrDrawingManager* drawingMgr,
                                           GrSurfaceProxyView srcView,
                                           const SkIRect& srcRect,
                                           GrSurfaceProxyView dstView,
                                           const SkIPoint& dstPoint,
                                           const GrCaps* caps) {
    SkASSERT(dstView.proxy());
    SkASSERT(srcView.proxy());
    SkIRect clippedSrcRect;
    SkIPoint clippedDstPoint;
    GrSurfaceProxy* srcProxy = srcView.proxy();
    GrSurfaceProxy* dstProxy = dstView.proxy();
    // If the rect is outside the srcProxy or dstProxy then we've already succeeded.
    if (!GrClipSrcRectAndDstPoint(dstProxy->dimensions(), srcProxy->dimensions(), srcRect, dstPoint,
                                  &clippedSrcRect, &clippedDstPoint)) {
        return nullptr;
    }

    if (caps->isFormatCompressed(dstProxy->backendFormat())) {
        return nullptr;
    }

    SkASSERT(dstView.origin() == srcView.origin());
    if (srcView.origin() == kBottomLeft_GrSurfaceOrigin) {
        int rectHeight = clippedSrcRect.height();
        clippedSrcRect.fTop = srcProxy->height() - clippedSrcRect.fBottom;
        clippedSrcRect.fBottom = clippedSrcRect.fTop + rectHeight;
        clippedDstPoint.fY = dstProxy->height() - clippedDstPoint.fY - rectHeight;
    }

    sk_sp<GrCopyRenderTask> task(new GrCopyRenderTask(
            drawingMgr, std::move(srcView), clippedSrcRect, std::move(dstView), clippedDstPoint));
    return std::move(task);
}

GrCopyRenderTask::GrCopyRenderTask(GrDrawingManager* drawingMgr,
                                   GrSurfaceProxyView srcView,
                                   const SkIRect& srcRect,
                                   GrSurfaceProxyView dstView,
                                   const SkIPoint& dstPoint)
        : GrRenderTask()
        , fSrcView(std::move(srcView))
        , fSrcRect(srcRect)
        , fDstPoint(dstPoint) {
    this->addTarget(drawingMgr, dstView);
}

void GrCopyRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // This renderTask doesn't have "normal" ops. In this case we still need to add an interval (so
    // fEndOfOpsTaskOpIndices will remain in sync), so we create a fake op# to capture the fact that
    // we read fSrcView and copy to target view.
    alloc->addInterval(fSrcView.proxy(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->addInterval(this->target(0).proxy(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->incOps();
}

bool GrCopyRenderTask::onExecute(GrOpFlushState* flushState) {
    GrSurfaceProxy* dstProxy = this->target(0).proxy();
    GrSurfaceProxy* srcProxy = fSrcView.proxy();
    if (!srcProxy->isInstantiated() || !dstProxy->isInstantiated()) {
        return false;
    }
    GrSurface* srcSurface = srcProxy->peekSurface();
    GrSurface* dstSurface = dstProxy->peekSurface();
    if (fSrcView.origin() == kBottomLeft_GrSurfaceOrigin) {
        if (srcProxy->height() != srcSurface->height()) {
            fSrcRect.offset(0, srcSurface->height() - srcProxy->height());
        }
        if (dstProxy->height() != dstSurface->height()) {
            fDstPoint.fY = fDstPoint.fY + (dstSurface->height() - dstProxy->height());
        }
    }
    return flushState->gpu()->copySurface(dstSurface, srcSurface, fSrcRect, fDstPoint);
}

