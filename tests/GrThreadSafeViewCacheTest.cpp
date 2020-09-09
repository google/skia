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

static constexpr int kImageWH = 32;
static constexpr auto kImageOrigin = kBottomLeft_GrSurfaceOrigin;

static SkImageInfo default_ii() {
    return SkImageInfo::Make(kImageWH, kImageWH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
}

class TestHelper {
public:
    struct Stats {
        int fNumSWCreations = 0;
        int fNumHWCreations = 0;
    };

    TestHelper(GrDirectContext* dContext) : fDContext(dContext) {

        fDst = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, default_ii());
        SkAssertResult(fDst);

        SkSurfaceCharacterization characterization;
        SkAssertResult(fDst->characterize(&characterization));

        fRecorder1 = std::make_unique<SkDeferredDisplayListRecorder>(characterization);
        fCanvas1 = fRecorder1->getCanvas();
        fRContext1 = fCanvas1->recordingContext();

        fRecorder2 = std::make_unique<SkDeferredDisplayListRecorder>(characterization);
        fCanvas2 = fRecorder2->getCanvas();
        fRContext2 = fCanvas2->recordingContext();
    }

    Stats* stats() { return &fStats; }

    int numCacheEntries() const {
        auto threadSafeViewCache = helper.threadSafeViewCache();
        return threadSafeViewCache->numEntries();
    }

    GrRecordingContext* rContext1() { return fRContext1; }

    GrRecordingContext* rContext2() { return fRContext2; }

    GrThreadSafeUniquelyKeyedProxyViewCache* threadSafeViewCache() {
        return fDContext->priv().threadSafeViewCache();
    }

    bool accessCachedView(GrRecordingContext* rContext) {
        return AccessCachedView(rContext, &fStats);
    }

private:
    static bool AccessCachedView(GrRecordingContext*, Stats*);
    static GrSurfaceProxyView CreateViewOnCpu(GrRecordingContext*, Stats*);
    static GrSurfaceProxyView CreateViewOnGpu(GrRecordingContext*, Stats*);

    Stats fStats;
    GrDirectContext* fDContext;
    sk_sp<SkSurface> fDst;

    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder1;
    SkCanvas* fCanvas1;
    GrRecordingContext* fRContext1;

    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder2;
    SkCanvas* fCanvas2;
    GrRecordingContext* fRContext2;
};

static void create_key(GrUniqueKey* key, int contents) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kDomain, 1);
    builder[0] = contents;
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

SkBitmap create_bitmap() {
    SkBitmap bitmap;

    bitmap.allocPixels(default_ii());

    SkCanvas tmp(bitmap);
    tmp.clear(SK_ColorWHITE);

    SkPaint blue;
    blue.setColor(SK_ColorBLUE);
    blue.setAntiAlias(true);

    tmp.drawPath(create_up_arrow_path(kImageWH, kImageWH), blue);

    return bitmap;
}

GrSurfaceProxyView TestHelper::CreateViewOnCpu(GrRecordingContext* rContext, Stats* stats) {
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    sk_sp<GrTextureProxy> proxy = proxyProvider->createProxyFromBitmap(create_bitmap(),
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

GrSurfaceProxyView TestHelper::CreateViewOnGpu(GrRecordingContext* rContext, Stats* stats) {
    auto rtc = GrRenderTargetContext::Make(rContext,
                                           GrColorType::kRGBA_8888,
                                           nullptr,
                                           SkBackingFit::kExact,
                                           {kImageWH, kImageWH},
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
                  create_up_arrow_path(kImageWH, kImageWH), GrStyle::SimpleFill());

    auto result = rtc->readSurfaceView();
    if (!result) {
        return {};
    }

    ++stats->fNumHWCreations;
    return result;
}

bool TestHelper::AccessCachedView(GrRecordingContext* rContext, Stats* stats) {
    GrUniqueKey key;

    create_key(&key, 1);

    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    auto threadSafeViewCache = rContext->priv().threadSafeViewCache();
    if (auto view = threadSafeViewCache->find(key)) {
        return true;
    }

    GrSurfaceProxyView mask;
    if (proxyProvider->isDDLProvider() == GrDDLProvider::kNo) {
        mask = CreateViewOnGpu(rContext, stats);
    } else {
        mask = CreateViewOnCpu(rContext, stats);
    }
    SkASSERT(mask);

    mask = threadSafeViewCache->add(key, mask);
    return false;
}


DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache1, reporter, ctxInfo) {
    TestHelper helper(ctxInfo.directContext());

    bool cacheHit = helper.accessCachedView(helper.rContext1());
    REPORTER_ASSERT(reporter, !cacheHit);

    cacheHit = helper.accessCachedView(helper.rContext2());
    REPORTER_ASSERT(reporter, cacheHit);

    REPORTER_ASSERT(reporter, helper.numCacheEntries() == 1);
    REPORTER_ASSERT(reporter, helper.stats()->fNumHWCreations == 0);
    REPORTER_ASSERT(reporter, helper.stats()->fNumSWCreations == 1);
}
