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
            : GrRenderTask()
            , fSemaphores(std::move(semaphores))
            , fNumSemaphores(numSemaphores)
            , fWaitedOn(std::move(surfaceView)) {}

private:
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        return proxy == fWaitedOn.proxy();
    }
    void gatherProxyIntervals(GrResourceAllocator*) const override;

    ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect*) override {
        return ExpectedOutcome::kTargetUnchanged;
    }

    bool onExecute(GrOpFlushState*) override;

#if GR_TEST_UTILS
    const char* name() const final { return "Wait"; }
#endif
#ifdef SK_DEBUG
    // No non-dst proxies.
    void visitProxies_debugOnly(const GrVisitProxyFunc&) const override {}
#endif
    std::unique_ptr<std::unique_ptr<GrSemaphore>[]> fSemaphores;
    int fNumSemaphores;

    // This field is separate from the main "targets" field on GrRenderTask because this task
    // does not actually write to the surface and so should not participate in the normal
    // lastRenderTask tracking that written-to targets do.
    GrSurfaceProxyView fWaitedOn;
};

#endif
