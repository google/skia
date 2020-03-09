/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrWaitRenderTask_DEFINED
#define GrWaitRenderTask_DEFINED

#include "src/gpu/GrRenderTask.h"
#include "src/gpu/GrSemaphore.h"

class GrWaitRenderTask final : public GrRenderTask {
public:
    GrWaitRenderTask(GrSurfaceProxyView surfaceView,
                     std::unique_ptr<std::unique_ptr<GrSemaphore>[]> semaphores,
                     int numSemaphores)
            : GrRenderTask(std::move(surfaceView))
            , fSemaphores(std::move(semaphores))
            , fNumSemaphores(numSemaphores) {}

private:
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        // This case should be handled by GrRenderTask.
        SkASSERT(proxy != fTargetView.proxy());
        return false;
    }
    void handleInternalAllocationFailure() override {}
    void gatherProxyIntervals(GrResourceAllocator*) const override;

    ExpectedOutcome onMakeClosed(const GrCaps&, SkIRect*) override {
        return ExpectedOutcome::kTargetUnchanged;
    }

    bool onExecute(GrOpFlushState*) override;

#ifdef SK_DEBUG
    // No non-dst proxies.
    void visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const override {}
#endif
    std::unique_ptr<std::unique_ptr<GrSemaphore>[]> fSemaphores;
    int fNumSemaphores;
};

#endif
