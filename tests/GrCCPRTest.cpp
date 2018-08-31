/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrClip.h"
#include "GrDrawingManager.h"
#include "GrPathRenderer.h"
#include "GrPaint.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrShape.h"
#include "GrTexture.h"
#include "SkMatrix.h"
#include "SkPathPriv.h"
#include "SkRect.h"
#include "sk_tool_utils.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"
#include "mock/GrMockTypes.h"
#include <cmath>

static constexpr int kCanvasSize = 100;

class CCPRClip : public GrClip {
public:
    CCPRClip(GrCoverageCountingPathRenderer* ccpr, const SkPath& path) : fCCPR(ccpr), fPath(path) {}

private:
    bool apply(GrContext* context, GrRenderTargetContext* rtc, bool, bool, GrAppliedClip* out,
               SkRect* bounds) const override {
        out->addCoverageFP(fCCPR->makeClipProcessor(rtc->priv().testingOnly_getOpListID(), fPath,
                                                    SkIRect::MakeWH(rtc->width(), rtc->height()),
                                                    rtc->width(), rtc->height(),
                                                    *context->contextPriv().caps()));
        return true;
    }
    bool quickContains(const SkRect&) const final { return false; }
    bool isRRect(const SkRect& rtBounds, SkRRect* rr, GrAA*) const final { return false; }
    void getConservativeBounds(int width, int height, SkIRect* rect, bool* iior) const final {
        rect->set(0, 0, width, height);
        if (iior) {
            *iior = false;
        }
    }
    GrCoverageCountingPathRenderer* const fCCPR;
    const SkPath fPath;
};

class CCPRPathDrawer {
public:
    CCPRPathDrawer(GrContext* ctx, skiatest::Reporter* reporter)
            : fCtx(ctx)
            , fCCPR(fCtx->contextPriv().drawingManager()->getCoverageCountingPathRenderer())
            , fRTC(fCtx->contextPriv().makeDeferredRenderTargetContext(
                                                         SkBackingFit::kExact, kCanvasSize,
                                                         kCanvasSize, kRGBA_8888_GrPixelConfig,
                                                         nullptr)) {
        if (!fCCPR) {
            ERRORF(reporter, "ccpr not enabled in GrContext for ccpr tests");
        }
        if (!fRTC) {
            ERRORF(reporter, "failed to create GrRenderTargetContext for ccpr tests");
        }
    }

    GrContext* ctx() const { return fCtx; }
    GrCoverageCountingPathRenderer* ccpr() const { return fCCPR; }

    bool valid() const { return fCCPR && fRTC; }
    void clear() const { fRTC->clear(nullptr, 0, GrRenderTargetContext::CanClearFullscreen::kYes); }
    void abandonGrContext() { fCtx = nullptr; fCCPR = nullptr; fRTC = nullptr; }

    void drawPath(const SkPath& path, const SkMatrix& matrix = SkMatrix::I()) const {
        SkASSERT(this->valid());

        GrPaint paint;
        paint.setColor4f(GrColor4f(0, 1, 0, 1));

        GrNoClip noClip;
        SkIRect clipBounds = SkIRect::MakeWH(kCanvasSize, kCanvasSize);

        GrShape shape(path);

        fCCPR->testingOnly_drawPathDirectly({
                fCtx, std::move(paint), &GrUserStencilSettings::kUnused, fRTC.get(), &noClip,
                &clipBounds, &matrix, &shape, GrAAType::kCoverage, false});
    }

    void clipFullscreenRect(SkPath clipPath, GrColor4f color = GrColor4f(0, 1, 0, 1)) {
        SkASSERT(this->valid());

        GrPaint paint;
        paint.setColor4f(color);

        fRTC->drawRect(CCPRClip(fCCPR, clipPath), std::move(paint), GrAA::kYes, SkMatrix::I(),
                       SkRect::MakeIWH(kCanvasSize, kCanvasSize));
    }

    void flush() const {
        SkASSERT(this->valid());
        fCtx->flush();
    }

private:
    GrContext* fCtx;
    GrCoverageCountingPathRenderer* fCCPR;
    sk_sp<GrRenderTargetContext> fRTC;
};

