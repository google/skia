/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAIndex.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkScopeExit.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

// Draws the image to a surface, does a asyncRescaleAndReadPixels of the image, and then sticks
// the result in a raster image.
static sk_sp<SkImage> do_read_and_scale(SkSurface* surface, const SkIRect& srcRect,
                                        const SkImageInfo& ii, SkSurface::RescaleGamma rescaleGamma,
                                        SkFilterQuality quality) {
    SkBitmap bmp;
    bmp.allocPixels(ii);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    struct Context {
        SkPixmap fPixmap;
        bool fCalled = false;
        bool fSucceeded = false;
    } context;
    SkAssertResult(bmp.peekPixels(&context.fPixmap));
    auto callback = [](void* c, const void* data, size_t rowBytes) {
        auto context = reinterpret_cast<Context*>(c);
        context->fCalled = true;
        if (!data) {
            context->fPixmap.reset();
            return;
        }
        context->fSucceeded = true;
        SkRectMemcpy(context->fPixmap.writable_addr(), context->fPixmap.rowBytes(), data, rowBytes,
                     context->fPixmap.info().minRowBytes(), context->fPixmap.height());
    };
    surface->asyncRescaleAndReadPixels(ii, srcRect, rescaleGamma, quality, callback, &context);
    while (!context.fCalled) {
        // Only GPU should actually be asynchronous.
        SkASSERT(surface->getCanvas()->getGrContext());
        surface->getCanvas()->getGrContext()->checkAsyncWorkCompletion();
    }
    return context.fSucceeded ? SkImage::MakeFromBitmap(bmp) : nullptr;
}

static sk_sp<SkImage> do_read_and_scale_yuv(SkSurface* surface, SkYUVColorSpace yuvCS,
                                            const SkIRect& srcRect, int dstW, int dstH,
                                            SkSurface::RescaleGamma rescaleGamma,
                                            SkFilterQuality quality, SkScopeExit* cleanup) {
    SkASSERT(!(dstW & 0b1) && !(dstH & 0b1));
    std::unique_ptr<uint8_t[]> yData(new uint8_t[dstW * dstH]);
    std::unique_ptr<uint8_t[]> uData(new uint8_t[dstW / 2 * dstH / 2]);
    std::unique_ptr<uint8_t[]> vData(new uint8_t[dstW / 2 * dstH / 2]);
    struct Context {
        int fW;
        int fH;
        uint8_t* fYData;
        uint8_t* fUData;
        uint8_t* fVData;
        bool fCalled = false;
        bool fSucceeded = false;
    } context{dstW, dstH, yData.get(), uData.get(), vData.get()};
    auto callback = [](void* c, const void* data[2], size_t rowBytes[2]) {
        auto context = reinterpret_cast<Context*>(c);
        context->fCalled = true;
        if (!data) {
            return;
        }
        context->fSucceeded = true;
        int w = context->fW;
        int h = context->fH;
        SkRectMemcpy(context->fYData, w, data[0], rowBytes[0], w, h);
        SkRectMemcpy(context->fUData, w / 2, data[1], rowBytes[1], w / 2, h / 2);
        SkRectMemcpy(context->fVData, w / 2, data[2], rowBytes[2], w / 2, h / 2);
    };
    surface->asyncRescaleAndReadPixelsYUV420(yuvCS, SkColorSpace::MakeSRGB(), srcRect, dstW, dstH,
                                             rescaleGamma, quality, callback, &context);
    while (!context.fCalled) {
        // Only GPU should actually be asynchronous.
        SkASSERT(surface->getCanvas()->getGrContext());
        surface->getCanvas()->getGrContext()->checkAsyncWorkCompletion();
    }
    if (!context.fSucceeded) {
        return nullptr;
    }
    auto* gr = surface->getCanvas()->getGrContext();
    GrBackendTexture backendTextures[3];
    GrBackendFormat format =
            gr->priv().caps()->getBackendFormatFromColorType(GrColorType::kAlpha_8);
    backendTextures[0] = gr->priv().getGpu()->createBackendTexture(
            dstW, dstH, format, GrMipMapped::kNo, GrRenderable::kNo, yData.get(), 0, nullptr,
            GrProtected::kNo);
    backendTextures[1] = gr->priv().getGpu()->createBackendTexture(
            dstW / 2, dstH / 2, format, GrMipMapped::kNo, GrRenderable::kNo,
            uData.get(), 0, nullptr, GrProtected::kNo);
    backendTextures[2] = gr->priv().getGpu()->createBackendTexture(
            dstW / 2, dstH / 2, format, GrMipMapped::kNo, GrRenderable::kNo,
            vData.get(), 0, nullptr, GrProtected::kNo);
    auto config = gr->priv().caps()->getConfigFromBackendFormat(format, GrColorType::kAlpha_8);
    SkColorChannel channel;
    if (config == kAlpha_8_as_Red_GrPixelConfig) {
        channel = SkColorChannel::kR;
    } else {
        SkASSERT(config == kAlpha_8_as_Alpha_GrPixelConfig);
        channel = SkColorChannel::kA;
    }
    SkYUVAIndex indices[4]{{0, channel}, {1, channel}, {2, channel}, {-1, SkColorChannel::kR}};
    *cleanup = {[gr, backendTextures] {
        GrFlushInfo flushInfo;
        flushInfo.fFlags = kSyncCpu_GrFlushFlag;
        gr->flush(flushInfo);
        gr->deleteBackendTexture(backendTextures[0]);
        gr->deleteBackendTexture(backendTextures[1]);
        gr->deleteBackendTexture(backendTextures[2]);
    }};

    return SkImage::MakeFromYUVATextures(gr, yuvCS, backendTextures, indices, {dstW, dstH},
                                         kTopLeft_GrSurfaceOrigin, SkColorSpace::MakeSRGB());
}

