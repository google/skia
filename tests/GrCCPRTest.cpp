/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tests/Test.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/gpu/GrTexture.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkExchange.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/ccpr/GrCCPathCache.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/geometry/GrShape.h"
#include "tools/ToolUtils.h"

#include <cmath>

static constexpr int kCanvasSize = 100;

enum class DoCoverageCount { kNo = false, kYes };
enum class DoStroke { kNo = false, kYes };

class CCPRClip : public GrClip {
public:
    CCPRClip(GrCoverageCountingPathRenderer* ccpr, const SkPath& path) : fCCPR(ccpr), fPath(path) {}

private:
    bool apply(GrRecordingContext* context, GrRenderTargetContext* rtc, bool useHWAA,
               bool hasUserStencilSettings, GrAppliedClip* out, SkRect* bounds) const override {
        out->addCoverageFP(fCCPR->makeClipProcessor(rtc->priv().testingOnly_getOpsTaskID(), fPath,
                                                    SkIRect::MakeWH(rtc->width(), rtc->height()),
                                                    *context->priv().caps()));
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
    CCPRPathDrawer(sk_sp<GrContext> ctx, skiatest::Reporter* reporter, DoStroke doStroke)
            : fCtx(ctx)
            , fCCPR(fCtx->priv().drawingManager()->getCoverageCountingPathRenderer())
            , fRTC(fCtx->priv().makeDeferredRenderTargetContext(
                      SkBackingFit::kExact, kCanvasSize, kCanvasSize, GrColorType::kRGBA_8888,
                      nullptr))
            , fDoStroke(DoStroke::kYes == doStroke) {
        if (!fCCPR) {
            ERRORF(reporter, "ccpr not enabled in GrContext for ccpr tests");
        }
        if (!fRTC) {
            ERRORF(reporter, "failed to create GrRenderTargetContext for ccpr tests");
        }
    }

    GrContext* ctx() const { return fCtx.get(); }
    GrCoverageCountingPathRenderer* ccpr() const { return fCCPR; }

    bool valid() const { return fCCPR && fRTC; }
    void clear() const { fRTC->clear(nullptr, SK_PMColor4fTRANSPARENT,
                                     GrRenderTargetContext::CanClearFullscreen::kYes); }
    void destroyGrContext() {
        SkASSERT(fCtx->unique());
        fRTC.reset();
        fCCPR = nullptr;
        fCtx.reset();
    }

    void drawPath(const SkPath& path, const SkMatrix& matrix = SkMatrix::I()) const {
        SkASSERT(this->valid());

        GrPaint paint;
        paint.setColor4f({ 0, 1, 0, 1 });

        GrNoClip noClip;
        SkIRect clipBounds = SkIRect::MakeWH(kCanvasSize, kCanvasSize);

        GrShape shape;
        if (!fDoStroke) {
            shape = GrShape(path);
        } else {
            // Use hairlines for now, since they are the only stroke type that doesn't require a
            // rigid-body transform. The CCPR stroke code makes no distinction between hairlines
            // and regular strokes other than how it decides the device-space stroke width.
            SkStrokeRec stroke(SkStrokeRec::kHairline_InitStyle);
            stroke.setStrokeParams(SkPaint::kRound_Cap, SkPaint::kMiter_Join, 4);
            shape = GrShape(path, GrStyle(stroke, nullptr));
        }

        fCCPR->testingOnly_drawPathDirectly({
                fCtx.get(), std::move(paint), &GrUserStencilSettings::kUnused, fRTC.get(), &noClip,
                &clipBounds, &matrix, &shape, GrAAType::kCoverage, false});
    }

    void clipFullscreenRect(SkPath clipPath, SkPMColor4f color = { 0, 1, 0, 1 }) {
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
    sk_sp<GrContext> fCtx;
    GrCoverageCountingPathRenderer* fCCPR;
    std::unique_ptr<GrRenderTargetContext> fRTC;
    const bool fDoStroke;
};

class CCPRTest {
public:
    void run(skiatest::Reporter* reporter, DoCoverageCount doCoverageCount, DoStroke doStroke) {
        GrMockOptions mockOptions;
        mockOptions.fInstanceAttribSupport = true;
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
        ctxOptions.fDisableCoverageCountingPaths = (DoCoverageCount::kNo == doCoverageCount);
        ctxOptions.fAllowPathMaskCaching = false;
        ctxOptions.fGpuPathRenderers = GpuPathRenderers::kCoverageCounting;

        this->customizeOptions(&mockOptions, &ctxOptions);

        sk_sp<GrContext> mockContext = GrContext::MakeMock(&mockOptions, ctxOptions);
        if (!mockContext) {
            ERRORF(reporter, "could not create mock context");
            return;
        }
        if (!mockContext->unique()) {
            ERRORF(reporter, "mock context is not unique");
            return;
        }

        CCPRPathDrawer ccpr(skstd::exchange(mockContext, nullptr), reporter, doStroke);
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
        test.run(reporter, DoCoverageCount::kYes, DoStroke::kNo); \
        test.run(reporter, DoCoverageCount::kYes, DoStroke::kYes); \
        test.run(reporter, DoCoverageCount::kNo, DoStroke::kNo); \
        /* FIXME: test.run(reporter, (DoCoverageCount::kNo, DoStroke::kYes) once supported. */ \
    }

class CCPR_cleanup : public CCPRTest {
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
};
DEF_CCPR_TEST(CCPR_cleanupWithTexAllocFail)

class CCPR_unregisterCulledOps : public CCPRTest {
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
        ccpr.destroyGrContext(); // Should not crash (DrawPathsOp should have unregistered itself).
    }
};
DEF_CCPR_TEST(CCPR_unregisterCulledOps)

class CCPR_parseEmptyPath : public CCPRTest {
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
DEF_CCPR_TEST(CCPR_parseEmptyPath)

static int get_mock_texture_id(const GrTexture* texture) {
    const GrBackendTexture& backingTexture = texture->getBackendTexture();
    SkASSERT(GrBackendApi::kMock == backingTexture.backend());

    if (!backingTexture.isValid()) {
        return 0;
    }

    GrMockTextureInfo info;
    backingTexture.getMockTextureInfo(&info);
    return info.fID;
}

// Base class for cache path unit tests.
class CCPRCacheTest : public CCPRTest {
protected:
    // Registers as an onFlush callback in order to snag the CCPR per-flush resources and note the
    // texture IDs.
    class RecordLastMockAtlasIDs : public GrOnFlushCallbackObject {
    public:
        RecordLastMockAtlasIDs(sk_sp<GrCoverageCountingPathRenderer> ccpr) : fCCPR(ccpr) {}

        int lastCopyAtlasID() const { return fLastCopyAtlasID; }
        int lastRenderedAtlasID() const { return fLastRenderedAtlasID; }

        void preFlush(GrOnFlushResourceProvider*, const uint32_t* opsTaskIDs, int numOpsTaskIDs,
                      SkTArray<std::unique_ptr<GrRenderTargetContext>>* out) override {
            fLastRenderedAtlasID = fLastCopyAtlasID = 0;

            const GrCCPerFlushResources* resources = fCCPR->testingOnly_getCurrentFlushResources();
            if (!resources) {
                return;
            }

            if (const GrTexture* tex = resources->testingOnly_frontCopyAtlasTexture()) {
                fLastCopyAtlasID = get_mock_texture_id(tex);
            }
            if (const GrTexture* tex = resources->testingOnly_frontRenderedAtlasTexture()) {
                fLastRenderedAtlasID = get_mock_texture_id(tex);
            }
        }

        void postFlush(GrDeferredUploadToken, const uint32_t*, int) override {}

    private:
        sk_sp<GrCoverageCountingPathRenderer> fCCPR;
        int fLastCopyAtlasID = 0;
        int fLastRenderedAtlasID = 0;
    };

    CCPRCacheTest() {
        static constexpr int primes[11] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31};

        SkRandom rand;
        for (size_t i = 0; i < SK_ARRAY_COUNT(fPaths); ++i) {
            int numPts = rand.nextRangeU(GrShape::kMaxKeyFromDataVerbCnt + 1,
                                         GrShape::kMaxKeyFromDataVerbCnt * 2);
            int step;
            do {
                step = primes[rand.nextU() % SK_ARRAY_COUNT(primes)];
            } while (step == numPts);
            fPaths[i] = ToolUtils::make_star(SkRect::MakeLTRB(0, 0, 1, 1), numPts, step);
        }
    }

    void drawPathsAndFlush(CCPRPathDrawer& ccpr, const SkMatrix& m) {
        this->drawPathsAndFlush(ccpr, &m, 1);
    }
    void drawPathsAndFlush(CCPRPathDrawer& ccpr, const SkMatrix* matrices, int numMatrices) {
        // Draw all the paths.
        for (size_t i = 0; i < SK_ARRAY_COUNT(fPaths); ++i) {
            ccpr.drawPath(fPaths[i], matrices[i % numMatrices]);
        }
        // Re-draw a few paths, to test the case where a cache entry is hit more than once in a
        // single flush.
        SkRandom rand;
        int duplicateIndices[10];
        for (size_t i = 0; i < SK_ARRAY_COUNT(duplicateIndices); ++i) {
            duplicateIndices[i] = rand.nextULessThan(SK_ARRAY_COUNT(fPaths));
        }
        for (size_t i = 0; i < SK_ARRAY_COUNT(duplicateIndices); ++i) {
            for (size_t j = 0; j <= i; ++j) {
                int idx = duplicateIndices[j];
                ccpr.drawPath(fPaths[idx], matrices[idx % numMatrices]);
            }
        }
        ccpr.flush();
    }

private:
    void customizeOptions(GrMockOptions*, GrContextOptions* ctxOptions) override {
        ctxOptions->fAllowPathMaskCaching = true;
    }

    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) final {
        RecordLastMockAtlasIDs atlasIDRecorder(sk_ref_sp(ccpr.ccpr()));
        ccpr.ctx()->priv().addOnFlushCallbackObject(&atlasIDRecorder);

        this->onRun(reporter, ccpr, atlasIDRecorder);

        ccpr.ctx()->priv().testingOnly_flushAndRemoveOnFlushCallbackObject(&atlasIDRecorder);
    }

    virtual void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr,
                       const RecordLastMockAtlasIDs&) = 0;

protected:
    SkPath fPaths[350];
};

