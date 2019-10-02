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
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

namespace {
struct AsyncContext {
    bool fCalled = false;
    std::unique_ptr<const SkSurface::AsyncReadResult> fResult;
};
}  // anonymous namespace

// Making this a lambda in the test functions caused:
//   "error: cannot compile this forwarded non-trivially copyable parameter yet"
// on x86/Win/Clang bot, referring to 'result'.
static void async_callback(void* c, std::unique_ptr<const SkSurface::AsyncReadResult> result) {
    auto context = static_cast<AsyncContext*>(c);
    context->fResult = std::move(result);
    context->fCalled = true;
};

// Draws the image to a surface, does a asyncRescaleAndReadPixels of the image, and then sticks
// the result in a raster image.
static sk_sp<SkImage> do_read_and_scale(SkSurface* surface, const SkIRect& srcRect,
                                        const SkImageInfo& ii, SkSurface::RescaleGamma rescaleGamma,
                                        SkFilterQuality quality) {
    auto* context = new AsyncContext();
    surface->asyncRescaleAndReadPixels(ii, srcRect, rescaleGamma, quality, async_callback, context);
    while (!context->fCalled) {
        // Only GPU should actually be asynchronous.
        SkASSERT(surface->getCanvas()->getGrContext());
        surface->getCanvas()->getGrContext()->checkAsyncWorkCompletion();
    }
    if (!context->fResult) {
        return nullptr;
    }
    SkPixmap pixmap(ii, context->fResult->data(0), context->fResult->rowBytes(0));
    auto releasePixels = [](const void*, void* c) { delete static_cast<AsyncContext*>(c); };
    return SkImage::MakeFromRaster(pixmap, releasePixels, context);
}

static sk_sp<SkImage> do_read_and_scale_yuv(SkSurface* surface, SkYUVColorSpace yuvCS,
                                            const SkIRect& srcRect, SkISize size,
                                            SkSurface::RescaleGamma rescaleGamma,
                                            SkFilterQuality quality, SkScopeExit* cleanup) {
    SkASSERT(!(size.width() & 0b1) && !(size.height() & 0b1));

    SkISize uvSize = {size.width()/2, size.height()/2};
    SkImageInfo yII  = SkImageInfo::Make(size,   kGray_8_SkColorType, kPremul_SkAlphaType);
    SkImageInfo uvII = SkImageInfo::Make(uvSize, kGray_8_SkColorType, kPremul_SkAlphaType);

    AsyncContext context;
    surface->asyncRescaleAndReadPixelsYUV420(yuvCS, SkColorSpace::MakeSRGB(), srcRect, size,
                                             rescaleGamma, quality, async_callback, &context);
    while (!context.fCalled) {
        // Only GPU should actually be asynchronous.
        SkASSERT(surface->getCanvas()->getGrContext());
        surface->getCanvas()->getGrContext()->checkAsyncWorkCompletion();
    }
    if (!context.fResult) {
        return nullptr;
    }
    auto* gr = surface->getCanvas()->getGrContext();
    GrBackendTexture backendTextures[3];

    SkPixmap yPM(yII,  context.fResult->data(0), context.fResult->rowBytes(0));
    SkPixmap uPM(uvII, context.fResult->data(1), context.fResult->rowBytes(1));
    SkPixmap vPM(uvII, context.fResult->data(2), context.fResult->rowBytes(2));

    backendTextures[0] = gr->createBackendTexture(yPM, GrRenderable::kNo, GrProtected::kNo);
    backendTextures[1] = gr->createBackendTexture(uPM, GrRenderable::kNo, GrProtected::kNo);
    backendTextures[2] = gr->createBackendTexture(vPM, GrRenderable::kNo, GrProtected::kNo);

    SkYUVAIndex indices[4] = {
        { 0, SkColorChannel::kR},
        { 1, SkColorChannel::kR},
        { 2, SkColorChannel::kR},
        {-1, SkColorChannel::kR}
    };

    *cleanup = {[gr, backendTextures] {
        GrFlushInfo flushInfo;
        flushInfo.fFlags = kSyncCpu_GrFlushFlag;
        gr->flush(flushInfo);
        gr->deleteBackendTexture(backendTextures[0]);
        gr->deleteBackendTexture(backendTextures[1]);
        gr->deleteBackendTexture(backendTextures[2]);
    }};

    return SkImage::MakeFromYUVATextures(gr, yuvCS, backendTextures, indices, size,
                                         kTopLeft_GrSurfaceOrigin, SkColorSpace::MakeSRGB());
}

