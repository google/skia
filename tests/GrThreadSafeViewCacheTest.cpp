/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrThreadSafeUniquelyKeyedProxyViewCache.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"

static constexpr int kImageWH = 32;
static constexpr auto kImageOrigin = kBottomLeft_GrSurfaceOrigin;

static SkImageInfo default_ii(int wh) {
    return SkImageInfo::Make(wh, wh, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
}

static bool is_instantiated(const GrSurfaceProxyView& view) {
    return view.proxy()->isInstantiated();
}

static bool is_lazy(const GrSurfaceProxyView& view) {
    return view.proxy()->isLazy();
}

static bool check_refs(const GrSurfaceProxyView& view, int numRefs) {
    return view.proxy()->refCntGreaterThan(numRefs) &&
           !view.proxy()->refCntGreaterThan(numRefs+1);
}

static void create_key(GrUniqueKey* key, int wh) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kDomain, 1);
    builder[0] = wh;
    builder.finish();
};

static SkPath create_up_arrow_path(int width, int height) {
    float halfW = width / 2.0f;
    float halfH = height / 2.0f;
    float thirdW = width / 3.0f;

    SkPath path;
    path.moveTo(0, halfH);
    path.lineTo(thirdW, halfH);
    path.lineTo(thirdW, height);
    path.lineTo(2*thirdW, height);
    path.lineTo(2*thirdW, halfH);
    path.lineTo(width, halfH);
    path.lineTo(halfW, 0);
    path.close();

    return path;
}

SkBitmap create_bitmap(int wh) {
    SkBitmap bitmap;

    bitmap.allocPixels(default_ii(wh));

    SkCanvas tmp(bitmap);
    tmp.clear(SK_ColorWHITE);

    SkPaint blue;
    blue.setColor(SK_ColorBLUE);
    blue.setAntiAlias(true);

    tmp.drawPath(create_up_arrow_path(wh, wh), blue);

    return bitmap;
}

class TestHelper {
public:
    struct Stats {
        bool check(int hits, int misses) const {
            return fCacheHits == hits && fCacheMisses == misses;
        }

        int fCacheHits = 0;
        int fCacheMisses = 0;

        int fNumSWCreations = 0;
        int fNumHWCreations = 0;
    };

    TestHelper(GrDirectContext* dContext) : fDContext(dContext) {

        fDst = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, default_ii(kImageWH));
        SkAssertResult(fDst);

        SkSurfaceCharacterization characterization;
        SkAssertResult(fDst->characterize(&characterization));

        fRecorder1 = std::make_unique<SkDeferredDisplayListRecorder>(characterization);
        fCanvas1 = fRecorder1->getCanvas();
        fRContext1 = fCanvas1->recordingContext();

        fRecorder2 = std::make_unique<SkDeferredDisplayListRecorder>(characterization);
        fCanvas2 = fRecorder2->getCanvas();
        fRContext2 = fCanvas2->recordingContext();

        SkBitmap tmp = create_bitmap(kImageWH);
        SkAssertResult(CreateBackendTexture(fDContext, &fBETex, tmp));
    }

    ~TestHelper() {
        DeleteBackendTexture(fDContext, fBETex);
    }

    Stats* stats() { return &fStats; }

    int numCacheEntries() const {
        return fDContext->priv().threadSafeViewCache()->numEntries();
    }

    GrDirectContext* dContext() { return fDContext; }

    GrRecordingContext* rContext1() { return fRContext1; }

    GrRecordingContext* rContext2() { return fRContext2; }

    GrThreadSafeUniquelyKeyedProxyViewCache* threadSafeViewCache() {
        return fDContext->priv().threadSafeViewCache();
    }

    GrSurfaceProxyView accessCachedView(GrRecordingContext* rContext, int wh,
                                        bool failLookup = false) {
        return AccessCachedView(rContext, fBETex, wh, failLookup, &fStats);
    }

    GrSurfaceProxyView gimme(int wh) {
        GrUniqueKey key;

        create_key(&key, wh);

        auto threadSafeViewCache = this->threadSafeViewCache();

        return threadSafeViewCache->find(key);
    }

private:
    static GrSurfaceProxyView AccessCachedView(GrRecordingContext*, GrBackendTexture, int wh,
                                               bool failLookup, Stats*);
    static GrSurfaceProxyView CreateViewOnCpu(GrRecordingContext*, int wh, Stats*);
    static GrSurfaceProxyView CreateViewOnGpu(GrDirectContext*, GrBackendTexture, int wh, Stats*);

    Stats fStats;
    GrDirectContext* fDContext = nullptr;
    sk_sp<SkSurface> fDst;

    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder1;
    SkCanvas* fCanvas1 = nullptr;
    GrRecordingContext* fRContext1 = nullptr;

    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder2;
    SkCanvas* fCanvas2 = nullptr;
    GrRecordingContext* fRContext2 = nullptr;

    GrBackendTexture fBETex;
};

GrSurfaceProxyView TestHelper::CreateViewOnCpu(GrRecordingContext* rContext,
                                               int wh,
                                               Stats* stats) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxyFromBitmap(create_bitmap(wh),
                                                                       GrMipmapped::kNo,
                                                                       SkBackingFit::kExact,
                                                                       SkBudgeted::kYes);
    if (!proxy) {
        return {};
    }

    GrSwizzle swizzle = rContext->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                                GrColorType::kRGBA_8888);
    ++stats->fNumSWCreations;
    return {std::move(proxy), kImageOrigin, swizzle};
}

