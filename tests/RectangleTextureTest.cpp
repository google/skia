/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"
#include "tests/TestUtils.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSurfaceContextPriv.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLUtil.h"
#include "tools/gpu/ProxyUtils.h"
#include "tools/gpu/gl/GLTestContext.h"

// skbug.com/5932
static void test_basic_draw_as_src(skiatest::Reporter* reporter, GrContext* context,
                                   sk_sp<GrTextureProxy> rectProxy, uint32_t expectedPixelValues[]) {
    GrBackendFormat format = rectProxy->backendFormat().makeTexture2D();
    SkASSERT(format.isValid());
    sk_sp<GrRenderTargetContext> rtContext(context->priv().makeDeferredRenderTargetContext(
                                                     format,
                                                     SkBackingFit::kExact, rectProxy->width(),
                                                     rectProxy->height(), rectProxy->config(),
                                                     nullptr));
    for (auto filter : {GrSamplerState::Filter::kNearest,
                        GrSamplerState::Filter::kBilerp,
                        GrSamplerState::Filter::kMipMap}) {
        rtContext->clear(nullptr, SkPMColor4f::FromBytes_RGBA(0xDDCCBBAA),
                         GrRenderTargetContext::CanClearFullscreen::kYes);
        auto fp = GrSimpleTextureEffect::Make(rectProxy, SkMatrix::I(), filter);
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
        rtc->clear(nullptr, SkPMColor4f::FromBytes_RGBA(color0),
                   GrRenderTargetContext::CanClearFullscreen::kNo);

        int w = rtc->width();
        int h = rtc->height();
        int pixelCnt = w * h;
        SkAutoTMalloc<uint32_t> expectedPixels(pixelCnt);

        // The clear color is a GrColor, our readback is to kRGBA_8888, which may be different.
        uint32_t expectedColor0 = 0;
        uint8_t* expectedBytes0 = reinterpret_cast<uint8_t*>(&expectedColor0);
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
        rtc->clear(&rect, SkPMColor4f::FromBytes_RGBA(color1),
                   GrRenderTargetContext::CanClearFullscreen::kNo);

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

        test_read_pixels(reporter, rtc, expectedPixels.get(), "RectangleTexture-clear");
    }
}

static void test_copy_to_surface(skiatest::Reporter* reporter,
                                 GrContext* context,
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
        auto origin = dstContext->asSurfaceProxy()->origin();
        auto src = sk_gpu_test::MakeTextureProxyFromData(
                context, renderable, dstContext->width(),
                dstContext->height(), kRGBA_8888_SkColorType, origin, pixels.get(), 0);
        // If this assert ever fails we can add a fallback to do copy as draw, but until then we can
        // be more restrictive.
        SkAssertResult(dstContext->testCopy(src.get()));
        test_read_pixels(reporter, dstContext, pixels.get(), testName);
    }
}

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(RectangleTexture, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    sk_gpu_test::GLTestContext* glContext = ctxInfo.glContext();
    static const int kWidth = 16;
    static const int kHeight = 16;

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

        GrBackendTexture rectangleTex(kWidth, kHeight, GrMipMapped::kNo, rectangleInfo);
        rectangleTex.setPixelConfig(kRGBA_8888_GrPixelConfig);

        GrColor refPixels[kWidth * kHeight];
        for (int y = 0; y < kHeight; ++y) {
            for (int x = 0; x < kWidth; ++x) {
                int y0 = useBLOrigin ? kHeight - y - 1 : y;
                refPixels[y * kWidth + x] = pixels[y0 * kWidth + x];
            }
        }

        sk_sp<GrTextureProxy> rectProxy = proxyProvider->wrapBackendTexture(
                rectangleTex, origin, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType);

        if (!rectProxy) {
            ERRORF(reporter, "Error creating proxy for rectangle texture.");
            GR_GL_CALL(glContext->gl(), DeleteTextures(1, &rectTexID));
            continue;
        }

        SkASSERT(rectProxy->mipMapped() == GrMipMapped::kNo);
        SkASSERT(rectProxy->peekTexture()->texturePriv().mipMapped() == GrMipMapped::kNo);

        SkASSERT(rectProxy->textureType() == GrTextureType::kRectangle);
        SkASSERT(rectProxy->peekTexture()->texturePriv().textureType() ==
                 GrTextureType::kRectangle);
        SkASSERT(rectProxy->hasRestrictedSampling());
        SkASSERT(rectProxy->peekTexture()->texturePriv().hasRestrictedSampling());

        test_basic_draw_as_src(reporter, context, rectProxy, refPixels);

        // Test copy to both a texture and RT
        test_copy_from_surface(reporter, context, rectProxy.get(), refPixels,
                               "RectangleTexture-copy-from");

        sk_sp<GrSurfaceContext> rectContext = context->priv().makeWrappedSurfaceContext(
                                                                            std::move(rectProxy));
        SkASSERT(rectContext);

        test_read_pixels(reporter, rectContext.get(), refPixels, "RectangleTexture-read");

        test_copy_to_surface(reporter, context, rectContext.get(), "RectangleTexture-copy-to");

        test_write_pixels(reporter, rectContext.get(), true, "RectangleTexture-write");

        test_clear(reporter, rectContext.get());

        GR_GL_CALL(glContext->gl(), DeleteTextures(1, &rectTexID));
    }
}
