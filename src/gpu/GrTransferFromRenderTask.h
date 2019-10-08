/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTransferFromRenderTask_DEFINED
#define GrTransferFromRenderTask_DEFINED

#include "src/gpu/GrRenderTask.h"

class GrTransferFromRenderTask final : public GrRenderTask {
public:
    GrTransferFromRenderTask(sk_sp<GrSurfaceProxy> srcProxy,
                             const SkIRect& srcRect,
                             GrColorType surfaceColorType,
                             GrColorType dstColorType,
                             sk_sp<GrGpuBuffer> dstBuffer,
                             size_t dstOffset)
            : GrRenderTask(nullptr)
            , fSrcProxy(std::move(srcProxy))
            , fSrcRect(srcRect)
            , fSurfaceColorType(surfaceColorType)
            , fDstColorType(dstColorType)
            , fDstBuffer(std::move(dstBuffer))
            , fDstOffset(dstOffset) {}

private:
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        SkASSERT(!fTarget);
        return proxy == fSrcProxy.get();
    }
    // If fSrcProxy is uninstantiated at flush time we simply will skip doing the transfer.
    void handleInternalAllocationFailure() override {}
    void gatherProxyIntervals(GrResourceAllocator*) const override;

    ExpectedOutcome onMakeClosed(const GrCaps&, SkIRect*) override {
        return ExpectedOutcome::kTargetUnchanged;
    }

    bool onExecute(GrOpFlushState*) override;

#ifdef SK_DEBUG
    void visitProxies_debugOnly(const VisitSurfaceProxyFunc& fn) const override {
        fn(fSrcProxy.get(), GrMipMapped::kNo);
    }
#endif

    sk_sp<GrSurfaceProxy> fSrcProxy;
    SkIRect fSrcRect;
    GrColorType fSurfaceColorType;
    GrColorType fDstColorType;
    sk_sp<GrGpuBuffer> fDstBuffer;
    size_t fDstOffset;

};

#endif