GrSurfaceProxyView TestHelper::CreateViewOnGpu(GrDirectContext* dContext,
                                               GrBackendTexture beTex,
                                               int wh,
                                               Stats* stats) {
#if 0
    auto rtc = GrRenderTargetContext::Make(rContext,
                                           GrColorType::kRGBA_8888,
                                           nullptr,
                                           SkBackingFit::kExact,
                                           {wh, wh},
                                           1,
                                           GrMipmapped::kNo,
                                           GrProtected::kNo,
                                           kImageOrigin);
    if (!rtc) {
        return {};
    }

    rtc->clear(SK_PMColor4fWHITE);

    GrPaint paint;
    paint.setColor4f({0, 0, 1, 1});

    rtc->drawPath(nullptr, std::move(paint), GrAA::kYes, SkMatrix::I(),
                  create_up_arrow_path(wh, wh), GrStyle::SimpleFill());

    rtc->flush(SkSurface::BackendSurfaceAccess::kNoAccess, GrFlushInfo(), nullptr);

    auto result = rtc->readSurfaceView();
    if (!result) {
        return {};
    }

    ++stats->fNumHWCreations;
    return result;
#endif
    SkASSERT(!stats->fNumHWCreations); // we should never do this more than once

    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();

    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(
            beTex, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRead_GrIOType);
    GrSwizzle swizzle = dContext->priv().caps()->getReadSwizzle(proxy->backendFormat(),
                                                                GrColorType::kRGBA_8888);
    ++stats->fNumHWCreations;
    return {std::move(proxy), kImageOrigin, swizzle};
}

GrSurfaceProxyView TestHelper::AccessCachedView(GrRecordingContext* rContext,
                                                GrBackendTexture beTex,
                                                int wh,
                                                bool failLookup,
                                                Stats* stats) {
    GrUniqueKey key;

    create_key(&key, wh);

    auto threadSafeViewCache = rContext->priv().threadSafeViewCache();
    if (auto view = threadSafeViewCache->find(key); !failLookup && view) {
        ++stats->fCacheHits;
        return view;
    }

    ++stats->fCacheMisses;

    GrSurfaceProxyView mask;
    if (GrDirectContext* dContext = rContext->asDirectContext()) {
        mask = CreateViewOnGpu(dContext, beTex, wh, stats);
    } else {
        mask = CreateViewOnCpu(rContext, wh, stats);
    }
    SkASSERT(mask);

    return threadSafeViewCache->add(key, mask);
}

// Case 1: ensure two DDL recorders share the view
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache1, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    auto first = helper.accessCachedView(helper.rContext1(), kImageWH);
    REPORTER_ASSERT(reporter, helper.stats()->check(0, 1));
    REPORTER_ASSERT(reporter, is_lazy(first));

    auto second = helper.accessCachedView(helper.rContext2(), kImageWH);
    REPORTER_ASSERT(reporter, helper.stats()->check(1, 1));
    REPORTER_ASSERT(reporter, first == second);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 1);

    GrSurfaceProxyView view = helper.gimme(kImageWH);
    REPORTER_ASSERT(reporter, check_refs(view, 3));
}

// Case 2: ensure that, if the direct context version wins, it is reused by the DDL recorders
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache2, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    auto first = helper.accessCachedView(helper.dContext(), kImageWH);
    REPORTER_ASSERT(reporter, helper.stats()->check(0, 1));
    REPORTER_ASSERT(reporter, is_instantiated(first));
    REPORTER_ASSERT(reporter, !is_lazy(first));

    auto second = helper.accessCachedView(helper.rContext1(), kImageWH);
    REPORTER_ASSERT(reporter, helper.stats()->check(1, 1));

    auto third = helper.accessCachedView(helper.rContext2(), kImageWH);
    REPORTER_ASSERT(reporter, helper.stats()->check(2, 1));

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 0);

    GrSurfaceProxyView view = helper.gimme(kImageWH);
    REPORTER_ASSERT(reporter, check_refs(view, 4));
}

// Case 3: ensure that, if the cpu-version wins, it is reused by the direct context
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache3, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    auto first = helper.accessCachedView(helper.rContext1(), kImageWH);
    REPORTER_ASSERT(reporter, helper.stats()->check(0, 1));
    REPORTER_ASSERT(reporter, is_lazy(first));

    auto second = helper.accessCachedView(helper.dContext(), kImageWH);
    REPORTER_ASSERT(reporter, helper.stats()->check(1, 1));
    REPORTER_ASSERT(reporter, first == second);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 1);

    GrSurfaceProxyView view = helper.gimme(kImageWH);
    REPORTER_ASSERT(reporter, check_refs(view, 3));
}

// Case 4: ensure that, if two DDL recorders get in a race, they still end up sharing a single view
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache4, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    auto first = helper.accessCachedView(helper.rContext1(), kImageWH);
    REPORTER_ASSERT(reporter, helper.stats()->check(0, 1));
    REPORTER_ASSERT(reporter, is_lazy(first));

    static const bool kFailLookup = true;
    auto second = helper.accessCachedView(helper.rContext2(), kImageWH, kFailLookup);
    REPORTER_ASSERT(reporter, helper.stats()->check(0, 2));
    REPORTER_ASSERT(reporter, first == second);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 2);

    GrSurfaceProxyView view = helper.gimme(kImageWH);
    REPORTER_ASSERT(reporter, check_refs(view, 3));
}
