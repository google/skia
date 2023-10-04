/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrTransferFromRenderTask.h"

#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrResourceAllocator.h"
#include "src/gpu/ganesh/GrResourceProvider.h"

void GrTransferFromRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // This renderTask doesn't have "normal" ops. In this case we still need to add an interval (so
    // fEndOfOpsTaskOpIndices will remain in sync), so we create a fake op# to capture the fact that
    // we read fSrcProxy.
    alloc->addInterval(fSrcProxy.get(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes,
                       GrResourceAllocator::AllowRecycling::kYes);
    alloc->incOps();
}

bool GrTransferFromRenderTask::onExecute(GrOpFlushState* flushState) {
    if (!fSrcProxy->isInstantiated()) {
        return false;
    }
    return flushState->gpu()->transferPixelsFrom(fSrcProxy->peekSurface(),
                                                 fSrcRect,
                                                 fSurfaceColorType,
                                                 fDstColorType,
                                                 fDstBuffer,
                                                 fDstOffset);
}
