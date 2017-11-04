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
#include "GrDrawingManager.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrShape.h"
#include "GrPathRenderer.h"
#include "GrPaint.h"
#include "SkMakeUnique.h"
#include "SkMatrix.h"
#include "SkPathPriv.h"
#include "SkRect.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"
#include "../tools/gpu/mock/MockTestContext.h"
#include <cmath>

static constexpr int kCanvasSize = 100;

class CCPRPathDrawer {
public:
    CCPRPathDrawer(GrContext* ctx)
            : fCtx(ctx)
            , fCCPR(fCtx->contextPriv().drawingManager()->getCoverageCountingPathRenderer())
            , fRTC(fCtx->makeDeferredRenderTargetContext(SkBackingFit::kExact, kCanvasSize,
                                                         kCanvasSize, kRGBA_8888_GrPixelConfig,
                                                         nullptr)) {
    }

    bool valid() const { return fCCPR && fRTC; }
    void clear() const { fRTC->clear(nullptr, 0, true); }
    void abandonGrContext() { fCtx = nullptr; fCCPR = nullptr; fRTC = nullptr; }

    void drawPath(SkPath path, GrColor4f color = GrColor4f(0, 1, 0, 1)) const {
        SkASSERT(this->valid());

        GrPaint paint;
        paint.setColor4f(color);

        GrNoClip noClip;
        SkIRect clipBounds = SkIRect::MakeWH(kCanvasSize, kCanvasSize);

        SkMatrix matrix = SkMatrix::I();

        path.setIsVolatile(true);
        GrShape shape(path);

        fCCPR->drawPath({fCtx, std::move(paint), &GrUserStencilSettings::kUnused, fRTC.get(),
                         &noClip, &clipBounds, &matrix, &shape, GrAAType::kCoverage, false});
    }

    void flush() const {
        SkASSERT(this->valid());
        fCtx->flush();
    }

private:
    GrContext*                        fCtx;
    GrCoverageCountingPathRenderer*   fCCPR;
    sk_sp<GrRenderTargetContext>      fRTC;
};

class CCPRTest {
public:
    void run(skiatest::Reporter* reporter) {
        std::unique_ptr<sk_gpu_test::TestContext> mockCtx(sk_gpu_test::CreateMockTestContext());
        if (!mockCtx) {
            ERRORF(reporter, "could not create mock test context.");
            return;
        }
        GrContextOptions ctxOptions;
        ctxOptions.fAllowPathMaskCaching = false;
        ctxOptions.fGpuPathRenderers = GpuPathRenderers::kCoverageCounting;
        fGrContext = mockCtx->makeGrContext(ctxOptions);
        if (!fGrContext) {
            ERRORF(reporter, "could not create mock GrContext.");
            return;
        }
        if (!fGrContext->unique()) {
            ERRORF(reporter, "mock GrContext is not unique.");
            return;
        }
        CCPRPathDrawer ccpr(fGrContext.get());
        if (!ccpr.valid()) {
            ERRORF(reporter, "CCPR not supported in mock GrContext");
            return;
        }
        fPath.moveTo(0, 0);
        fPath.cubicTo(50, 50, 0, 50, 50, 0);
        this->onRun(reporter, ccpr);
    }

    virtual ~CCPRTest() {}

protected:
    virtual void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) = 0;

    sk_sp<GrContext>   fGrContext;
    SkPath             fPath;
};

#define DEF_CCPR_TEST(name) \
    DEF_GPUTEST(name, reporter, factory) { \
        name test; \
        test.run(reporter); \
    }

class GrCCPRTest_cleanup : public CCPRTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Normal usage.
        for (int i = 0; i < 10; ++i) {
            ccpr.drawPath(fPath);
        }
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        ccpr.flush();
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Destroy without flush.
        for (int i = 0; i < 10; ++i) {
            ccpr.drawPath(fPath);
        }
        ccpr.abandonGrContext();
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        fGrContext.reset();
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));
    }
};
DEF_CCPR_TEST(GrCCPRTest_cleanup)

class GrCCPRTest_unregisterDeletedOp : public CCPRTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Normal usage.
        ccpr.drawPath(fPath);
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        ccpr.clear(); // Clear should delete the CCPR DrawPathsOp.
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));
        ccpr.flush(); // Should not crash (DrawPathsOp should have unregistered itself).

        // Destroy without flush.
        ccpr.drawPath(fPath);
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        ccpr.clear(); // Clear should delete the CCPR DrawPathsOp.
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));
        ccpr.abandonGrContext();
        fGrContext.reset(); // Should not crash (DrawPathsOp should have unregistered itself).
    }
};
DEF_CCPR_TEST(GrCCPRTest_unregisterDeletedOp)

class CCPRRenderingTest {
public:
    void run(skiatest::Reporter* reporter, GrContext* ctx) const {
        if (!ctx->contextPriv().drawingManager()->getCoverageCountingPathRenderer()) {
            return; // ccpr not enabled.
        }
        CCPRPathDrawer ccpr(ctx);
        if (!ccpr.valid()) {
            ERRORF(reporter, "could not create ccpr rendering context.");
            return;
        }
        this->onRun(reporter, ccpr);
    }

    virtual ~CCPRRenderingTest() {}

protected:
    virtual void onRun(skiatest::Reporter* reporter, const CCPRPathDrawer& ccpr) const = 0;
};

#define DEF_CCPR_RENDERING_TEST(name) \
    DEF_GPUTEST_FOR_RENDERING_CONTEXTS(name, reporter, ctxInfo) { \
        name test; \
        test.run(reporter, ctxInfo.grContext()); \
    }

class GrCCPRTest_busyPath : public CCPRRenderingTest {
    void onRun(skiatest::Reporter* reporter, const CCPRPathDrawer& ccpr) const override {
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
                      // If it does, maybe fiddle with fMaxInstancesPerDrawArraysWithoutCrashing in
                      // your platform's GrGLCaps.
    }
};
DEF_CCPR_RENDERING_TEST(GrCCPRTest_busyPath)

#endif
