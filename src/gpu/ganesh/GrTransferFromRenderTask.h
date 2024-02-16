/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTransferFromRenderTask_DEFINED
#define GrTransferFromRenderTask_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"

#include <cstddef>
#include <utility>

class GrOpFlushState;
class GrRecordingContext;
class GrResourceAllocator;

class GrTransferFromRenderTask final : public GrRenderTask {
public:
    GrTransferFromRenderTask(sk_sp<GrSurfaceProxy> srcProxy,
                             const SkIRect& srcRect,
                             GrColorType surfaceColorType,
                             GrColorType dstColorType,
                             sk_sp<GrGpuBuffer> dstBuffer,
                             size_t dstOffset)
            : GrRenderTask()
            , fSrcProxy(std::move(srcProxy))
            , fSrcRect(srcRect)
            , fSurfaceColorType(surfaceColorType)
            , fDstColorType(dstColorType)
            , fDstBuffer(std::move(dstBuffer))
            , fDstOffset(dstOffset) {}

private:
    bool onIsUsed(GrSurfaceProxy* proxy) const override {
        SkASSERT(0 == this->numTargets());
        return proxy == fSrcProxy.get();
    }
    void gatherProxyIntervals(GrResourceAllocator*) const override;

    ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect*) override {
        return ExpectedOutcome::kTargetUnchanged;
    }

    bool onExecute(GrOpFlushState*) override;

#if defined(GR_TEST_UTILS)
    const char* name() const final { return "TransferFrom"; }
#endif
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrVisitProxyFunc& func) const override {
        func(fSrcProxy.get(), skgpu::Mipmapped::kNo);
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

