/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBufferUpdateRenderTask_DEFINED
#define GrBufferUpdateRenderTask_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrRenderTask.h"

#include <cstddef>

class GrGpuBuffer;
class GrOpFlushState;
class GrRecordingContext;
class GrResourceAllocator;
class GrSurfaceProxy;
class SkData;
struct SkIRect;

class GrBufferUpdateRenderTask final : public GrRenderTask {
public:
    static sk_sp<GrRenderTask> Make(sk_sp<SkData> src, sk_sp<GrGpuBuffer> dst, size_t dstOffset);

    ~GrBufferUpdateRenderTask() override;

private:
    GrBufferUpdateRenderTask(sk_sp<SkData> src, sk_sp<GrGpuBuffer> dst, size_t dstOffset);

    bool onIsUsed(GrSurfaceProxy* proxy) const override { return false; }
    void gatherProxyIntervals(GrResourceAllocator*) const override {}  // no proxies
    ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect* targetUpdateBounds) override {
        return ExpectedOutcome::kTargetUnchanged;  // no target
    }
    bool onExecute(GrOpFlushState*) override;

#if defined(GPU_TEST_UTILS)
    const char* name() const final { return "BufferUpdate"; }
#endif
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrVisitProxyFunc&) const override {}
#endif

    sk_sp<SkData>      fSrc;
    sk_sp<GrGpuBuffer> fDst;
    size_t             fDstOffset;
};

#endif