class CCPRTest {
public:
    void run(skiatest::Reporter* reporter) {
        GrMockOptions mockOptions;
        mockOptions.fInstanceAttribSupport = true;
        mockOptions.fMapBufferFlags = GrCaps::kCanMap_MapFlag;
        mockOptions.fConfigOptions[kAlpha_half_GrPixelConfig].fRenderability =
                GrMockOptions::ConfigOptions::Renderability::kNonMSAA;
        mockOptions.fConfigOptions[kAlpha_half_GrPixelConfig].fTexturable = true;
        mockOptions.fConfigOptions[kAlpha_8_GrPixelConfig].fRenderability =
                GrMockOptions::ConfigOptions::Renderability::kNonMSAA;
        mockOptions.fConfigOptions[kAlpha_8_GrPixelConfig].fTexturable = true;
        mockOptions.fGeometryShaderSupport = true;
        mockOptions.fIntegerSupport = true;
        mockOptions.fFlatInterpolationSupport = true;

        GrContextOptions ctxOptions;
        ctxOptions.fAllowPathMaskCaching = false;
        ctxOptions.fGpuPathRenderers = GpuPathRenderers::kCoverageCounting;

        this->customizeOptions(&mockOptions, &ctxOptions);

        fMockContext = GrContext::MakeMock(&mockOptions, ctxOptions);
        if (!fMockContext) {
            ERRORF(reporter, "could not create mock context");
            return;
        }
        if (!fMockContext->unique()) {
            ERRORF(reporter, "mock context is not unique");
            return;
        }

        CCPRPathDrawer ccpr(fMockContext.get(), reporter);
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

    sk_sp<GrContext> fMockContext;
    SkPath fPath;
};

#define DEF_CCPR_TEST(name)                      \
    DEF_GPUTEST(name, reporter, /* options */) { \
        name test;                               \
        test.run(reporter);                      \
    }

class GrCCPRTest_cleanup : public CCPRTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Ensure paths get unreffed.
        for (int i = 0; i < 10; ++i) {
            ccpr.drawPath(fPath);
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
            ccpr.drawPath(fPath);
            ccpr.clipFullscreenRect(fPath);
        }
        ccpr.abandonGrContext();
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        fMockContext.reset();
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));
    }
};
DEF_CCPR_TEST(GrCCPRTest_cleanup)

class GrCCPRTest_cleanupWithTexAllocFail : public GrCCPRTest_cleanup {
    void customizeOptions(GrMockOptions* mockOptions, GrContextOptions*) override {
        mockOptions->fFailTextureAllocations = true;
    }
};
DEF_CCPR_TEST(GrCCPRTest_cleanupWithTexAllocFail)

class GrCCPRTest_unregisterCulledOps : public CCPRTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Ensure Ops get unregistered from CCPR when culled early.
        ccpr.drawPath(fPath);
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        ccpr.clear(); // Clear should delete the CCPR Op.
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));
        ccpr.flush(); // Should not crash (DrawPathsOp should have unregistered itself).

        // Ensure Op unregisters work when we delete the context without flushing.
        ccpr.drawPath(fPath);
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        ccpr.clear(); // Clear should delete the CCPR DrawPathsOp.
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));
        ccpr.abandonGrContext();
        fMockContext.reset(); // Should not crash (DrawPathsOp should have unregistered itself).
    }
};
DEF_CCPR_TEST(GrCCPRTest_unregisterCulledOps)

class GrCCPRTest_parseEmptyPath : public CCPRTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Make a path large enough that ccpr chooses to crop it by the RT bounds, and ends up with
        // an empty path.
        SkPath largeOutsidePath;
        largeOutsidePath.moveTo(-1e30f, -1e30f);
        largeOutsidePath.lineTo(-1e30f, +1e30f);
        largeOutsidePath.lineTo(-1e10f, +1e30f);
        ccpr.drawPath(largeOutsidePath);

        // Normally an empty path is culled before reaching ccpr, however we use a back door for
        // testing so this path will make it.
        SkPath emptyPath;
        SkASSERT(emptyPath.isEmpty());
        ccpr.drawPath(emptyPath);

        // This is the test. It will exercise various internal asserts and verify we do not crash.
        ccpr.flush();

        // Now try again with clips.
        ccpr.clipFullscreenRect(largeOutsidePath);
        ccpr.clipFullscreenRect(emptyPath);
        ccpr.flush();

        // ... and both.
        ccpr.drawPath(largeOutsidePath);
        ccpr.clipFullscreenRect(largeOutsidePath);
        ccpr.drawPath(emptyPath);
        ccpr.clipFullscreenRect(emptyPath);
        ccpr.flush();
    }
};
DEF_CCPR_TEST(GrCCPRTest_parseEmptyPath)

// This test exercises CCPR's cache capabilities by drawing many paths with two different
// transformation matrices. We then vary the matrices independently by whole and partial pixels,
// and verify the caching behaved as expected.
class GrCCPRTest_cache : public CCPRTest {
    void customizeOptions(GrMockOptions*, GrContextOptions* ctxOptions) override {
        ctxOptions->fAllowPathMaskCaching = true;
    }

    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        static constexpr int kPathSize = 20;
        SkRandom rand;

