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
    GrTextureResolveRenderTask(sk_sp<GrSurfaceProxy> proxy,
                               GrSurfaceProxy::ResolveFlags resolveFlags)
            : GrRenderTask(std::move(proxy))
            , fResolveFlags(resolveFlags) {
        // Ensure the last render task that operated on the target is closed. That's where msaa and
        // mipmaps should have been marked dirty.
        SkASSERT(!fTarget->getLastRenderTask() || fTarget->getLastRenderTask()->isClosed());
        SkASSERT(GrSurfaceProxy::ResolveFlags::kNone != fResolveFlags);
    }

    void init(const GrCaps&);

private:
    void onPrepare(GrOpFlushState*) override {}
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        SkASSERT(proxy != fTarget.get());  // This case should be handled by GrRenderTask.
        return false;
    }
    void handleInternalAllocationFailure() override {}
    void gatherProxyIntervals(GrResourceAllocator*) const override;

    ExpectedOutcome onMakeClosed(const GrCaps&) override {
        return ExpectedOutcome::kTargetUnchanged;
    }

    bool onExecute(GrOpFlushState*) override;

#ifdef SK_DEBUG
    // No non-dst proxies.
    void visitProxies_debugOnly(const VisitSurfaceProxyFunc& fn) const override {}
#endif

    const GrSurfaceProxy::ResolveFlags fResolveFlags;
};

#endif
