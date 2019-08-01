/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureResolveRenderTask_DEFINED
#define GrTextureResolveRenderTask_DEFINED

#include "src/gpu/GrRenderTask.h"
#include "src/gpu/GrTextureResolveManager.h"

class GrTextureResolveRenderTask final : public GrRenderTask {
public:
    GrTextureResolveRenderTask(
            sk_sp<GrTextureProxy>, GrTextureResolveManager::ResolveFlags, const GrCaps&);

private:
    using ResolveFlags = GrTextureResolveManager::ResolveFlags;

    void onPrepare(GrOpFlushState*) override {}
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        SkASSERT(proxy != fTarget.get());  // This case should be handled by GrRenderTask.
        return false;
    }
    void handleInternalAllocationFailure() override {}
    void gatherProxyIntervals(GrResourceAllocator*) const override;
    bool onExecute(GrOpFlushState*) override;

    const ResolveFlags fResolveFlags;
};

#endif