// Ensures ccpr always reuses the same atlas texture in the animation use case.
class CCPR_cache_animationAtlasReuse : public CCPRCacheTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr,
               const RecordLastMockAtlasIDs& atlasIDRecorder) override {
        SkMatrix m = SkMatrix::MakeTrans(kCanvasSize/2, kCanvasSize/2);
        m.preScale(80, 80);
        m.preTranslate(-.5,-.5);
        this->drawPathsAndFlush(ccpr, m);

        REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
        REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastRenderedAtlasID());
        const int atlasID = atlasIDRecorder.lastRenderedAtlasID();

        // Ensures we always reuse the same atlas texture in the animation use case.
        for (int i = 0; i < 12; ++i) {
            // 59 is prime, so we will hit every integer modulo 360 before repeating.
            m.preRotate(59, .5, .5);

            // Go twice. Paths have to get drawn twice with the same matrix before we cache their
            // atlas. This makes sure that on the subsequent draw, after an atlas has been cached
            // and is then invalidated since the matrix will change, that the same underlying
            // texture object is still reused for the next atlas.
            for (int j = 0; j < 2; ++j) {
                this->drawPathsAndFlush(ccpr, m);
                // Nothing should be copied to an 8-bit atlas after just two draws.
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
                REPORTER_ASSERT(reporter, atlasIDRecorder.lastRenderedAtlasID() == atlasID);
            }
        }

        // Do the last draw again. (On draw 3 they should get copied to an 8-bit atlas.)
        this->drawPathsAndFlush(ccpr, m);
        REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastCopyAtlasID());
        REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());

        // Now double-check that everything continues to hit the cache as expected when the matrix
        // doesn't change.
        for (int i = 0; i < 10; ++i) {
            this->drawPathsAndFlush(ccpr, m);
            REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
            REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());
        }
    }
};
DEF_CCPR_TEST(CCPR_cache_animationAtlasReuse)

