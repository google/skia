/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureResolveRenderTask_DEFINED
#define GrTextureResolveRenderTask_DEFINED

#include "src/gpu/GrRenderTask.h"

class GrTextureResolveRenderTask final : public GrRenderTask {
public:
    GrTextureResolveRenderTask() : GrRenderTask() {}
    ~GrTextureResolveRenderTask() override;

    void addProxy(sk_sp<GrSurfaceProxy> proxy, GrSurfaceProxy::ResolveFlags, const GrCaps&);

private:
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        // This case should be handled by GrRenderTask.
        SkASSERT(proxy != fTargetView.proxy());
        return false;
    }
    void handleInternalAllocationFailure() override {
        // No need to do anything special here. We just double check the proxies during onExecute.
    }
    void gatherProxyIntervals(GrResourceAllocator*) const override;

    ExpectedOutcome onMakeClosed(const GrCaps&, SkIRect*) override {
        return ExpectedOutcome::kTargetUnchanged;
    }

    bool onExecute(GrOpFlushState*) override;

#ifdef SK_DEBUG
    SkDEBUGCODE(void visitProxies_debugOnly(const GrOp::VisitProxyFunc&) const override;)
#endif

    struct Resolve {
        Resolve(sk_sp<GrSurfaceProxy> proxy, GrSurfaceProxy::ResolveFlags flags)
                : fProxy(std::move(proxy)), fFlags(flags) {}
        sk_sp<GrSurfaceProxy> fProxy;
        GrSurfaceProxy::ResolveFlags fFlags;
        SkIRect fMSAAResolveRect;
    };

    SkSTArray<4, Resolve> fResolves;
};

#endif
