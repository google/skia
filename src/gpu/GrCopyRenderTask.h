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
    /**
     * Copies pixels from srcRect in src to SkIRect::MakePtSize(dstPoint, srcRect.dimensions) in
     * dst. The coordinates are absolute pixel coordinates in the proxies' backing stores.
     */
    static sk_sp<GrRenderTask> Make(GrDrawingManager*,
                                    sk_sp<GrSurfaceProxy> src,
                                    SkIRect srcRect,
                                    sk_sp<GrSurfaceProxy> dst,
                                    SkIPoint dstPoint,
                                    const GrCaps*);

private:
    GrCopyRenderTask(GrDrawingManager*,
                     sk_sp<GrSurfaceProxy> src,
                     SkIRect srcRect,
                     sk_sp<GrSurfaceProxy> dst,
                     SkIPoint dstPoint);

    bool onIsUsed(GrSurfaceProxy* proxy) const override { return proxy == fSrc.get(); }
    // If instantiation failed, at flush time we simply will skip doing the copy.
    void handleInternalAllocationFailure() override {}
    void gatherProxyIntervals(GrResourceAllocator*) const override;
    ExpectedOutcome onMakeClosed(const GrCaps&, SkIRect* targetUpdateBounds) override {
        *targetUpdateBounds = SkIRect::MakePtSize(fDstPoint, fSrcRect.size());
        return ExpectedOutcome::kTargetDirty;
    }
    bool onExecute(GrOpFlushState*) override;

#if GR_TEST_UTILS
    const char* name() const final { return "Copy"; }
#endif
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const override {
        fn(fSrc.get(), GrMipmapped::kNo);
    }
#endif

    sk_sp<GrSurfaceProxy> fSrc;
    SkIRect fSrcRect;
    SkIPoint fDstPoint;
};

#endif