class CCPR_cache_recycleEntries : public CCPRCacheTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr,
               const RecordLastMockAtlasIDs& atlasIDRecorder) override {
        SkMatrix m = SkMatrix::MakeTrans(kCanvasSize/2, kCanvasSize/2);
        m.preScale(80, 80);
        m.preTranslate(-.5,-.5);

        auto cache = ccpr.ccpr()->testingOnly_getPathCache();
        REPORTER_ASSERT(reporter, cache);

        const auto& lru = cache->testingOnly_getLRU();

        SkTArray<const void*> expectedPtrs;

        // Ensures we always reuse the same atlas texture in the animation use case.
        for (int i = 0; i < 5; ++i) {
            // 59 is prime, so we will hit every integer modulo 360 before repeating.
            m.preRotate(59, .5, .5);

            // Go twice. Paths have to get drawn twice with the same matrix before we cache their
            // atlas.
            for (int j = 0; j < 2; ++j) {
                this->drawPathsAndFlush(ccpr, m);
                // Nothing should be copied to an 8-bit atlas after just two draws.
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
                REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastRenderedAtlasID());
            }

            int idx = 0;
            for (const GrCCPathCacheEntry* entry : lru) {
                if (0 == i) {
                    expectedPtrs.push_back(entry);
                } else {
                    // The same pointer should have been recycled for the new matrix.
                    REPORTER_ASSERT(reporter, entry == expectedPtrs[idx]);
                }
                ++idx;
            }
        }
    }
};
DEF_CCPR_TEST(CCPR_cache_recycleEntries)

