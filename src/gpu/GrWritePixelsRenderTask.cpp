/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrWritePixelsRenderTask.h"

#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceAllocator.h"

sk_sp<GrRenderTask> GrWritePixelsTask::Make(GrDrawingManager* dm,
                                            sk_sp<GrSurfaceProxy> dst,
                                            SkIRect rect,
                                            GrColorType srcColorType,
                                            GrColorType dstColorType,
                                            const GrMipLevel texels[],
                                            int levelCount,
                                            sk_sp<SkData> pixelStorage) {
    return sk_sp<GrRenderTask>(new GrWritePixelsTask(dm,
                                                     std::move(dst),
                                                     rect,
                                                     srcColorType,
                                                     dstColorType,
                                                     texels,
                                                     levelCount,
                                                     std::move(pixelStorage)));
}

GrWritePixelsTask::GrWritePixelsTask(GrDrawingManager* dm,
                                     sk_sp<GrSurfaceProxy> dst,
                                     SkIRect rect,
                                     GrColorType srcColorType,
                                     GrColorType dstColorType,
                                     const GrMipLevel texels[],
                                     int levelCount,
                                     sk_sp<SkData> pixelStorage)
        : fRect(rect)
        , fSrcColorType(srcColorType)
        , fDstColorType(dstColorType)
        , fStorage(std::move(pixelStorage)) {
    this->addTarget(dm, std::move(dst));
    fLevels.reset(levelCount);
    std::copy_n(texels, levelCount, fLevels.get());
}

void GrWritePixelsTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    alloc->addInterval(this->target(0), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->incOps();
}

GrRenderTask::ExpectedOutcome GrWritePixelsTask::onMakeClosed(const GrCaps&,
                                                              SkIRect* targetUpdateBounds) {
    *targetUpdateBounds = fRect;
    return ExpectedOutcome::kTargetDirty;
}

bool GrWritePixelsTask::onExecute(GrOpFlushState* flushState) {
    GrSurfaceProxy* dstProxy = this->target(0);
    if (!dstProxy->isInstantiated()) {
        return false;
    }
    GrSurface* dstSurface = dstProxy->peekSurface();
    return flushState->gpu()->writePixels(dstSurface,
                                          fRect.fLeft,
                                          fRect.fTop,
                                          fRect.width(),
                                          fRect.height(),
                                          fDstColorType,
                                          fSrcColorType,
                                          fLevels.get(),
                                          fLevels.count());
}
