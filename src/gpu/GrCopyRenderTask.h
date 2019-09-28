/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCopyRenderTask_DEFINED
#define GrCopyRenderTask_DEFINED

#include "src/gpu/GrRenderTask.h"

class GrCopyRenderTask final : public GrRenderTask {
public:
    static sk_sp<GrRenderTask> Make(sk_sp<GrSurfaceProxy> srcProxy,
                                    const SkIRect& srcRect,
                                    sk_sp<GrSurfaceProxy> dstProxy,
                                    const SkIPoint& dstPoint);

private:
    GrCopyRenderTask(sk_sp<GrSurfaceProxy> srcProxy,
                     const SkIRect& srcRect,
                     sk_sp<GrSurfaceProxy> dstProxy,
                     const SkIPoint& dstPoint);

    void onPrepare(GrOpFlushState*) override {}
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        SkASSERT(proxy != fTarget.get());  // This case should be handled by GrRenderTask.
        return proxy == fSrcProxy.get();
    }
    // If instantiation failed, at flush time we simply will skip doing the copy.
    void handleInternalAllocationFailure() override {}
    void gatherProxyIntervals(GrResourceAllocator*) const override;
    ExpectedOutcome onMakeClosed(const GrCaps&) override {
        return ExpectedOutcome::kTargetDirty;
    }
    bool onExecute(GrOpFlushState*) override;

#ifdef SK_DEBUG
    void visitProxies_debugOnly(const VisitSurfaceProxyFunc& fn) const override {
        fn(fSrcProxy.get(), GrMipMapped::kNo);
    }
#endif

    sk_sp<GrSurfaceProxy> fSrcProxy;
    SkIRect fSrcRect;
    SkIPoint fDstPoint;
};

#endif