// Draws a grid of rescales. The columns are none, low, and high filter quality. The rows are
// rescale in src gamma and rescale in linear gamma.
static skiagm::DrawResult do_rescale_grid(SkCanvas* canvas, SkSurface* surface,
                                          const SkIRect& srcRect, int newW, int newH, bool doYUV420,
                                          SkString* errorMsg, int pad = 0) {
    if (doYUV420) {
        if (!canvas->getGrContext() || !canvas->getGrContext()->priv().asDirectContext()) {
            errorMsg->printf("YUV420 only supported on direct GPU for now.");
            return skiagm::DrawResult::kSkip;
        }
    }
    if (canvas->imageInfo().colorType() == kUnknown_SkColorType) {
        *errorMsg = "Not supported on recording/vector backends.";
        return skiagm::DrawResult::kSkip;
    }
    const auto ii = canvas->imageInfo().makeWH(newW, newH);

    SkYUVColorSpace yuvColorSpace = kRec601_SkYUVColorSpace;
    canvas->save();
    for (auto gamma : {SkSurface::RescaleGamma::kSrc, SkSurface::RescaleGamma::kLinear}) {
        canvas->save();
        for (auto quality : {kNone_SkFilterQuality, kLow_SkFilterQuality, kHigh_SkFilterQuality}) {
            SkScopeExit cleanup;
            sk_sp<SkImage> result;
            if (doYUV420) {
                result = do_read_and_scale_yuv(surface, yuvColorSpace, srcRect, newW, newH, gamma,
                                               quality, &cleanup);
                if (!result) {
                    errorMsg->printf("YUV420 async call failed. Allowed for now.");
                    return skiagm::DrawResult::kSkip;
                }
                int nextCS = static_cast<int>(yuvColorSpace + 1) % (kLastEnum_SkYUVColorSpace + 1);
                yuvColorSpace = static_cast<SkYUVColorSpace>(nextCS);
            } else {
                result = do_read_and_scale(surface, srcRect, ii, gamma, quality);
                if (!result) {
                    errorMsg->printf("async read call failed.");
                    return skiagm::DrawResult::kFail;
                }
            }
            canvas->drawImage(result, 0, 0);
            canvas->translate(newW + pad, 0);
        }
        canvas->restore();
        canvas->translate(0, newH + pad);
    }
    canvas->restore();
    return skiagm::DrawResult::kOk;
}

static skiagm::DrawResult do_rescale_image_grid(SkCanvas* canvas, const char* imageFile,
                                                const SkIRect& srcRect, int newW, int newH,
                                                bool doYUV420, SkString* errorMsg) {
    auto image = GetResourceAsImage(imageFile);
    if (!image) {
        errorMsg->printf("Could not load image file %s.", imageFile);
        return skiagm::DrawResult::kFail;
    }
    if (canvas->imageInfo().colorType() == kUnknown_SkColorType) {
        *errorMsg = "Not supported on recording/vector backends.";
        return skiagm::DrawResult::kSkip;
    }
    // Turn the image into a surface in order to call the read and rescale API
    auto surfInfo = image->imageInfo().makeWH(image->width(), image->height());
    auto surface = canvas->makeSurface(surfInfo);
    if (!surface && surfInfo.colorType() == kBGRA_8888_SkColorType) {
        surfInfo = surfInfo.makeColorType(kRGBA_8888_SkColorType);
        surface = canvas->makeSurface(surfInfo);
    }
    if (!surface) {
        *errorMsg = "Could not create surface for image.";
        // When testing abandoned GrContext we expect surface creation to fail.
        if (canvas->getGrContext() && canvas->getGrContext()->abandoned()) {
            return skiagm::DrawResult::kSkip;
        }
        return skiagm::DrawResult::kFail;
    }
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    surface->getCanvas()->drawImage(image, 0, 0, &paint);
    return do_rescale_grid(canvas, surface.get(), srcRect, newW, newH, doYUV420, errorMsg);
}

