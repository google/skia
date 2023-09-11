/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCopyRenderTask_DEFINED
#define GrCopyRenderTask_DEFINED

#include "src/gpu/ganesh/GrRenderTask.h"
#include "src/gpu/ganesh/GrSamplerState.h"

class GrCopyRenderTask final : public GrRenderTask {
public:
    /**
     * Copies pixels from srcRect in src to dstRect in dst. srcRect and dstRect must both be
     * contained in their respective surface dimensions; they do not have to have the same size
     * if the GPU supports scaling and filtering while copying. The src/dst share a common origin.
     */
    static sk_sp<GrRenderTask> Make(GrDrawingManager*,
                                    sk_sp<GrSurfaceProxy> dst,
                                    SkIRect dstRect,
                                    sk_sp<GrSurfaceProxy> src,
                                    SkIRect srcRect,
                                    GrSamplerState::Filter filter,
                                    GrSurfaceOrigin);

private:
    GrCopyRenderTask(GrDrawingManager*,
                     sk_sp<GrSurfaceProxy> dst,
                     SkIRect dstRect,
                     sk_sp<GrSurfaceProxy> src,
                     SkIRect srcRect,
                     GrSamplerState::Filter filter,
                     GrSurfaceOrigin);

    void onMakeSkippable() override { fSrc.reset(); }
    bool onIsUsed(GrSurfaceProxy* proxy) const override { return proxy == fSrc.get(); }
    void gatherProxyIntervals(GrResourceAllocator*) const override;
    ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect* targetUpdateBounds) override;
    bool onExecute(GrOpFlushState*) override;

#if defined(GR_TEST_UTILS)
    const char* name() const final { return "Copy"; }
#endif
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrVisitProxyFunc& func) const override {
        func(fSrc.get(), skgpu::Mipmapped::kNo);
    }
#endif

    sk_sp<GrSurfaceProxy> fSrc;
    SkIRect fSrcRect;
    SkIRect fDstRect;
    GrSamplerState::Filter fFilter;
    GrSurfaceOrigin fOrigin;
};

#endif

