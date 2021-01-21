/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"
#include "tests/TestUtils.h"

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/SkGr.h"
#ifdef SK_GL
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLUtil.h"
#endif
#include "tools/gpu/ProxyUtils.h"

// skbug.com/5932
static void test_basic_draw_as_src(skiatest::Reporter* reporter, GrDirectContext* dContext,
                                   GrSurfaceProxyView rectView, GrColorType colorType,
                                   SkAlphaType alphaType, uint32_t expectedPixelValues[]) {
    auto fillContext = GrSurfaceFillContext::Make(
            dContext, {colorType, kPremul_SkAlphaType, nullptr, rectView.dimensions()});
    for (auto filter : {GrSamplerState::Filter::kNearest, GrSamplerState::Filter::kLinear}) {
        for (auto mm : {GrSamplerState::MipmapMode::kNone, GrSamplerState::MipmapMode::kLinear}) {
            fillContext->clear(SkPMColor4f::FromBytes_RGBA(0xDDCCBBAA));
            auto fp = GrTextureEffect::Make(rectView, alphaType, SkMatrix::I(), filter, mm);
            fillContext->fillWithFP(std::move(fp));
            TestReadPixels(reporter, dContext, fillContext.get(), expectedPixelValues,
                           "RectangleTexture-basic-draw");
        }
    }
}

static void test_clear(skiatest::Reporter* reporter, GrDirectContext* dContext,
                       GrSurfaceContext* rectContext) {
    if (GrSurfaceFillContext* sfc = rectContext->asFillContext()) {
        // Clear the whole thing.
        GrColor color0 = GrColorPackRGBA(0xA, 0xB, 0xC, 0xD);
        sfc->clear(SkPMColor4f::FromBytes_RGBA(color0));

        int w = sfc->width();
        int h = sfc->height();
        int pixelCnt = w * h;
        SkAutoTMalloc<uint32_t> expectedPixels(pixelCnt);

        // The clear color is a GrColor, our readback is to kRGBA_8888, which may be different.
        uint32_t expectedColor0 = 0;
        uint8_t* expectedBytes0 = reinterpret_cast<uint8_t*>(&expectedColor0);
        expectedBytes0[0] = GrColorUnpackR(color0);
        expectedBytes0[1] = GrColorUnpackG(color0);
        expectedBytes0[2] = GrColorUnpackB(color0);
        expectedBytes0[3] = GrColorUnpackA(color0);
        for (int i = 0; i < sfc->width() * sfc->height(); ++i) {
            expectedPixels.get()[i] = expectedColor0;
        }

        // Clear the the top to a different color.
        GrColor color1 = GrColorPackRGBA(0x1, 0x2, 0x3, 0x4);
        SkIRect rect = SkIRect::MakeWH(w, h/2);
        sfc->clear(rect, SkPMColor4f::FromBytes_RGBA(color1));

        uint32_t expectedColor1 = 0;
        uint8_t* expectedBytes1 = reinterpret_cast<uint8_t*>(&expectedColor1);
        expectedBytes1[0] = GrColorUnpackR(color1);
        expectedBytes1[1] = GrColorUnpackG(color1);
        expectedBytes1[2] = GrColorUnpackB(color1);
        expectedBytes1[3] = GrColorUnpackA(color1);

        for (int y = 0; y < h/2; ++y) {
            for (int x = 0; x < w; ++x) {
                expectedPixels.get()[y * h + x] = expectedColor1;
            }
        }

        TestReadPixels(reporter, dContext, sfc, expectedPixels.get(), "RectangleTexture-clear");
    }
}