// Draws a grid of rescales. The columns are none, low, and high filter quality. The rows are
// rescale in src gamma and rescale in linear gamma.
static skiagm::DrawResult do_rescale_grid(SkCanvas* canvas, SkSurface* surface,
                                          const SkIRect& srcRect, SkISize newSize, bool doYUV420,
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
    const auto ii = canvas->imageInfo().makeDimensions(newSize);

    SkYUVColorSpace yuvColorSpace = kRec601_SkYUVColorSpace;
    canvas->save();
    for (auto gamma : {SkSurface::RescaleGamma::kSrc, SkSurface::RescaleGamma::kLinear}) {
        canvas->save();
        for (auto quality : {kNone_SkFilterQuality, kLow_SkFilterQuality, kHigh_SkFilterQuality}) {
            SkScopeExit cleanup;
            sk_sp<SkImage> result;
            if (doYUV420) {
                result = do_read_and_scale_yuv(surface, yuvColorSpace, srcRect, newSize, gamma,
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
            canvas->translate(newSize.width() + pad, 0);
        }
        canvas->restore();
        canvas->translate(0, newSize.height() + pad);
    }
    canvas->restore();
    return skiagm::DrawResult::kOk;
}

static skiagm::DrawResult do_rescale_image_grid(SkCanvas* canvas, const char* imageFile,
                                                const SkIRect& srcRect, SkISize newSize,
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
    auto surfInfo = image->imageInfo().makeDimensions(image->dimensions());
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
    return do_rescale_grid(canvas, surface.get(), srcRect, newSize, doYUV420, errorMsg);
}

#define DEF_RESCALE_AND_READ_GM(IMAGE_FILE, TAG, SRC_RECT, W, H)                              \
    DEF_SIMPLE_GM_CAN_FAIL(async_rescale_and_read_##TAG, canvas, errorMsg, 3 * W, 2 * H) {    \
        ToolUtils::draw_checkerboard(canvas, SK_ColorDKGRAY, SK_ColorLTGRAY, 25);             \
        return do_rescale_image_grid(canvas, #IMAGE_FILE, SRC_RECT, {W, H}, false, errorMsg); \
    }

#define DEF_RESCALE_AND_READ_YUV_GM(IMAGE_FILE, TAG, SRC_RECT, W, H)                              \
    DEF_SIMPLE_GM_CAN_FAIL(async_rescale_and_read_yuv420_##TAG, canvas, errorMsg, 3 * W, 2 * H) { \
        ToolUtils::draw_checkerboard(canvas, SK_ColorDKGRAY, SK_ColorLTGRAY, 25);                 \
        return do_rescale_image_grid(canvas, #IMAGE_FILE, SRC_RECT, {W, H}, true, errorMsg);      \
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
    SkISize downSize = {static_cast<int>(kInner/2),  static_cast<int>(kInner / 2)};
    result = do_rescale_grid(canvas, surface.get(), srcRect, downSize, false, errorMsg, kPad);
    if (result != skiagm::DrawResult::kOk) {
        return result;
    }
    canvas->translate(0, 2 * downSize.height());
    SkISize upSize = {static_cast<int>(kInner * 3.5), static_cast<int>(kInner * 4.6)};
    result = do_rescale_grid(canvas, surface.get(), srcRect, upSize, false, errorMsg, kPad);
    if (result != skiagm::DrawResult::kOk) {
        return result;
    }
    return skiagm::DrawResult::kOk;
}