#define DEF_RESCALE_AND_READ_GM(IMAGE_FILE, TAG, SRC_RECT, W, H)                            \
    DEF_SIMPLE_GM_CAN_FAIL(async_rescale_and_read_##TAG, canvas, errorMsg, 3 * W, 2 * H) {  \
        ToolUtils::draw_checkerboard(canvas, SK_ColorDKGRAY, SK_ColorLTGRAY, 25);           \
        return do_rescale_image_grid(canvas, #IMAGE_FILE, SRC_RECT, W, H, false, errorMsg); \
    }

#define DEF_RESCALE_AND_READ_YUV_GM(IMAGE_FILE, TAG, SRC_RECT, W, H)                               \
    DEF_SIMPLE_GM_CAN_FAIL(async_rescale_and_read_yuv420_##TAG, canvas, errorMsg, 3 * W, 2 * H) { \
        ToolUtils::draw_checkerboard(canvas, SK_ColorDKGRAY, SK_ColorLTGRAY, 25);                  \
        return do_rescale_image_grid(canvas, #IMAGE_FILE, SRC_RECT, W, H, true, errorMsg);         \
    }

DEF_RESCALE_AND_READ_YUV_GM(images/yellow_rose.webp, rose, SkIRect::MakeXYWH(50, 5, 200, 150),
                            410, 376)

DEF_RESCALE_AND_READ_GM(images/yellow_rose.webp, rose, SkIRect::MakeXYWH(100, 20, 100, 100),
                        410, 410)

DEF_RESCALE_AND_READ_GM(images/dog.jpg, dog_down, SkIRect::MakeXYWH(0, 10, 180, 150), 45, 45)
DEF_RESCALE_AND_READ_GM(images/dog.jpg, dog_up, SkIRect::MakeWH(180, 180), 800, 400)

DEF_RESCALE_AND_READ_GM(images/text.png, text_down, SkIRect::MakeWH(637, 105), (int)(0.7 * 637),
                        (int)(0.7 * 105))
DEF_RESCALE_AND_READ_GM(images/text.png, text_up, SkIRect::MakeWH(637, 105), (int)(1.2 * 637),
                        (int)(1.2 * 105))
DEF_RESCALE_AND_READ_GM(images/text.png, text_up_large, SkIRect::MakeXYWH(300, 0, 300, 105),
                        (int)(2.4 * 300), (int)(2.4 * 105))

DEF_SIMPLE_GM_CAN_FAIL(async_rescale_and_read_no_bleed, canvas, errorMsg, 60, 60) {
    if (canvas->imageInfo().colorType() == kUnknown_SkColorType) {
        *errorMsg = "Not supported on recording/vector backends.";
        return skiagm::DrawResult::kSkip;
    }

    static constexpr int kBorder = 5;
    static constexpr int kInner = 5;
    const auto srcRect = SkIRect::MakeXYWH(kBorder, kBorder, kInner, kInner);
    auto surfaceII =
            SkImageInfo::Make(kInner + 2 * kBorder, kInner + 2 * kBorder, kRGBA_8888_SkColorType,
                              kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
    auto surface = canvas->makeSurface(surfaceII);
    if (!surface) {
        *errorMsg = "Could not create surface for image.";
        // When testing abandoned GrContext we expect surface creation to fail.
        if (canvas->getGrContext() && canvas->getGrContext()->abandoned()) {
            return skiagm::DrawResult::kSkip;
        }
        return skiagm::DrawResult::kFail;
    }
    surface->getCanvas()->clear(SK_ColorRED);
    surface->getCanvas()->save();
    surface->getCanvas()->clipRect(SkRect::Make(srcRect), SkClipOp::kIntersect, false);
    surface->getCanvas()->clear(SK_ColorBLUE);
    surface->getCanvas()->restore();
    static constexpr int kPad = 2;
    canvas->translate(kPad, kPad);
    skiagm::DrawResult result;
    auto downW = static_cast<int>(kInner / 2);
    auto downH = static_cast<int>(kInner / 2);
    result = do_rescale_grid(canvas, surface.get(), srcRect, downW, downH, false, errorMsg, kPad);
    if (result != skiagm::DrawResult::kOk) {
        return result;
    }
    canvas->translate(0, 2 * downH);
    auto upW = static_cast<int>(kInner * 3.5);
    auto upH = static_cast<int>(kInner * 4.6);
    result = do_rescale_grid(canvas, surface.get(), srcRect, upW, upH, false, errorMsg, kPad);
    if (result != skiagm::DrawResult::kOk) {
        return result;
    }
    return skiagm::DrawResult::kOk;
}
