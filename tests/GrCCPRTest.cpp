/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tests/Test.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "tools/ToolUtils.h"

#include <cmath>

static constexpr int kCanvasSize = 100;

class CCPRClip : public GrClip {
public:
    CCPRClip(GrCoverageCountingPathRenderer* ccpr, const SkPath& path) : fCCPR(ccpr), fPath(path) {}

private:
    SkIRect getConservativeBounds() const final { return fPath.getBounds().roundOut(); }
    Effect apply(GrRecordingContext* context, GrSurfaceDrawContext* rtc, GrAAType,
                 bool hasUserStencilSettings, GrAppliedClip* out,
                 SkRect* bounds) const override {
        out->addCoverageFP(fCCPR->makeClipProcessor(
                /*inputFP=*/nullptr, rtc->getOpsTask()->uniqueID(), fPath,
                SkIRect::MakeWH(rtc->width(), rtc->height()), *context->priv().caps()));
        return Effect::kClipped;
    }

    GrCoverageCountingPathRenderer* const fCCPR;
    const SkPath fPath;
};

class CCPRPathDrawer {
public:
    CCPRPathDrawer(sk_sp<GrDirectContext> dContext, skiatest::Reporter* reporter)
            : fDContext(dContext)
            , fCCPR(fDContext->priv().drawingManager()->getCoverageCountingPathRenderer())
            , fRTC(GrSurfaceDrawContext::Make(
                      fDContext.get(), GrColorType::kRGBA_8888, nullptr, SkBackingFit::kExact,
                      {kCanvasSize, kCanvasSize})) {
        if (!fCCPR) {
            ERRORF(reporter, "ccpr not enabled in GrDirectContext for ccpr tests");
        }
        if (!fRTC) {
            ERRORF(reporter, "failed to create GrSurfaceDrawContext for ccpr tests");
        }
    }

    GrDirectContext* dContext() const { return fDContext.get(); }
    GrCoverageCountingPathRenderer* ccpr() const { return fCCPR; }

    bool valid() const { return fCCPR && fRTC; }
    void clear() const { fRTC->clear(SK_PMColor4fTRANSPARENT); }
    void destroyGrContext() {
        SkASSERT(fDContext->unique());
        fRTC.reset();
        fCCPR = nullptr;
        fDContext.reset();
    }

    void clipFullscreenRect(SkPath clipPath, const SkMatrix& matrix = SkMatrix::I()) const {
        SkASSERT(this->valid());

        GrPaint paint;
        paint.setColor4f({0, 1, 0, 1});

        CCPRClip clip(fCCPR, clipPath);
        fRTC->drawRect(&clip, std::move(paint), GrAA::kYes, SkMatrix::I(),
                       SkRect::MakeIWH(kCanvasSize, kCanvasSize));
    }

    void flush() const {
        SkASSERT(this->valid());
        fDContext->flushAndSubmit();
    }

private:
    sk_sp<GrDirectContext> fDContext;
    GrCoverageCountingPathRenderer* fCCPR;
    std::unique_ptr<GrSurfaceDrawContext> fRTC;
};

