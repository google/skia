/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrClip.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrShape.h"
#include "GrPathRenderer.h"
#include "GrPaint.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"
#include <cmath>

static constexpr int kCanvasSize = 100;

class CCPRPathDrawer {
public:
    CCPRPathDrawer(GrContext* ctx)
            : fCtx(ctx)
            , fCCPR(GrCoverageCountingPathRenderer::CreateIfSupported(*fCtx->caps()))
            , fRTC(fCtx->makeDeferredRenderTargetContext(SkBackingFit::kExact, kCanvasSize,
                                                         kCanvasSize, kRGBA_8888_GrPixelConfig,
                                                         nullptr)) {
        if (fCCPR) {
            fCtx->contextPriv().addOnFlushCallbackObject(fCCPR.get());
        }
    }

    ~CCPRPathDrawer() {
        if (fCCPR) {
            fCtx->contextPriv().testingOnly_flushAndRemoveOnFlushCallbackObject(fCCPR.get());
        }
    }

    bool valid() { return fCCPR && fRTC; }

    void clear() { fRTC->clear(nullptr, 0, true); }

    void drawPath(const SkPath& path, GrColor4f color = GrColor4f(0, 1, 0, 1)) {
        GrPaint paint;
        paint.setColor4f(color);
        GrNoClip noClip;
        SkIRect clipBounds = SkIRect::MakeWH(kCanvasSize, kCanvasSize);
        SkMatrix matrix = SkMatrix::I();
        GrShape shape(path);
        fCCPR->drawPath({fCtx, std::move(paint), &GrUserStencilSettings::kUnused, fRTC.get(),
                         &noClip, &clipBounds, &matrix, &shape, GrAAType::kCoverage, false});
    }

    void flush() {
        fCtx->flush();
    }

private:
    GrContext* const                        fCtx;
    sk_sp<GrCoverageCountingPathRenderer>   fCCPR;
    sk_sp<GrRenderTargetContext>            fRTC;
};

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrCCPRTest, reporter, ctxInfo) {
    GrContext* const ctx = ctxInfo.grContext();
    if (!GrCoverageCountingPathRenderer::IsSupported(*ctx->caps())) {
        return;
    }

    CCPRPathDrawer ccpr(ctx);
    if (!ccpr.valid()) {
        ERRORF(reporter, "could not create render target context for ccpr.");
        return;
    }

    // Test very busy paths.
    static constexpr int kNumBusyVerbs = 1 << 17;
    ccpr.clear();
    SkPath busyPath;
    busyPath.moveTo(0, 0); // top left
    busyPath.lineTo(kCanvasSize, kCanvasSize); // bottom right
    for (int i = 2; i < kNumBusyVerbs; ++i) {
        float offset = i * ((float)kCanvasSize / kNumBusyVerbs);
        busyPath.lineTo(kCanvasSize - offset, kCanvasSize + offset); // offscreen
    }
    ccpr.drawPath(busyPath);

    ccpr.flush(); // If this doesn't crash, the test passed.
                  // If it does, maybe fiddle with fMaxInstancesPerDrawArraysWithoutCrashing in your
                  // platform's GrGLCaps.
}

#endif
