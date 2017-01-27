/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLUtil.h"
#include "gl/GLTestContext.h"

static void test_read_pixels(skiatest::Reporter* reporter, GrContext* context,
                             GrSurfaceContext* srcContext, uint32_t expectedPixelValues[]) {
    int pixelCnt = srcContext->width() * srcContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    memset(pixels.get(), 0, sizeof(uint32_t)*pixelCnt);

    SkImageInfo ii = SkImageInfo::Make(srcContext->width(), srcContext->height(),
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    bool read = srcContext->readPixels(ii, pixels.get(), 0, 0, 0);
    if (!read) {
        ERRORF(reporter, "Error reading rectangle texture.");
    }

    for (int i = 0; i < pixelCnt; ++i) {
        if (pixels.get()[i] != expectedPixelValues[i]) {
            ERRORF(reporter, "Error, pixel value %d should be 0x%08x, got 0x%08x.", i,
                   expectedPixelValues[i], pixels.get()[i]);
            break;
        }
    }
}

static void test_write_pixels(skiatest::Reporter* reporter, GrContext* context,
                              GrSurfaceContext* rectSurfaceContext) {
    int pixelCnt = rectSurfaceContext->width() * rectSurfaceContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    for (int y = 0; y < rectSurfaceContext->width(); ++y) {
        for (int x = 0; x < rectSurfaceContext->height(); ++x) {
            pixels.get()[y * rectSurfaceContext->width() + x] = GrColorPackRGBA(x, y, x + y, x * y);
        }
    }

    SkImageInfo ii = SkImageInfo::Make(rectSurfaceContext->width(), rectSurfaceContext->height(),
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    bool write = rectSurfaceContext->writePixels(ii, pixels.get(), 0, 0, 0);
    if (!write) {
        ERRORF(reporter, "Error writing to rectangle texture.");
    }

    test_read_pixels(reporter, context, rectSurfaceContext, pixels.get());
}

static void test_copy_surface_src(skiatest::Reporter* reporter, GrContext* context,
                                  GrSurfaceProxy* rectProxy, uint32_t expectedPixelValues[]) {
    GrSurfaceDesc copyDstDesc;
    copyDstDesc.fConfig = kRGBA_8888_GrPixelConfig;
    copyDstDesc.fWidth = rectProxy->width();
    copyDstDesc.fHeight = rectProxy->height();

    for (auto flags : {kNone_GrSurfaceFlags, kRenderTarget_GrSurfaceFlag}) {
        copyDstDesc.fFlags = flags;

        sk_sp<GrSurfaceContext> dstContext(GrSurfaceProxy::TestCopy(context, copyDstDesc,
                                                                    rectProxy));

        test_read_pixels(reporter, context, dstContext.get(), expectedPixelValues);
    }
}

static void test_copy_surface_dst(skiatest::Reporter* reporter, GrContext* context,
                                  GrSurfaceContext* rectContext) {

    int pixelCnt = rectContext->width() * rectContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    for (int y = 0; y < rectContext->width(); ++y) {
        for (int x = 0; x < rectContext->height(); ++x) {
            pixels.get()[y * rectContext->width() + x] = GrColorPackRGBA(y, x, x * y, x *+ y);
        }
    }
    for (auto flags : {kNone_GrSurfaceFlags, kRenderTarget_GrSurfaceFlag}) {
        GrSurfaceDesc copySrcDesc;
        copySrcDesc.fConfig = kRGBA_8888_GrPixelConfig;
        copySrcDesc.fWidth = rectContext->width();
        copySrcDesc.fHeight = rectContext->height();
        copySrcDesc.fFlags = flags;

        sk_sp<GrSurfaceProxy> src(GrSurfaceProxy::MakeDeferred(*context->caps(),
                                                               context->textureProvider(),
                                                               copySrcDesc,
                                                               SkBudgeted::kYes, pixels.get(), 0));
        rectContext->copy(src.get());

        test_read_pixels(reporter, context, rectContext, pixels.get());
    }
}

// skbug.com/5932
static void test_basic_draw_as_src(skiatest::Reporter* reporter, GrContext* context,
                                   sk_sp<GrSurfaceProxy> rectProxy, uint32_t expectedPixelValues[]) {
    sk_sp<GrRenderTargetContext> rtContext(
            context->makeRenderTargetContext(SkBackingFit::kExact, rectProxy->width(),
                                             rectProxy->height(), rectProxy->config(),
                                             nullptr));
    for (auto filter : {GrSamplerParams::kNone_FilterMode,
                        GrSamplerParams::kBilerp_FilterMode,
                        GrSamplerParams::kMipMap_FilterMode}) {
        rtContext->clear(nullptr, 0xDDCCBBAA, true);
        sk_sp<GrFragmentProcessor> fp(GrSimpleTextureEffect::Make(
                                                        context,
                                                        sk_ref_sp(rectProxy->asTextureProxy()),
                                                        nullptr,
                                                        SkMatrix::I(), filter));
        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.addColorFragmentProcessor(std::move(fp));
        rtContext->drawPaint(GrNoClip(), std::move(paint), SkMatrix::I());
        test_read_pixels(reporter, context, rtContext.get(), expectedPixelValues);
    }
}

static void test_clear(skiatest::Reporter* reporter, GrContext* context,
                       GrSurfaceContext* rectContext) {
    if (GrRenderTargetContext* rtc = rectContext->asRenderTargetContext()) {
        // Clear the whole thing.
        GrColor color0 = GrColorPackRGBA(0xA, 0xB, 0xC, 0xD);
        rtc->clear(nullptr, color0, false);

        int w = rtc->width();
        int h = rtc->height();
        int pixelCnt = w * h;
        SkAutoTMalloc<uint32_t> expectedPixels(pixelCnt);

        // The clear color is a GrColor, our readback is to kRGBA_8888, which may be different.
        uint32_t expectedColor0 = 0;
        uint8_t* expectedBytes0 = SkTCast<uint8_t*>(&expectedColor0);
        expectedBytes0[0] = GrColorUnpackR(color0);
        expectedBytes0[1] = GrColorUnpackG(color0);
        expectedBytes0[2] = GrColorUnpackB(color0);
        expectedBytes0[3] = GrColorUnpackA(color0);
        for (int i = 0; i < rtc->width() * rtc->height(); ++i) {
            expectedPixels.get()[i] = expectedColor0;
        }

        // Clear the the top to a different color.
        GrColor color1 = GrColorPackRGBA(0x1, 0x2, 0x3, 0x4);
        SkIRect rect = SkIRect::MakeWH(w, h/2);
        rtc->clear(&rect, color1, false);

        uint32_t expectedColor1 = 0;
        uint8_t* expectedBytes1 = SkTCast<uint8_t*>(&expectedColor1);
        expectedBytes1[0] = GrColorUnpackR(color1);
        expectedBytes1[1] = GrColorUnpackG(color1);
        expectedBytes1[2] = GrColorUnpackB(color1);
        expectedBytes1[3] = GrColorUnpackA(color1);

        for (int y = 0; y < h/2; ++y) {
            for (int x = 0; x < w; ++x) {
                expectedPixels.get()[y * h + x] = expectedColor1;
            }
        }

        test_read_pixels(reporter, context, rtc, expectedPixels.get());
    }
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(RectangleTexture, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    sk_gpu_test::GLTestContext* glContext = ctxInfo.glContext();
    static const int kWidth = 13;
    static const int kHeight = 13;

    GrColor pixels[kWidth * kHeight];
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            pixels[y * kWidth + x] = y * kWidth + x;
        }
    }

    for (int origin = 0; origin < 2; ++origin) {
        GrGLuint rectTexID = glContext->createTextureRectangle(kWidth, kHeight, GR_GL_RGBA,
                                                               GR_GL_RGBA, GR_GL_UNSIGNED_BYTE,
                                                               pixels);

        if (!rectTexID) {
            return;
        }

        // Let GrContext know that we messed with the GL context directly.
        context->resetContext();

        // Wrap the rectangle texture ID in a GrTexture
        GrGLTextureInfo rectangleInfo;
        rectangleInfo.fID = rectTexID;
        rectangleInfo.fTarget = GR_GL_TEXTURE_RECTANGLE;

        GrBackendTextureDesc rectangleDesc;
        rectangleDesc.fFlags = kRenderTarget_GrBackendTextureFlag;
        rectangleDesc.fConfig = kRGBA_8888_GrPixelConfig;
        rectangleDesc.fWidth = kWidth;
        rectangleDesc.fHeight = kHeight;
        rectangleDesc.fOrigin = origin ? kBottomLeft_GrSurfaceOrigin : kTopLeft_GrSurfaceOrigin;
        rectangleDesc.fTextureHandle = reinterpret_cast<GrBackendObject>(&rectangleInfo);

        GrColor refPixels[kWidth * kHeight];
        bool flipRef = rectangleDesc.fOrigin == kBottomLeft_GrSurfaceOrigin;
        for (int y = 0; y < kHeight; ++y) {
            for (int x = 0; x < kWidth; ++x) {
                int y0 = flipRef ? kHeight - y - 1 : y;
                refPixels[y * kWidth + x] = pixels[y0 * kWidth + x];
            }
        }

        sk_sp<GrSurfaceProxy> rectProxy;

        {
            sk_sp<GrTexture> rectangleTexture(
                context->textureProvider()->wrapBackendTexture(rectangleDesc));
            if (!rectangleTexture) {
                ERRORF(reporter, "Error wrapping rectangle texture in GrTexture.");
                GR_GL_CALL(glContext->gl(), DeleteTextures(1, &rectTexID));
                continue;
            }

            rectProxy = GrSurfaceProxy::MakeWrapped(std::move(rectangleTexture));
            if (!rectProxy) {
                ERRORF(reporter, "Error creating proxy for rectangle texture.");
                GR_GL_CALL(glContext->gl(), DeleteTextures(1, &rectTexID));
                continue;
            }
        }

        test_basic_draw_as_src(reporter, context, rectProxy, refPixels);

        test_copy_surface_src(reporter, context, rectProxy.get(), refPixels);

        sk_sp<GrSurfaceContext> rectContext = context->contextPriv().makeWrappedSurfaceContext(
                                                                    std::move(rectProxy), nullptr);
        SkASSERT(rectContext);

        test_read_pixels(reporter, context, rectContext.get(), refPixels);

        test_copy_surface_dst(reporter, context, rectContext.get());

        test_write_pixels(reporter, context, rectContext.get());

        test_clear(reporter, context, rectContext.get());

        GR_GL_CALL(glContext->gl(), DeleteTextures(1, &rectTexID));
    }
}

#endif