class CCPRTest {
public:
    void run(skiatest::Reporter* reporter) {
        GrMockOptions mockOptions;
        mockOptions.fDrawInstancedSupport = true;
        mockOptions.fHalfFloatVertexAttributeSupport = true;
        mockOptions.fMapBufferFlags = GrCaps::kCanMap_MapFlag;
        mockOptions.fConfigOptions[(int)GrColorType::kAlpha_F16].fRenderability =
                GrMockOptions::ConfigOptions::Renderability::kNonMSAA;
        mockOptions.fConfigOptions[(int)GrColorType::kAlpha_F16].fTexturable = true;
        mockOptions.fConfigOptions[(int)GrColorType::kAlpha_8].fRenderability =
                GrMockOptions::ConfigOptions::Renderability::kMSAA;
        mockOptions.fConfigOptions[(int)GrColorType::kAlpha_8].fTexturable = true;
        mockOptions.fGeometryShaderSupport = true;
        mockOptions.fIntegerSupport = true;
        mockOptions.fFlatInterpolationSupport = true;

        GrContextOptions ctxOptions;
        ctxOptions.fAllowPathMaskCaching = false;
        ctxOptions.fGpuPathRenderers = GpuPathRenderers::kCoverageCounting;

        this->customizeOptions(&mockOptions, &ctxOptions);

        sk_sp<GrDirectContext> mockContext = GrDirectContext::MakeMock(&mockOptions, ctxOptions);
        if (!mockContext) {
            ERRORF(reporter, "could not create mock context");
            return;
        }
        if (!mockContext->unique()) {
            ERRORF(reporter, "mock context is not unique");
            return;
        }

        CCPRPathDrawer ccpr(std::exchange(mockContext, nullptr), reporter);
        if (!ccpr.valid()) {
            return;
        }

        fPath.moveTo(0, 0);
        fPath.cubicTo(50, 50, 0, 50, 50, 0);
        this->onRun(reporter, ccpr);
    }

    virtual ~CCPRTest() {}

protected:
    virtual void customizeOptions(GrMockOptions*, GrContextOptions*) {}
    virtual void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) = 0;

    SkPath fPath;
};

#define DEF_CCPR_TEST(name) \
    DEF_GPUTEST(name, reporter, /* options */) { \
        name test; \
        test.run(reporter); \
    }

class CCPR_cleanup : public CCPRTest {
protected:
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Ensure paths get unreffed.
        for (int i = 0; i < 10; ++i) {
            ccpr.clipFullscreenRect(fPath);
        }
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        ccpr.flush();
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Ensure clip paths get unreffed.
        for (int i = 0; i < 10; ++i) {
            ccpr.clipFullscreenRect(fPath);
        }
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        ccpr.flush();
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Ensure paths get unreffed when we delete the context without flushing.
        for (int i = 0; i < 10; ++i) {
            ccpr.clipFullscreenRect(fPath);
            ccpr.clipFullscreenRect(fPath);
        }
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));

        ccpr.destroyGrContext();
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));
    }
};
DEF_CCPR_TEST(CCPR_cleanup)

class CCPR_cleanupWithTexAllocFail : public CCPR_cleanup {
    void customizeOptions(GrMockOptions* mockOptions, GrContextOptions*) override {
        mockOptions->fFailTextureAllocations = true;
    }
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        ((GrRecordingContext*)ccpr.dContext())->priv().incrSuppressWarningMessages();
        this->CCPR_cleanup::onRun(reporter, ccpr);
    }
};
DEF_CCPR_TEST(CCPR_cleanupWithTexAllocFail)

class CCPR_parseEmptyPath : public CCPRTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Make a path large enough that ccpr chooses to crop it by the RT bounds, and ends up with
        // an empty path.
        SkPath largeOutsidePath = SkPath::Polygon({
            {-1e30f, -1e30f},
            {-1e30f, +1e30f},
            {-1e10f, +1e30f},
        }, false);
        ccpr.clipFullscreenRect(largeOutsidePath);

        // Normally an empty path is culled before reaching ccpr, however we use a back door for
        // testing so this path will make it.
        SkPath emptyPath;
        SkASSERT(emptyPath.isEmpty());
        ccpr.clipFullscreenRect(emptyPath);

        // This is the test. It will exercise various internal asserts and verify we do not crash.
        ccpr.flush();

        // Now try again with clips.
        ccpr.clipFullscreenRect(largeOutsidePath);
        ccpr.clipFullscreenRect(emptyPath);
        ccpr.flush();

        // ... and both.
        ccpr.clipFullscreenRect(largeOutsidePath);
        ccpr.clipFullscreenRect(largeOutsidePath);
        ccpr.clipFullscreenRect(emptyPath);
        ccpr.clipFullscreenRect(emptyPath);
        ccpr.flush();
    }
};
DEF_CCPR_TEST(CCPR_parseEmptyPath)

