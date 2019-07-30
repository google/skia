/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureResolveOpList_DEFINED
#define GrTextureResolveOpList_DEFINED

#include "src/gpu/GrOpList.h"
#include "src/gpu/GrTextureResolveManager.h"

class GrTextureResolveOpList final : public GrOpList {
    using ResolveFlags = GrTextureResolveManager::ResolveFlags;

public:
    GrTextureResolveOpList(sk_sp<GrOpMemoryPool>, sk_sp<GrTextureProxy>, ResolveFlags,
                           GrAuditTrail*, const GrCaps&);

private:
    void endFlush() override {}
    void onPrepare(GrOpFlushState*) override {}
    void purgeOpsWithUninstantiatedProxies() override {}
    bool copySurface(GrRecordingContext*, GrSurfaceProxy*, GrSurfaceProxy*, const SkIRect&,
                     const SkIPoint&) override {
        SkASSERT(!"Should not call copySurface on GrTextureResolveOpList.");
        return false;
    }
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        SkASSERT(proxy != fTarget.get());  // This case should be handled by GrOpList.
        return false;
    }
    void gatherProxyIntervals(GrResourceAllocator*) const override;
    bool onExecute(GrOpFlushState*) override;

    const ResolveFlags fResolveFlags;
};

#endif