// Ensures mostly-visible paths get their full mask cached.
class CCPR_cache_mostlyVisible : public CCPRCacheTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr,
               const RecordLastMockAtlasIDs& atlasIDRecorder) override {
        SkMatrix matrices[3] = {
            SkMatrix::MakeScale(kCanvasSize/2, kCanvasSize/2), // Fully visible.
            SkMatrix::MakeScale(kCanvasSize * 1.25, kCanvasSize * 1.25), // Mostly visible.
            SkMatrix::MakeScale(kCanvasSize * 1.5, kCanvasSize * 1.5), // Mostly NOT visible.
        };

        for (int i = 0; i < 10; ++i) {
            this->drawPathsAndFlush(ccpr, matrices, 3);
            if (2 == i) {
                // The mostly-visible paths should still get cached.
                REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastCopyAtlasID());
            } else {
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
            }
            // Ensure mostly NOT-visible paths never get cached.
            REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastRenderedAtlasID());
        }

        // Clear the path cache.
        this->drawPathsAndFlush(ccpr, SkMatrix::I());

        // Now only draw the fully/mostly visible ones.
        for (int i = 0; i < 2; ++i) {
            this->drawPathsAndFlush(ccpr, matrices, 2);
            REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
            REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastRenderedAtlasID());
        }

        // On draw 3 they should get copied to an 8-bit atlas.
        this->drawPathsAndFlush(ccpr, matrices, 2);
        REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastCopyAtlasID());
        REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());

        for (int i = 0; i < 10; ++i) {
            this->drawPathsAndFlush(ccpr, matrices, 2);
            REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
            REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());
        }

        // Draw a different part of the path to ensure the full mask was cached.
        matrices[1].postTranslate(SkScalarFloorToInt(kCanvasSize * -.25f),
                                  SkScalarFloorToInt(kCanvasSize * -.25f));
        for (int i = 0; i < 10; ++i) {
            this->drawPathsAndFlush(ccpr, matrices, 2);
            REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
            REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());
        }
    }
};
DEF_CCPR_TEST(CCPR_cache_mostlyVisible)