static void test_copy_to_surface(skiatest::Reporter* reporter,
                                 GrDirectContext* dContext,
                                 GrSurfaceContext* dstContext,
                                 const char* testName) {

    int pixelCnt = dstContext->width() * dstContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    for (int y = 0; y < dstContext->width(); ++y) {
        for (int x = 0; x < dstContext->height(); ++x) {
            pixels.get()[y * dstContext->width() + x] =
                SkColorToPremulGrColor(SkColorSetARGB(2*y, y, x, x * y));
        }
    }

    for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
        auto origin = dstContext->origin();
        GrImageInfo info(GrColorType::kRGBA_8888,
                         kPremul_SkAlphaType,
                         nullptr,
                         dstContext->dimensions());
        GrPixmap pixmap(info, pixels.get(), dstContext->width()*sizeof(uint32_t));
        auto srcView = sk_gpu_test::MakeTextureProxyViewFromData(dContext,
                                                                 renderable,
                                                                 origin,
                                                                 pixmap);
        // If this assert ever fails we can add a fallback to do copy as draw, but until then we can
        // be more restrictive.
        SkAssertResult(dstContext->testCopy(srcView.refProxy()));
        TestReadPixels(reporter, dContext, dstContext, pixels.get(), testName);
    }
}

#ifdef SK_GL
DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(RectangleTexture, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();
    static const int kWidth = 16;
    static const int kHeight = 16;

    uint32_t pixels[kWidth * kHeight];
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            pixels[y * kWidth + x] = y * kWidth + x;
        }
    }
    auto ii = SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkPixmap pm(ii, pixels, sizeof(uint32_t)*kWidth);

    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin }) {

        auto format = GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_RECTANGLE);
        GrBackendTexture rectangleTex = dContext->createBackendTexture(kWidth,
                                                                       kHeight,
                                                                       format,
                                                                       GrMipmapped::kNo,
                                                                       GrRenderable::kYes);
        if (!rectangleTex.isValid()) {
            continue;
        }

        if (!dContext->updateBackendTexture(rectangleTex, &pm, 1, origin, nullptr, nullptr)) {
            continue;
        }

        GrColor refPixels[kWidth * kHeight];
        for (int y = 0; y < kHeight; ++y) {
            for (int x = 0; x < kWidth; ++x) {
                refPixels[y * kWidth + x] = pixels[y * kWidth + x];
            }
        }

        sk_sp<GrTextureProxy> rectProxy = proxyProvider->wrapBackendTexture(
                rectangleTex, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType);

        if (!rectProxy) {
            dContext->deleteBackendTexture(rectangleTex);
            continue;
        }

        SkASSERT(rectProxy->mipmapped() == GrMipmapped::kNo);
        SkASSERT(rectProxy->peekTexture()->mipmapped() == GrMipmapped::kNo);

        SkASSERT(rectProxy->textureType() == GrTextureType::kRectangle);
        SkASSERT(rectProxy->peekTexture()->textureType() == GrTextureType::kRectangle);
        SkASSERT(rectProxy->hasRestrictedSampling());
        SkASSERT(rectProxy->peekTexture()->hasRestrictedSampling());

        GrImageInfo grII = ii;
        GrSwizzle swizzle = dContext->priv().caps()->getReadSwizzle(rectangleTex.getBackendFormat(),
                                                                    grII.colorType());
        GrSurfaceProxyView view(rectProxy, origin, swizzle);

        test_basic_draw_as_src(reporter, dContext, view, grII.colorType(), kPremul_SkAlphaType,
                               refPixels);

        // Test copy to both a texture and RT
        TestCopyFromSurface(reporter, dContext, rectProxy, origin, grII.colorType(), refPixels,
                            "RectangleTexture-copy-from");

        auto rectContext = GrSurfaceContext::Make(dContext, std::move(view), grII.colorInfo());
        SkASSERT(rectContext);

        TestReadPixels(reporter, dContext, rectContext.get(), refPixels, "RectangleTexture-read");

        test_copy_to_surface(reporter, dContext, rectContext.get(), "RectangleTexture-copy-to");

        TestWritePixels(reporter, dContext, rectContext.get(), true, "RectangleTexture-write");

        test_clear(reporter, dContext, rectContext.get());

        dContext->deleteBackendTexture(rectangleTex);
    }
}
#endif
