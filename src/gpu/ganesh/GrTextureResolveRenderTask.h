/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureResolveRenderTask_DEFINED
#define GrTextureResolveRenderTask_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"

class GrDrawingManager;
class GrOpFlushState;
class GrRecordingContext;
class GrResourceAllocator;

class GrTextureResolveRenderTask final : public GrRenderTask {
public:
    GrTextureResolveRenderTask() : GrRenderTask() {}

    void addProxy(GrDrawingManager*, sk_sp<GrSurfaceProxy> proxy,
                  GrSurfaceProxy::ResolveFlags, const GrCaps&);
#if defined(GPU_TEST_UTILS)
    GrSurfaceProxy::ResolveFlags flagsForProxy(sk_sp<GrSurfaceProxy>) const;
#endif

private:
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        return false;
    }
    void gatherProxyIntervals(GrResourceAllocator*) const override;

    ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect*) override {
        return ExpectedOutcome::kTargetUnchanged;
    }

    bool onExecute(GrOpFlushState*) override;

#if defined(GPU_TEST_UTILS)
    const char* name() const final { return "TextureResolve"; }
#endif
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrVisitProxyFunc&) const override;
#endif

    struct Resolve {
        Resolve(GrSurfaceProxy::ResolveFlags flags) : fFlags(flags) {}
        GrSurfaceProxy::ResolveFlags fFlags;
        SkIRect fMSAAResolveRect;
    };

    skia_private::STArray<4, Resolve> fResolves;
};

#endif
