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
                                           sk_sp<GrSurfaceProxy> src,
                                           SkIRect srcRect,
                                           sk_sp<GrSurfaceProxy> dst,
                                           SkIPoint dstPoint,
                                           const GrCaps* caps) {
    SkASSERT(src);
    SkASSERT(dst);

    // Make sure our caller's values are inside the backing surfaces' bounds.
    SkASSERT(SkIRect::MakeSize(src->backingStoreDimensions()).contains(srcRect));
    SkASSERT(SkIRect::MakeSize(dst->backingStoreDimensions()).contains(
             SkIRect::MakePtSize(dstPoint, srcRect.size())));

    sk_sp<GrCopyRenderTask> task(new GrCopyRenderTask(drawingMgr,
                                                      std::move(src),
                                                      srcRect,
                                                      std::move(dst),
                                                      dstPoint));
    return std::move(task);
}

GrCopyRenderTask::GrCopyRenderTask(GrDrawingManager* drawingMgr,
                                   sk_sp<GrSurfaceProxy> src,
                                   SkIRect srcRect,
                                   sk_sp<GrSurfaceProxy> dst,
                                   SkIPoint dstPoint)
        : GrRenderTask(), fSrc(std::move(src)), fSrcRect(srcRect), fDstPoint(dstPoint) {
    this->addTarget(drawingMgr, std::move(dst));
}

void GrCopyRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // This renderTask doesn't have "normal" ops. In this case we still need to add an interval (so
    // fEndOfOpsTaskOpIndices will remain in sync), so we create a fake op# to capture the fact that
    // we read fSrcView and copy to target view.
    alloc->addInterval(fSrc.get(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->addInterval(this->target(0), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->incOps();
}

bool GrCopyRenderTask::onExecute(GrOpFlushState* flushState) {
    GrSurfaceProxy* dstProxy = this->target(0);
    if (!fSrc->isInstantiated() || !dstProxy->isInstantiated()) {
        return false;
    }
    GrSurface* srcSurface = fSrc->peekSurface();
    GrSurface* dstSurface = dstProxy->peekSurface();
    return flushState->gpu()->copySurface(dstSurface, srcSurface, fSrcRect, fDstPoint);
}

