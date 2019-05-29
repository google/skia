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
#include "include/gpu/GrContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"
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
    } context;
    SkAssertResult(bmp.peekPixels(&context.fPixmap));
    auto callback = [](void* c, const void* data, size_t rowBytes) {
        auto context = reinterpret_cast<Context*>(c);
        context->fCalled = true;
        if (!data) {
            context->fPixmap.reset();
            return;
        }
        SkRectMemcpy(context->fPixmap.writable_addr(), context->fPixmap.rowBytes(), data, rowBytes,
                     context->fPixmap.info().minRowBytes(), context->fPixmap.height());
    };
    surface->asyncRescaleAndReadPixels(ii, srcRect, rescaleGamma, quality, callback, &context);
    while (!context.fCalled) {
        // Only GPU should actually be asynchronous.
        SkASSERT(surface->getCanvas()->getGrContext());
        surface->getCanvas()->getGrContext()->checkAsyncWorkCompletion();
    }
    return SkImage::MakeFromBitmap(bmp);
}

// Draws a grid of rescales. The columns are none, low, and high filter quality. The rows are
// rescale in src gamma and rescale in linear gamma.
static skiagm::DrawResult do_rescale_grid(SkCanvas* canvas, SkSurface* surface,
                                          const SkIRect& srcRect, int newW, int newH,
                                          SkString* errorMsg, int pad = 0) {
    if (canvas->imageInfo().colorType() == kUnknown_SkColorType) {
        *errorMsg = "Not supported on recording/vector backends.";
        return skiagm::DrawResult::kSkip;
    }
    const auto ii = canvas->imageInfo().makeWH(newW, newH);

    canvas->save();
    for (auto linear : {SkSurface::RescaleGamma::kSrc, SkSurface::RescaleGamma::kLinear}) {
        canvas->save();
        for (auto quality : {kNone_SkFilterQuality, kLow_SkFilterQuality, kHigh_SkFilterQuality}) {
            auto rescaled = do_read_and_scale(surface, srcRect, ii, linear, quality);
            canvas->drawImage(rescaled, 0, 0);
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
                                                SkString* errorMsg) {
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
    return do_rescale_grid(canvas, surface.get(), srcRect, newW, newH, errorMsg);
}

#define DEF_RESCALE_AND_READ_GM(IMAGE_FILE, TAG, SRC_RECT, W, H)                           \
    DEF_SIMPLE_GM_CAN_FAIL(async_rescale_and_read_##TAG, canvas, errorMsg, 3 * W, 2 * H) { \
        ToolUtils::draw_checkerboard(canvas, SK_ColorDKGRAY, SK_ColorLTGRAY, 25);          \
        return do_rescale_image_grid(canvas, #IMAGE_FILE, SRC_RECT, W, H, errorMsg);       \
    }

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
    result = do_rescale_grid(canvas, surface.get(), srcRect, downW, downH, errorMsg, kPad);
    if (result != skiagm::DrawResult::kOk) {
        return result;
    }
    canvas->translate(0, 2 * downH);
    auto upW = static_cast<int>(kInner * 3.5);
    auto upH = static_cast<int>(kInner * 4.6);
    result = do_rescale_grid(canvas, surface.get(), srcRect, upW, upH, errorMsg, kPad);
    if (result != skiagm::DrawResult::kOk) {
        return result;
    }
    return skiagm::DrawResult::kOk;
}
