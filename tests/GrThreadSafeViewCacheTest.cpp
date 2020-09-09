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
    return SkImageInfo::Make(kImageWH, kImageWH, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
}

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

static GrSurfaceProxyView create_view_on_cpu(GrRecordingContext* rContext) {
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
    return {std::move(proxy), kImageOrigin, swizzle};
}

static GrSurfaceProxyView create_view_on_gpu(GrRecordingContext* rContext) {
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

    return rtc->readSurfaceView();
}

static bool access_cached_view(GrRecordingContext* rContext) {
    GrUniqueKey key;

    create_key(&key, 1);

    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    auto threadSafeViewCache = rContext->priv().threadSafeViewCache();
    if (auto view = threadSafeViewCache->find(key)) {
        return true;
    }

    GrSurfaceProxyView mask;
    if (proxyProvider->isDDLProvider() == GrDDLProvider::kNo) {
        mask = create_view_on_gpu(rContext);
    } else {
        mask = create_view_on_cpu(rContext);
    }
    SkASSERT(mask);

    mask = threadSafeViewCache->add(key, mask);
    return false;
}

class TestSetup {
public:
    TestSetup(GrDirectContext* dContext) {
        fDst = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, default_ii());

        SkSurfaceCharacterization characterization;
        SkAssertResult(fDst->characterize(&characterization));

        fRecorder1 = std::make_unique<SkDeferredDisplayListRecorder>(characterization);
        fCanvas1 = fRecorder1->getCanvas();
        fRContext1 = fCanvas1->recordingContext();

        fRecorder2 = std::make_unique<SkDeferredDisplayListRecorder>(characterization);
        fCanvas2 = fRecorder2->getCanvas();
        fRContext2 = fCanvas2->recordingContext();
    }

    GrRecordingContext* rContext1() { return fRContext1; }

private:
    sk_sp<SkSurface> fDst;

    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder1;
    SkCanvas* fCanvas1;
    GrRecordingContext* fRContext1;

    std::unique_ptr<SkDeferredDisplayListRecorder> fRecorder2;
    SkCanvas* fCanvas2;
    GrRecordingContext* fRContext2;
};

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache1, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    auto threadSafeViewCache = dContext->priv().threadSafeViewCache();

    TestSetup setup(dContext);

    bool cacheHit = access_cached_view(setup.rContext1());
    REPORTER_ASSERT(reporter, !cacheHit);

    cacheHit = access_cached_view(setup.rContext2());
    REPORTER_ASSERT(reporter, cacheHit);

    REPORTER_ASSERT(reporter, threadSafeViewCache->numEntries() == 1);
}