// Ensures GrContext::performDeferredCleanup works.
class CCPR_cache_deferredCleanup : public CCPRCacheTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr,
               const RecordLastMockAtlasIDs& atlasIDRecorder) override {
        SkMatrix m = SkMatrix::MakeScale(20, 20);
        int lastRenderedAtlasID = 0;

        for (int i = 0; i < 5; ++i) {
            this->drawPathsAndFlush(ccpr, m);
            REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
            REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastRenderedAtlasID());
            int renderedAtlasID = atlasIDRecorder.lastRenderedAtlasID();
            REPORTER_ASSERT(reporter, renderedAtlasID != lastRenderedAtlasID);
            lastRenderedAtlasID = renderedAtlasID;

            this->drawPathsAndFlush(ccpr, m);
            REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
            REPORTER_ASSERT(reporter, lastRenderedAtlasID == atlasIDRecorder.lastRenderedAtlasID());

            // On draw 3 they should get copied to an 8-bit atlas.
            this->drawPathsAndFlush(ccpr, m);
            REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastCopyAtlasID());
            REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());

            for (int i = 0; i < 10; ++i) {
                this->drawPathsAndFlush(ccpr, m);
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());
            }

            ccpr.ctx()->performDeferredCleanup(std::chrono::milliseconds(0));
        }
    }
};
DEF_CCPR_TEST(CCPR_cache_deferredCleanup)

// Verifies the cache/hash table internals.
class CCPR_cache_hashTable : public CCPRCacheTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr,
               const RecordLastMockAtlasIDs& atlasIDRecorder) override {
        using CoverageType = GrCCAtlas::CoverageType;
        SkMatrix m = SkMatrix::MakeScale(20, 20);

        for (int i = 0; i < 5; ++i) {
            this->drawPathsAndFlush(ccpr, m);
            if (2 == i) {
                REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastCopyAtlasID());
            } else {
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
            }
            if (i < 2) {
                REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastRenderedAtlasID());
            } else {
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());
            }

            auto cache = ccpr.ccpr()->testingOnly_getPathCache();
            REPORTER_ASSERT(reporter, cache);

            const auto& hash = cache->testingOnly_getHashTable();
            const auto& lru = cache->testingOnly_getLRU();
            int count = 0;
            for (GrCCPathCacheEntry* entry : lru) {
                auto* node = hash.find(entry->cacheKey());
                REPORTER_ASSERT(reporter, node);
                REPORTER_ASSERT(reporter, node->entry() == entry);
                REPORTER_ASSERT(reporter, 0 == entry->testingOnly_peekOnFlushRefCnt());
                REPORTER_ASSERT(reporter, entry->unique());
                if (0 == i) {
                    REPORTER_ASSERT(reporter, !entry->cachedAtlas());
                } else {
                    const GrCCCachedAtlas* cachedAtlas = entry->cachedAtlas();
                    REPORTER_ASSERT(reporter, cachedAtlas);
                    if (1 == i) {
                        REPORTER_ASSERT(reporter, ccpr.ccpr()->coverageType()
                                                          == cachedAtlas->coverageType());
                    } else {
                        REPORTER_ASSERT(reporter, CoverageType::kA8_LiteralCoverage
                                                          == cachedAtlas->coverageType());
                    }
                    REPORTER_ASSERT(reporter, cachedAtlas->textureKey().isValid());
                    // The actual proxy should not be held past the end of a flush.
                    REPORTER_ASSERT(reporter, !cachedAtlas->getOnFlushProxy());
                    REPORTER_ASSERT(reporter, 0 == cachedAtlas->testingOnly_peekOnFlushRefCnt());
                }
                ++count;
            }
            REPORTER_ASSERT(reporter, hash.count() == count);
        }
    }
};
DEF_CCPR_TEST(CCPR_cache_hashTable)

