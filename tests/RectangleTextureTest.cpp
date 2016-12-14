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
                             GrTexture* texture, uint32_t expectedPixelValues[]) {
    int pixelCnt = texture->width() * texture->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    memset(pixels.get(), 0, sizeof(uint32_t)*pixelCnt);
    bool read = texture->readPixels(0, 0, texture->width(), texture->height(),
                                    kRGBA_8888_GrPixelConfig, pixels.get());
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
                              GrTexture* rectangleTexture) {
    int pixelCnt = rectangleTexture->width() * rectangleTexture->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    for (int y = 0; y < rectangleTexture->width(); ++y) {
        for (int x = 0; x < rectangleTexture->height(); ++x) {
            pixels.get()[y * rectangleTexture->width() + x] = GrColorPackRGBA(x, y, x + y, x * y);
        }
    }
    bool write = rectangleTexture->writePixels(0, 0, rectangleTexture->width(),
                                               rectangleTexture->height(), kRGBA_8888_GrPixelConfig,
                                               pixels.get());
    if (!write) {
        ERRORF(reporter, "Error writing to rectangle texture.");
    }
    test_read_pixels(reporter, context, rectangleTexture, pixels.get());
}

static void test_copy_surface_src(skiatest::Reporter* reporter, GrContext* context,
                                  GrTexture* rectangleTexture, uint32_t expectedPixelValues[]) {
    for (auto flags : {kNone_GrSurfaceFlags, kRenderTarget_GrSurfaceFlag}) {
        GrSurfaceDesc copyDstDesc;
        copyDstDesc.fConfig = kRGBA_8888_GrPixelConfig;
        copyDstDesc.fWidth = rectangleTexture->width();
        copyDstDesc.fHeight = rectangleTexture->height();
        copyDstDesc.fFlags = flags;
        sk_sp<GrTexture> dst(
                context->textureProvider()->createTexture(copyDstDesc, SkBudgeted::kYes));
        context->copySurface(dst.get(), rectangleTexture);
        test_read_pixels(reporter, context, dst.get(), expectedPixelValues);
    }
}

static void test_copy_surface_dst(skiatest::Reporter* reporter, GrContext* context,
                                  GrTexture* rectangleTexture) {
    int pixelCnt = rectangleTexture->width() * rectangleTexture->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    for (int y = 0; y < rectangleTexture->width(); ++y) {
        for (int x = 0; x < rectangleTexture->height(); ++x) {
            pixels.get()[y * rectangleTexture->width() + x] = GrColorPackRGBA(y, x, x * y, x *+ y);
        }
    }
    for (auto flags : {kNone_GrSurfaceFlags, kRenderTarget_GrSurfaceFlag}) {
        GrSurfaceDesc copySrcDesc;
        copySrcDesc.fConfig = kRGBA_8888_GrPixelConfig;
        copySrcDesc.fWidth = rectangleTexture->width();
        copySrcDesc.fHeight = rectangleTexture->height();
        copySrcDesc.fFlags = flags;
        sk_sp<GrTexture> src(context->textureProvider()->createTexture(
                copySrcDesc, SkBudgeted::kYes, pixels.get(), 0));

        context->copySurface(rectangleTexture, src.get());
        test_read_pixels(reporter, context, rectangleTexture, pixels.get());
    }
}

// skbug.com/5932
static void test_basic_draw(skiatest::Reporter* reporter, GrContext* context,
                            GrTexture* rectangleTexture, uint32_t expectedPixelValues[]) {
    sk_sp<GrRenderTargetContext> rtContext(
            context->makeRenderTargetContext(SkBackingFit::kExact, rectangleTexture->width(),
                                             rectangleTexture->height(), rectangleTexture->config(),
                                             nullptr));
    SkMatrix m;
    m.setIDiv(rectangleTexture->width(), rectangleTexture->height());
    for (auto filter : {GrSamplerParams::kNone_FilterMode,
                        GrSamplerParams::kBilerp_FilterMode,
                        GrSamplerParams::kMipMap_FilterMode}) {
        rtContext->clear(nullptr, 0xDDCCBBAA, true);
        sk_sp<GrFragmentProcessor> fp(GrSimpleTextureEffect::Make(rectangleTexture,
                                                                  nullptr, m, filter));
        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.addColorFragmentProcessor(std::move(fp));
        rtContext->drawPaint(GrNoClip(), paint, SkMatrix::I());
        test_read_pixels(reporter, context, rtContext->asTexture().get(), expectedPixelValues);
    }
}

static void test_clear(skiatest::Reporter* reporter, GrContext* context,
                       GrTexture* rectangleTexture) {
    if (rectangleTexture->asRenderTarget()) {
        sk_sp<GrRenderTargetContext> rtc(context->contextPriv().makeWrappedRenderTargetContext(
                                                    sk_ref_sp(rectangleTexture->asRenderTarget()),
                                                    nullptr));
        if (!rtc) {
            ERRORF(reporter, "Could not get GrRenderTargetContext for rectangle texture.");
            return;
        }

        // Clear the whole thing.
        GrColor color0 = GrColorPackRGBA(0xA, 0xB, 0xC, 0xD);
        rtc->clear(nullptr, color0, false);

        int w = rectangleTexture->width();
        int h = rectangleTexture->height();
        int pixelCnt = w * h;
        SkAutoTMalloc<uint32_t> expectedPixels(pixelCnt);

        // The clear color is a GrColor, our readback is to kRGBA_8888, which may be different.
        uint32_t expectedColor0 = 0;
        uint8_t* expectedBytes0 = SkTCast<uint8_t*>(&expectedColor0);
        expectedBytes0[0] = GrColorUnpackR(color0);
        expectedBytes0[1] = GrColorUnpackG(color0);
        expectedBytes0[2] = GrColorUnpackB(color0);
        expectedBytes0[3] = GrColorUnpackA(color0);
        for (int i = 0; i < rectangleTexture->width() * rectangleTexture->height(); ++i) {
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

        test_read_pixels(reporter, context, rectangleTexture, expectedPixels.get());
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

        sk_sp<GrTexture> rectangleTexture(
            context->textureProvider()->wrapBackendTexture(rectangleDesc));
        if (!rectangleTexture) {
            ERRORF(reporter, "Error wrapping rectangle texture in GrTexture.");
            GR_GL_CALL(glContext->gl(), DeleteTextures(1, &rectTexID));
            continue;
        }

        test_read_pixels(reporter, context, rectangleTexture.get(), refPixels);

        test_basic_draw(reporter, context, rectangleTexture.get(), refPixels);

        test_copy_surface_src(reporter, context, rectangleTexture.get(), refPixels);

        test_copy_surface_dst(reporter, context, rectangleTexture.get());

        test_write_pixels(reporter, context, rectangleTexture.get());

        test_clear(reporter, context, rectangleTexture.get());

        GR_GL_CALL(glContext->gl(), DeleteTextures(1, &rectTexID));
    }
}

#endif
