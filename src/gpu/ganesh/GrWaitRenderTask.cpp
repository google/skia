/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrWaitRenderTask.h"

#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrResourceAllocator.h"

void GrWaitRenderTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // This renderTask doesn't have "normal" ops. In this case we still need to add an interval (so
    // fEndOfOpsTaskOpIndices will remain in sync), so we create a fake op# to capture the fact that
    // we manipulate our target's proxy.
    SkASSERT(0 == this->numTargets());
    auto fakeOp = alloc->curOp();
    alloc->addInterval(fWaitedOn.proxy(), fakeOp, fakeOp,
                       GrResourceAllocator::ActualUse::kYes,
                       GrResourceAllocator::AllowRecycling::kYes);
    alloc->incOps();
}

bool GrWaitRenderTask::onExecute(GrOpFlushState* flushState) {
    for (int i = 0; i < fNumSemaphores; ++i) {
        // If we don't have a semaphore here it means we failed to wrap it. That happens if the
        // client didn't give us a valid semaphore to begin with. Therefore, it is fine to not wait
        // on it.
        if (fSemaphores[i]) {
            flushState->gpu()->waitSemaphore(fSemaphores[i].get());
        }
    }
    return true;
}
