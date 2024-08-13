/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrWritePixelsTask_DEFINED
#define GrWritePixelsTask_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrRenderTask.h"

class GrDrawingManager;
class GrOpFlushState;
class GrRecordingContext;
class GrResourceAllocator;
class GrSurfaceProxy;

class GrWritePixelsTask final : public GrRenderTask {
public:
    static sk_sp<GrRenderTask> Make(GrDrawingManager*,
                                    sk_sp<GrSurfaceProxy>,
                                    SkIRect,
                                    GrColorType srcColorType,
                                    GrColorType dstColorType,
                                    const GrMipLevel[],
                                    int levelCount);

private:
    GrWritePixelsTask(GrDrawingManager*,
                      sk_sp<GrSurfaceProxy> dst,
                      SkIRect,
                      GrColorType srcColorType,
                      GrColorType dstColorType,
                      const GrMipLevel[],
                      int levelCount);

    bool onIsUsed(GrSurfaceProxy* proxy) const override { return false; }
    void gatherProxyIntervals(GrResourceAllocator*) const override;
    ExpectedOutcome onMakeClosed(GrRecordingContext*, SkIRect* targetUpdateBounds) override;
    bool onExecute(GrOpFlushState*) override;

#if defined(GPU_TEST_UTILS)
    const char* name() const final { return "WritePixels"; }
#endif
#ifdef SK_DEBUG
    void visitProxies_debugOnly(const GrVisitProxyFunc&) const override {}
#endif

    skia_private::AutoSTArray<16, GrMipLevel> fLevels;
    SkIRect fRect;
    GrColorType fSrcColorType;
    GrColorType fDstColorType;
};

#endif