        SkPath paths[300];
        int primes[11] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31};
        for (size_t i = 0; i < SK_ARRAY_COUNT(paths); ++i) {
            int numPts = rand.nextRangeU(GrShape::kMaxKeyFromDataVerbCnt + 1,
                                         GrShape::kMaxKeyFromDataVerbCnt * 2);
            paths[i] = sk_tool_utils::make_star(SkRect::MakeIWH(kPathSize, kPathSize), numPts,
                                                primes[rand.nextU() % SK_ARRAY_COUNT(primes)]);
        }

        SkMatrix matrices[2] = {
            SkMatrix::MakeTrans(5, 5),
            SkMatrix::MakeTrans(kCanvasSize - kPathSize - 5, kCanvasSize - kPathSize - 5)
        };

        int firstAtlasID = -1;

        for (int iterIdx = 0; iterIdx < 10; ++iterIdx) {
            static constexpr int kNumHitsBeforeStash = 2;
            static const GrUniqueKey gInvalidUniqueKey;

            // Draw all the paths then flush. Repeat until a new stash occurs.
            const GrUniqueKey* stashedAtlasKey = &gInvalidUniqueKey;
            for (int j = 0; j < kNumHitsBeforeStash; ++j) {
                // Nothing should be stashed until its hit count reaches kNumHitsBeforeStash.
                REPORTER_ASSERT(reporter, !stashedAtlasKey->isValid());

                for (size_t i = 0; i < SK_ARRAY_COUNT(paths); ++i) {
                    ccpr.drawPath(paths[i], matrices[i % 2]);
                }
                ccpr.flush();

                stashedAtlasKey = &ccpr.ccpr()->testingOnly_getStashedAtlasKey();
            }

            // Figure out the mock backend ID of the atlas texture stashed away by CCPR.
            GrMockTextureInfo stashedAtlasInfo;
            stashedAtlasInfo.fID = -1;
            if (stashedAtlasKey->isValid()) {
                GrResourceProvider* rp = ccpr.ctx()->contextPriv().resourceProvider();
                sk_sp<GrSurface> stashedAtlas = rp->findByUniqueKey<GrSurface>(*stashedAtlasKey);
                REPORTER_ASSERT(reporter, stashedAtlas);
                if (stashedAtlas) {
                    const auto& backendTexture = stashedAtlas->asTexture()->getBackendTexture();
                    backendTexture.getMockTextureInfo(&stashedAtlasInfo);
                }
            }

            if (0 == iterIdx) {
                // First iteration: just note the ID of the stashed atlas and continue.
                REPORTER_ASSERT(reporter, stashedAtlasKey->isValid());
                firstAtlasID = stashedAtlasInfo.fID;
                continue;
            }

            switch (iterIdx % 3) {
                case 1:
                    // This draw should have gotten 100% cache hits; we only did integer translates
                    // last time (or none if it was the first flush). Therefore, no atlas should
                    // have been stashed away.
                    REPORTER_ASSERT(reporter, !stashedAtlasKey->isValid());

                    // Invalidate even path masks.
                    matrices[0].preTranslate(1.6f, 1.4f);
                    break;

                case 2:
                    // Even path masks were invalidated last iteration by a subpixel translate. They
                    // should have been re-rendered this time and stashed away in the CCPR atlas.
                    REPORTER_ASSERT(reporter, stashedAtlasKey->isValid());

                    // 'firstAtlasID' should be kept as a scratch texture in the resource cache.
                    REPORTER_ASSERT(reporter, stashedAtlasInfo.fID == firstAtlasID);

                    // Invalidate odd path masks.
                    matrices[1].preTranslate(-1.4f, -1.6f);
                    break;

                case 0:
                    // Odd path masks were invalidated last iteration by a subpixel translate. They
                    // should have been re-rendered this time and stashed away in the CCPR atlas.
                    REPORTER_ASSERT(reporter, stashedAtlasKey->isValid());

                    // 'firstAtlasID' is the same texture that got stashed away last time (assuming
                    // no assertion failures). So if it also got stashed this time, it means we
                    // first copied the even paths out of it, then recycled the exact same texture
                    // to render the odd paths. This is the expected behavior.
                    REPORTER_ASSERT(reporter, stashedAtlasInfo.fID == firstAtlasID);

                    // Integer translates: all path masks stay valid.
                    matrices[0].preTranslate(-1, -1);
                    matrices[1].preTranslate(1, 1);
                    break;
            }
        }
    }
};
DEF_CCPR_TEST(GrCCPRTest_cache)

class CCPRRenderingTest {
public:
    void run(skiatest::Reporter* reporter, GrContext* ctx) const {
        if (!ctx->contextPriv().drawingManager()->getCoverageCountingPathRenderer()) {
            return; // CCPR is not enabled on this GPU.
        }
        CCPRPathDrawer ccpr(ctx, reporter);
        if (!ccpr.valid()) {
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
