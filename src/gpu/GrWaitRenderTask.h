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
    GrWaitRenderTask(sk_sp<GrSurfaceProxy> proxy, std::unique_ptr<sk_sp<GrSemaphore>[]> semaphores,
                     int numSemaphores)
            : GrRenderTask(std::move(proxy))
            , fSemaphores(std::move(semaphores))
            , fNumSemaphores(numSemaphores){}

private:
    void onPrepare(GrOpFlushState*) override {}
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        SkASSERT(proxy != fTarget.get());  // This case should be handled by GrRenderTask.
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
    void visitProxies_debugOnly(const VisitSurfaceProxyFunc& fn) const override {}
#endif
    std::unique_ptr<sk_sp<GrSemaphore>[]> fSemaphores;
    int fNumSemaphores;
};

#endif
