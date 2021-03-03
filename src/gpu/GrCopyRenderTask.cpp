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
                                           GrSurfaceOrigin origin) {
    SkASSERT(src);
    SkASSERT(dst);

    if (!GrClipSrcRectAndDstPoint(dst->dimensions(),
                                  src->dimensions(),
                                  srcRect,
                                  dstPoint,
                                  &srcRect,
                                  &dstPoint)) {
        return nullptr;
    }

    return sk_sp<GrRenderTask>(new GrCopyRenderTask(drawingMgr,
                                                    std::move(src),
                                                    srcRect,
                                                    std::move(dst),
                                                    dstPoint,
                                                    origin));
}

GrCopyRenderTask::GrCopyRenderTask(GrDrawingManager* drawingMgr,
                                   sk_sp<GrSurfaceProxy> src,
                                   SkIRect srcRect,
                                   sk_sp<GrSurfaceProxy> dst,
                                   SkIPoint dstPoint,
                                   GrSurfaceOrigin origin)
        : fSrc(std::move(src)), fSrcRect(srcRect), fDstPoint(dstPoint), fOrigin(origin) {
    this->addTarget(drawingMgr, std::move(dst));
}

void GrCopyRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    if (!fSrc) {
        alloc->incOps();
        return;
    }
    // This renderTask doesn't have "normal" ops. In this case we still need to add an interval (so
    // fEndOfOpsTaskOpIndices will remain in sync), so we create a fake op# to capture the fact that
    // we read fSrcView and copy to target view.
    alloc->addInterval(fSrc.get(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->addInterval(this->target(0), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->incOps();
}

GrRenderTask::ExpectedOutcome GrCopyRenderTask::onMakeClosed(const GrCaps&,
                                                             SkIRect* targetUpdateBounds) {
    // We don't expect to be marked skippable before being closed.
    SkASSERT(fSrc);
    *targetUpdateBounds = GrNativeRect::MakeIRectRelativeTo(
            fOrigin,
            this->target(0)->height(),
            SkIRect::MakePtSize(fDstPoint, fSrcRect.size()));
    return ExpectedOutcome::kTargetDirty;
}

bool GrCopyRenderTask::onExecute(GrOpFlushState* flushState) {
    if (!fSrc) {
        // Did nothing, just like we're supposed to.
        return true;
    }
    GrSurfaceProxy* dstProxy = this->target(0);
    if (!fSrc->isInstantiated() || !dstProxy->isInstantiated()) {
        return false;
    }
    GrSurface* srcSurface = fSrc->peekSurface();
    GrSurface* dstSurface = dstProxy->peekSurface();
    SkIRect srcRect = GrNativeRect::MakeIRectRelativeTo(fOrigin, srcSurface->height(), fSrcRect);
    SkIPoint dstPoint = fDstPoint;
    if (fOrigin == kBottomLeft_GrSurfaceOrigin) {
        dstPoint.fY = dstSurface->height() - dstPoint.fY - srcRect.height();
    }
    return flushState->gpu()->copySurface(dstSurface, srcSurface, srcRect, dstPoint);
}

