/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"

#include <utility>

// This GM exercises the SkImageSource ImageFilter class.

static void fill_rect_filtered(SkCanvas* canvas,
                               const SkRect& clipRect,
                               sk_sp<SkImageFilter> filter) {
    SkPaint paint;
    paint.setImageFilter(std::move(filter));
    canvas->save();
    canvas->clipRect(clipRect);
    canvas->drawPaint(paint);
    canvas->restore();
}

class ImageSourceGM : public skiagm::GM {
public:
    ImageSourceGM() { }

protected:
    SkString getName() const override { return SkString("imagesource"); }

    SkISize getISize() override { return SkISize::Make(500, 150); }

    void onOnceBeforeDraw() override {
        fImage = ToolUtils::create_string_image(100, 100, 0xFFFFFFFF, 20, 70, 96, "e");
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        const SkRect srcRect = SkRect::MakeXYWH(20, 20, 30, 30);
        const SkRect dstRect = SkRect::MakeXYWH(0, 10, 60, 60);
        const SkRect clipRect = SkRect::MakeXYWH(0, 0, 100, 100);
        const SkRect bounds = SkRect::MakeIWH(fImage->width(), fImage->height());
        const SkSamplingOptions sampling({1/3.0f, 1/3.0f});

        {
            // Draw an unscaled bitmap.
            sk_sp<SkImageFilter> imageSource(SkImageFilters::Image(fImage, SkFilterMode::kNearest));
            fill_rect_filtered(canvas, clipRect, std::move(imageSource));
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            // Draw an unscaled subset of the source bitmap (srcRect -> srcRect).
            sk_sp<SkImageFilter> imageSourceSrcRect(
                    SkImageFilters::Image(fImage, srcRect, srcRect, sampling));
            fill_rect_filtered(canvas, clipRect, std::move(imageSourceSrcRect));
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            // Draw a subset of the bitmap scaled to a destination rect (srcRect -> dstRect).
            sk_sp<SkImageFilter> imageSourceSrcRectDstRect(
                    SkImageFilters::Image(fImage, srcRect, dstRect, sampling));
            fill_rect_filtered(canvas, clipRect, std::move(imageSourceSrcRectDstRect));
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            // Draw the entire bitmap scaled to a destination rect (bounds -> dstRect).
            sk_sp<SkImageFilter> imageSourceDstRectOnly(
                    SkImageFilters::Image(fImage, bounds, dstRect, sampling));
            fill_rect_filtered(canvas, clipRect, std::move(imageSourceDstRectOnly));
            canvas->translate(SkIntToScalar(100), 0);
        }
    }

private:
    sk_sp<SkImage> fImage;
    using INHERITED = GM;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ImageSourceGM; )
