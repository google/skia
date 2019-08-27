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
    GrTextureResolveRenderTask(sk_sp<GrTextureProxy> textureProxy, GrTextureResolveFlags flags)
            : GrRenderTask(std::move(textureProxy))
            , fResolveFlags(flags) {
        SkASSERT(GrTextureResolveFlags::kNone != fResolveFlags);
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
    void visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const override {}
#endif

    const GrTextureResolveFlags fResolveFlags;
};

#endif
