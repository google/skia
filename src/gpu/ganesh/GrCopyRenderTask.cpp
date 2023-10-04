/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrCopyRenderTask.h"

#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrNativeRect.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrResourceAllocator.h"

sk_sp<GrRenderTask> GrCopyRenderTask::Make(GrDrawingManager* drawingMgr,
                                           sk_sp<GrSurfaceProxy> dst,
                                           SkIRect dstRect,
                                           sk_sp<GrSurfaceProxy> src,
                                           SkIRect srcRect,
                                           GrSamplerState::Filter filter,
                                           GrSurfaceOrigin origin) {
    SkASSERT(src);
    SkASSERT(dst);

    // canCopySurface() should have returned true, guaranteeing this property.
    SkASSERT(SkIRect::MakeSize(dst->dimensions()).contains(dstRect));
    SkASSERT(SkIRect::MakeSize(src->dimensions()).contains(srcRect));

    return sk_sp<GrRenderTask>(new GrCopyRenderTask(drawingMgr,
                                                    std::move(dst),
                                                    dstRect,
                                                    std::move(src),
                                                    srcRect,
                                                    filter,
                                                    origin));
}

GrCopyRenderTask::GrCopyRenderTask(GrDrawingManager* drawingMgr,
                                   sk_sp<GrSurfaceProxy> dst,
                                   SkIRect dstRect,
                                   sk_sp<GrSurfaceProxy> src,
                                   SkIRect srcRect,
                                   GrSamplerState::Filter filter,
                                   GrSurfaceOrigin origin)
        : fSrc(std::move(src))
        , fSrcRect(srcRect)
        , fDstRect(dstRect)
        , fFilter(filter)
        , fOrigin(origin) {
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
                       GrResourceAllocator::ActualUse::kYes,
                       GrResourceAllocator::AllowRecycling::kYes);
    alloc->addInterval(this->target(0), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes,
                       GrResourceAllocator::AllowRecycling::kYes);
    alloc->incOps();
}

GrRenderTask::ExpectedOutcome GrCopyRenderTask::onMakeClosed(GrRecordingContext*,
                                                             SkIRect* targetUpdateBounds) {
    // We don't expect to be marked skippable before being closed.
    SkASSERT(fSrc);
    *targetUpdateBounds = GrNativeRect::MakeIRectRelativeTo(
            fOrigin,
            this->target(0)->height(),
            fDstRect);
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
    SkIRect dstRect = GrNativeRect::MakeIRectRelativeTo(fOrigin, dstSurface->height(), fDstRect);
    return flushState->gpu()->copySurface(dstSurface, dstRect, srcSurface, srcRect, fFilter);
}