// Ensures paths get cached even when using a sporadic flushing pattern and drawing out of order
// (a la Chrome tiles).
class CCPR_cache_multiFlush : public CCPRCacheTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr,
               const RecordLastMockAtlasIDs& atlasIDRecorder) override {
        static constexpr int kNumPaths = SK_ARRAY_COUNT(fPaths);
        static constexpr int kBigPrimes[] = {
                9323, 11059, 22993, 38749, 45127, 53147, 64853, 77969, 83269, 99989};

        SkRandom rand;
        SkMatrix m = SkMatrix::I();

        for (size_t i = 0; i < SK_ARRAY_COUNT(kBigPrimes); ++i) {
            int prime = kBigPrimes[i];
            int endPathIdx = (int)rand.nextULessThan(kNumPaths);
            int pathIdx = endPathIdx;
            int nextFlush = rand.nextRangeU(1, 47);
            for (int j = 0; j < kNumPaths; ++j) {
                pathIdx = (pathIdx + prime) % kNumPaths;
                int repeat = rand.nextRangeU(1, 3);
                for (int k = 0; k < repeat; ++k) {
                    ccpr.drawPath(fPaths[pathIdx], m);
                }
                if (nextFlush == j) {
                    ccpr.flush();
                    // The paths are small enough that we should never copy to an A8 atlas.
                    REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
                    if (i < 2) {
                        REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastRenderedAtlasID());
                    } else {
                        REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());
                    }
                    nextFlush = SkTMin(j + (int)rand.nextRangeU(1, 29), kNumPaths - 1);
                }
            }
            SkASSERT(endPathIdx == pathIdx % kNumPaths);
        }
    }
};
DEF_CCPR_TEST(CCPR_cache_multiFlush)

// Ensures a path drawn over mutiple tiles gets cached.
class CCPR_cache_multiTileCache : public CCPRCacheTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr,
               const RecordLastMockAtlasIDs& atlasIDRecorder) override {
        // Make sure a path drawn over 9 tiles gets cached (1 tile out of 9 is >10% visibility).
        const SkMatrix m0 = SkMatrix::MakeScale(kCanvasSize*3, kCanvasSize*3);
        const SkPath p0 = fPaths[0];
        for (int i = 0; i < 9; ++i) {
            static constexpr int kRowOrder[9] = {0,1,1,0,2,2,2,1,0};
            static constexpr int kColumnOrder[9] = {0,0,1,1,0,1,2,2,2};

            SkMatrix tileM = m0;
            tileM.postTranslate(-kCanvasSize * kColumnOrder[i], -kCanvasSize * kRowOrder[i]);
            ccpr.drawPath(p0, tileM);
            ccpr.flush();
            if (i < 5) {
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
                REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastRenderedAtlasID());
            } else if (5 == i) {
                REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastCopyAtlasID());
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());
            } else {
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());
            }
        }

        // Now make sure paths don't get cached when visibility is <10% for every draw (12 tiles).
        const SkMatrix m1 = SkMatrix::MakeScale(kCanvasSize*4, kCanvasSize*3);
        const SkPath p1 = fPaths[1];
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 4; ++col) {
                SkMatrix tileM = m1;
                tileM.postTranslate(-kCanvasSize * col, -kCanvasSize * row);
                ccpr.drawPath(p1, tileM);
                ccpr.flush();
                REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
                REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastRenderedAtlasID());
            }
        }

        // Double-check the cache is still intact.
        ccpr.drawPath(p0, m0);
        ccpr.flush();
        REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
        REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());

        ccpr.drawPath(p1, m1);
        ccpr.flush();
        REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
        REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastRenderedAtlasID());
    }
};
DEF_CCPR_TEST(CCPR_cache_multiTileCache)

// This test exercises CCPR's cache capabilities by drawing many paths with two different
// transformation matrices. We then vary the matrices independently by whole and partial pixels,
// and verify the caching behaved as expected.
class CCPR_cache_partialInvalidate : public CCPRCacheTest {
    void customizeOptions(GrMockOptions*, GrContextOptions* ctxOptions) override {
        ctxOptions->fAllowPathMaskCaching = true;
    }

