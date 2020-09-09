/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrThreadSafeUniquelyKeyedProxyViewCache.h"
#include "tests/Test.h"

static constexpr int kImageWH = 32;
static constexpr auto kImageOrigin = kBottomLeft_GrSurfaceOrigin;

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

    SkImageInfo ii = SkImageInfo::Make(kImageWH, kImageWH,
                                       kRGBA_8888_SkColorType,
                                       kUnpremul_SkAlphaType);
    bitmap.allocPixels(ii);

    SkCanvas tmp(bitmap);
    tmp.clear(SK_ColorWHITE);

    SkPaint blue;
    blue.setColor(SK_ColorBLUE);
    blue.setAntiAlias(true);

    tmp.drawPath(create_up_arrow_path(kImageWH, kImageWH), blue);

    return bitmap;
}

static GrSurfaceProxyView create_on_cpu(GrRecordingContext* rContext) {
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

static GrSurfaceProxyView create_on_gpu(GrRecordingContext* rContext) {
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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrThreadSafeViewCache1, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    SkBitmap bm = create_bitmap();

    auto threadSafeViewCache = dContext->priv().threadSafeViewCache();

}
