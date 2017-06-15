/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestUtils.h"

#if SK_SUPPORT_GPU
#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrTest.h"
#include "gl/GLTestContext.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLUtil.h"

// skbug.com/5932
static void test_basic_draw_as_src(skiatest::Reporter* reporter, GrContext* context,
                                   sk_sp<GrTextureProxy> rectProxy, uint32_t expectedPixelValues[]) {
    sk_sp<GrRenderTargetContext> rtContext(
            context->makeDeferredRenderTargetContext(SkBackingFit::kExact, rectProxy->width(),
                                                     rectProxy->height(), rectProxy->config(),
                                                     nullptr));
    for (auto filter : {GrSamplerParams::kNone_FilterMode,
                        GrSamplerParams::kBilerp_FilterMode,
                        GrSamplerParams::kMipMap_FilterMode}) {
        rtContext->clear(nullptr, 0xDDCCBBAA, true);
        sk_sp<GrFragmentProcessor> fp(GrSimpleTextureEffect::Make(
                                                        rectProxy,
                                                        nullptr,
                                                        SkMatrix::I(), filter));
        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.addColorFragmentProcessor(std::move(fp));
        rtContext->drawPaint(GrNoClip(), std::move(paint), SkMatrix::I());
        test_read_pixels(reporter, rtContext.get(), expectedPixelValues,
                         "RectangleTexture-basic-draw");
    }
}

static void test_clear(skiatest::Reporter* reporter, GrSurfaceContext* rectContext) {
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

        test_read_pixels(reporter, rtc, expectedPixels.get(), "RectangleTexture-clear");
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

    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin }) {
        bool useBLOrigin = kBottomLeft_GrSurfaceOrigin == origin;

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

        GrBackendTexture rectangleTex(kWidth, kHeight, kRGBA_8888_GrPixelConfig, rectangleInfo);

        GrColor refPixels[kWidth * kHeight];
        for (int y = 0; y < kHeight; ++y) {
            for (int x = 0; x < kWidth; ++x) {
                int y0 = useBLOrigin ? kHeight - y - 1 : y;
                refPixels[y * kWidth + x] = pixels[y0 * kWidth + x];
            }
        }

        sk_sp<GrTextureProxy> rectProxy = GrSurfaceProxy::MakeWrappedBackend(context,
                                                                             rectangleTex,
                                                                             origin);
        if (!rectProxy) {
            ERRORF(reporter, "Error creating proxy for rectangle texture.");
            GR_GL_CALL(glContext->gl(), DeleteTextures(1, &rectTexID));
            continue;
        }

        test_basic_draw_as_src(reporter, context, rectProxy, refPixels);

        // Test copy to both a texture and RT
        test_copy_from_surface(reporter, context, rectProxy.get(), refPixels,
                               false, "RectangleTexture-copy-from");

        sk_sp<GrSurfaceContext> rectContext = context->contextPriv().makeWrappedSurfaceContext(
                                                                    std::move(rectProxy), nullptr);
        SkASSERT(rectContext);

        test_read_pixels(reporter, rectContext.get(), refPixels, "RectangleTexture-read");

        test_copy_to_surface(reporter, context->resourceProvider(),
                              rectContext.get(), "RectangleTexture-copy-to");

        test_write_pixels(reporter, rectContext.get(), true, "RectangleTexture-write");

        test_clear(reporter, rectContext.get());

        GR_GL_CALL(glContext->gl(), DeleteTextures(1, &rectTexID));
    }
}

#endif