// This test doesn't currently work with clips. Forcing unnatural early deletion of the pending
// paths triggers an assert.
#if 0
class CCPR_unrefPerOpsTaskPathsBeforeOps : public CCPRTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));
        for (int i = 0; i < 10000; ++i) {
            // Draw enough paths to make the arena allocator hit the heap.
            ccpr.clipFullscreenRect(fPath);
        }

        // Unref the GrCCPerOpsTaskPaths object.
        auto perOpsTaskPathsMap = ccpr.ccpr()->detachPendingPaths();
        perOpsTaskPathsMap.clear();

        // Now delete the Op and all its draws.
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        ccpr.flush();
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));
    }
};
DEF_CCPR_TEST(CCPR_unrefPerOpsTaskPathsBeforeOps)
#endif

class CCPRRenderingTest {
public:
    void run(skiatest::Reporter* reporter, GrDirectContext* dContext) const {
        if (dContext->priv().drawingManager()->getCoverageCountingPathRenderer()) {
            CCPRPathDrawer drawer(sk_ref_sp(dContext), reporter);
            if (!drawer.valid()) {
                return;
            }
            this->onRun(reporter, drawer);
        }
    }

    virtual ~CCPRRenderingTest() {}

protected:
    virtual void onRun(skiatest::Reporter* reporter, const CCPRPathDrawer& ccpr) const = 0;
};

#define DEF_CCPR_RENDERING_TEST(name) \
    DEF_GPUTEST_FOR_RENDERING_CONTEXTS(name, reporter, ctxInfo) { \
        name test; \
        test.run(reporter, ctxInfo.directContext()); \
    }

class CCPR_busyPath : public CCPRRenderingTest {
    void onRun(skiatest::Reporter* reporter, const CCPRPathDrawer& ccpr) const override {
        static constexpr int kNumBusyVerbs = 1 << 17;
        ccpr.clear();
        SkPathBuilder busyPath;
        busyPath.moveTo(0, 0); // top left
        busyPath.lineTo(kCanvasSize, kCanvasSize); // bottom right
        for (int i = 2; i < kNumBusyVerbs; ++i) {
            float offset = i * ((float)kCanvasSize / kNumBusyVerbs);
            busyPath.lineTo(kCanvasSize - offset, kCanvasSize + offset); // offscreen
        }
        ccpr.clipFullscreenRect(busyPath.detach());

        ccpr.flush(); // If this doesn't crash, the test passed.
                      // If it does, maybe fiddle with fMaxInstancesPerDrawArraysWithoutCrashing in
                      // your platform's GrGLCaps.
    }
};
DEF_CCPR_RENDERING_TEST(CCPR_busyPath)

// https://bugs.chromium.org/p/chromium/issues/detail?id=1102117
class CCPR_evictCacheEntryForPendingDrawOp : public CCPRRenderingTest {
    void onRun(skiatest::Reporter* reporter, const CCPRPathDrawer& ccpr) const override {
        static constexpr SkRect kRect = SkRect::MakeWH(50, 50);
        ccpr.clear();

        // make sure path is cached.
        for (int i = 0; i < 2; i++) {
            SkPath path;
            path.addRect(kRect);

            ccpr.clipFullscreenRect(path);
            ccpr.flush();
        }

        // make enough cached draws to make DoCopies happen.
        for (int i = 0; i <= GrCoverageCountingPathRenderer::kDoCopiesThreshold; i++) {
            SkPath path;
            path.addRect(kRect);
            ccpr.clipFullscreenRect(path);
        }

        // now draw the path in an incompatible matrix. Previous draw's cached atlas should
        // not be invalidated. otherwise, this flush would render more paths than allocated for.
        auto m = SkMatrix::Translate(0.1f, 0.1f);
        SkPath path;
        path.addRect(kRect);
        ccpr.clipFullscreenRect(path, m);
        ccpr.flush();

        // if this test does not crash, it is passed.
    }
};
DEF_CCPR_RENDERING_TEST(CCPR_evictCacheEntryForPendingDrawOp)
