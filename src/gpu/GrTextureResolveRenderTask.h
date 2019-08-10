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
    static sk_sp<GrRenderTask> Make(
            sk_sp<GrTextureProxy>, GrTextureResolveFlags, const GrCaps&);

private:
    GrTextureResolveRenderTask(sk_sp<GrTextureProxy> textureProxy, GrTextureResolveFlags flags)
            : GrRenderTask(std::move(textureProxy))
            , fResolveFlags(flags) {
        SkASSERT(GrTextureResolveFlags::kNone != fResolveFlags);
    }

    void onPrepare(GrOpFlushState*) override {}
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        SkASSERT(proxy != fTarget.get());  // This case should be handled by GrRenderTask.
        return false;
    }
    void handleInternalAllocationFailure() override {}
    void gatherProxyIntervals(GrResourceAllocator*) const override;
    bool onExecute(GrOpFlushState*) override;

    const GrTextureResolveFlags fResolveFlags;
};

#endif
