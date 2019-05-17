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
static sk_sp<SkImage> do_read_and_scale(
        SkImage* image, const SkIRect& srcRect, const SkImageInfo& ii,
        SkSurface::RescaleGamma rescaleGamma, SkFilterQuality quality,
        std::function<sk_sp<SkSurface>(const SkImageInfo&)> makeSurface) {
    SkBitmap bmp;
    bmp.allocPixels(ii);
    // Turn the image into a surface in order to call the read and rescale API
    auto surf = makeSurface(image->imageInfo().makeWH(image->width(), image->height()));
    if (!surf) {
        return nullptr;
    }
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    surf->getCanvas()->drawImage(image, 0, 0, &paint);
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
    surf->asyncRescaleAndReadPixels(ii, srcRect, rescaleGamma, quality, callback, &context);
    while (!context.fCalled) {
        // Only GPU should actually be asynchronous.
        SkASSERT(surf->getCanvas()->getGrContext());
        surf->getCanvas()->getGrContext()->checkAsyncWorkCompletion();
    }
    return SkImage::MakeFromBitmap(bmp);
}

// Draws a grid of rescales. The columns are none, low, and high filter quality. The rows are
// rescale in src gamma and rescale in linear gamma.
static skiagm::DrawResult do_rescale_grid(SkCanvas* canvas, const char* imageFile,
                                          const SkIRect& srcRect, int newW, int newH,
                                          SkString* errorMsg) {
    if (canvas->imageInfo().colorType() == kUnknown_SkColorType) {
        *errorMsg = "Not supported on recording/vector backends.";
        return skiagm::DrawResult::kSkip;
    }
    auto image = GetResourceAsImage(imageFile);
    if (!image) {
        errorMsg->printf("Could not load image file %s.", imageFile);
        return skiagm::DrawResult::kFail;
    }
    const auto ii = canvas->imageInfo().makeWH(newW, newH);
    auto makeSurface = [canvas](const SkImageInfo& info) { return canvas->makeSurface(info); };

    canvas->save();
    for (auto linear : {SkSurface::RescaleGamma::kSrc, SkSurface::RescaleGamma::kLinear}) {
        canvas->save();
        for (auto quality : {kNone_SkFilterQuality, kLow_SkFilterQuality, kHigh_SkFilterQuality}) {
            auto rescaled =
                    do_read_and_scale(image.get(), srcRect, ii, linear, quality, makeSurface);
            canvas->drawImage(rescaled, 0, 0);
            canvas->translate(newW, 0);
        }
        canvas->restore();
        canvas->translate(0, newH);
    }
    canvas->restore();
    return skiagm::DrawResult::kOk;
}

#define DEF_RESCALE_AND_READ_GM(IMAGE_FILE, TAG, SRC_RECT, W, H)                           \
    DEF_SIMPLE_GM_CAN_FAIL(async_rescale_and_read_##TAG, canvas, errorMsg, 3 * W, 2 * H) { \
        ToolUtils::draw_checkerboard(canvas, SK_ColorDKGRAY, SK_ColorLTGRAY, 25);          \
        return do_rescale_grid(canvas, #IMAGE_FILE, SRC_RECT, W, H, errorMsg);             \
    }

DEF_RESCALE_AND_READ_GM(images/yellow_rose.webp, rose, SkIRect::MakeXYWH(100, 20, 100, 100),
                        410, 410)

DEF_RESCALE_AND_READ_GM(images/dog.jpg, dog_down, SkIRect::MakeXYWH(0, 10, 180, 150), 45, 45)
DEF_RESCALE_AND_READ_GM(images/dog.jpg, dog_up, SkIRect::MakeWH(180, 180), 680, 400)

DEF_RESCALE_AND_READ_GM(images/text.png, text_down, SkIRect::MakeWH(637, 105), (int)(0.7 * 637),
                        (int)(0.7 * 105))
DEF_RESCALE_AND_READ_GM(images/text.png, text_up, SkIRect::MakeWH(637, 105), (int)(1.2 * 637),
                        (int)(1.2 * 105))
DEF_RESCALE_AND_READ_GM(images/text.png, text_up_large, SkIRect::MakeXYWH(300, 0, 300, 105),
                        (int)(2.4 * 300), (int)(2.4 * 105))
