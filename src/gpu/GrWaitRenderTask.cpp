/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrWaitRenderTask.h"

#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceAllocator.h"

void GrWaitRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // This renderTask doesn't have "normal" ops. In this case we still need to add an interval (so
    // fEndOfOpsTaskOpIndices will remain in sync), so we create a fake op# to capture the fact that
    // we manipulate fTargetView's proxy.
    alloc->addInterval(fTargetView.proxy(), alloc->curOp(), alloc->curOp(),
                       GrResourceAllocator::ActualUse::kYes);
    alloc->incOps();
}

bool GrWaitRenderTask::onExecute(GrOpFlushState* flushState) {
    for (int i = 0; i < fNumSemaphores; ++i) {
        flushState->gpu()->waitSemaphore(fSemaphores[i].get());
    }
    return true;
}