    static constexpr int kPathSize = 4;

    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr,
               const RecordLastMockAtlasIDs& atlasIDRecorder) override {
        SkMatrix matrices[2] = {
            SkMatrix::MakeTrans(5, 5),
            SkMatrix::MakeTrans(kCanvasSize - kPathSize - 5, kCanvasSize - kPathSize - 5)
        };
        matrices[0].preScale(kPathSize, kPathSize);
        matrices[1].preScale(kPathSize, kPathSize);

        int firstAtlasID = 0;

        for (int iterIdx = 0; iterIdx < 4*3*2; ++iterIdx) {
            this->drawPathsAndFlush(ccpr, matrices, 2);

            if (0 == iterIdx) {
                // First iteration: just note the ID of the stashed atlas and continue.
                firstAtlasID = atlasIDRecorder.lastRenderedAtlasID();
                REPORTER_ASSERT(reporter, 0 != firstAtlasID);
                continue;
            }

            int testIdx = (iterIdx/2) % 3;
            int repetitionIdx = iterIdx % 2;
            switch (testIdx) {
                case 0:
                    if (0 == repetitionIdx) {
                        // This is the big test. New paths were drawn twice last round. On hit 2
                        // (last time), 'firstAtlasID' was cached as a 16-bit atlas. Now, on hit 3,
                        // these paths should be copied out of 'firstAtlasID', and into an A8 atlas.
                        // THEN: we should recycle 'firstAtlasID' and reuse that same texture to
                        // render the new masks.
                        REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastCopyAtlasID());
                        REPORTER_ASSERT(reporter,
                                        atlasIDRecorder.lastRenderedAtlasID() == firstAtlasID);
                    } else {
                        REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
                        // This is hit 2 for the new masks. Next time they will be copied to an A8
                        // atlas.
                        REPORTER_ASSERT(reporter,
                                        atlasIDRecorder.lastRenderedAtlasID() == firstAtlasID);
                    }

                    if (1 == repetitionIdx) {
                        // Integer translates: all path masks stay valid.
                        matrices[0].preTranslate(-1, -1);
                        matrices[1].preTranslate(1, 1);
                    }
                    break;

                case 1:
                    if (0 == repetitionIdx) {
                        // New paths were drawn twice last round. The third hit (now) they should be
                        // copied to an A8 atlas.
                        REPORTER_ASSERT(reporter, 0 != atlasIDRecorder.lastCopyAtlasID());
                    } else {
                        REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());
                    }

                    // This draw should have gotten 100% cache hits; we only did integer translates
                    // last time (or none if it was the first flush). Therefore, everything should
                    // have been cached.
                    REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastRenderedAtlasID());

                    if (1 == repetitionIdx) {
                        // Invalidate even path masks.
                        matrices[0].preTranslate(1.6f, 1.4f);
                    }
                    break;

                case 2:
                    // No new masks to copy from last time; it had 100% cache hits.
                    REPORTER_ASSERT(reporter, 0 == atlasIDRecorder.lastCopyAtlasID());

                    // Even path masks were invalidated last iteration by a subpixel translate.
                    // They should have been re-rendered this time in the original 'firstAtlasID'
                    // texture.
                    REPORTER_ASSERT(reporter,
                                    atlasIDRecorder.lastRenderedAtlasID() == firstAtlasID);

                    if (1 == repetitionIdx) {
                        // Invalidate odd path masks.
                        matrices[1].preTranslate(-1.4f, -1.6f);
                    }
                    break;
            }
        }
    }
};
DEF_CCPR_TEST(CCPR_cache_partialInvalidate)

class CCPR_unrefPerOpsTaskPathsBeforeOps : public CCPRTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));
        for (int i = 0; i < 10000; ++i) {
            // Draw enough paths to make the arena allocator hit the heap.
            ccpr.drawPath(fPath);
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

class CCPRRenderingTest {
public:
    void run(skiatest::Reporter* reporter, GrContext* ctx, DoStroke doStroke) const {
        if (auto ccpr = ctx->priv().drawingManager()->getCoverageCountingPathRenderer()) {
            if (DoStroke::kYes == doStroke &&
                GrCCAtlas::CoverageType::kA8_Multisample == ccpr->coverageType()) {
                return;  // Stroking is not yet supported for multisample.
            }
            CCPRPathDrawer drawer(sk_ref_sp(ctx), reporter, doStroke);
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
        test.run(reporter, ctxInfo.grContext(), DoStroke::kNo); \
        test.run(reporter, ctxInfo.grContext(), DoStroke::kYes); \
    }

class CCPR_busyPath : public CCPRRenderingTest {
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
DEF_CCPR_RENDERING_TEST(CCPR_busyPath)
