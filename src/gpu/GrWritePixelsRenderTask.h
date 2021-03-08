/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrWritePixelsTask_DEFINED
#define GrWritePixelsTask_DEFINED

#include "src/gpu/GrRenderTask.h"

class GrWritePixelsTask final : public GrRenderTask {
public:
    static sk_sp<GrRenderTask> Make(GrDrawingManager*,
                                    sk_sp<GrSurfaceProxy>,
                                    SkIRect,
                                    GrColorType srcColorType,
                                    GrColorType dstColorType,
                                    const GrMipLevel[],
                                    int levelCount,
                                    sk_sp<SkData> pixelStorage);

private:
    GrWritePixelsTask(GrDrawingManager*,
                      sk_sp<GrSurfaceProxy> dst,
                      SkIRect,
                      GrColorType srcColorType,
                      GrColorType dstColorType,
                      const GrMipLevel[],
                      int levelCount,
                      sk_sp<SkData> pixelStorage);

    bool onIsUsed(GrSurfaceProxy* proxy) const override { return false; }
    void gatherProxyIntervals(GrResourceAllocator*) const override;
    ExpectedOutcome onMakeClosed(const GrCaps&, SkIRect* targetUpdateBounds) override;
    bool onExecute(GrOpFlushState*) override;

#if GR_TEST_UTILS
    const char* name() const final { return "WritePixels"; }
#endif
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrOp::VisitProxyFunc& fn) const override {}
#endif

    SkIRect fRect;
    GrColorType fSrcColorType;
    GrColorType fDstColorType;
    SkAutoSTArray<16, GrMipLevel> fLevels;
    sk_sp<SkData> fStorage;
};

#endif

