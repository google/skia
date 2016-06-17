/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkImage.h"
#include "SkImageSource.h"

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
    SkString onShortName() override {
        return SkString("imagesource");
    }

    SkISize onISize() override { return SkISize::Make(500, 150); }

    void onOnceBeforeDraw() override {
        SkBitmap bm = sk_tool_utils::create_string_bitmap(100, 100, 0xFFFFFFFF, 20, 70, 96, "e");
        fImage = SkImage::MakeFromBitmap(bm);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        const SkRect srcRect = SkRect::MakeXYWH(20, 20, 30, 30);
        const SkRect dstRect = SkRect::MakeXYWH(0, 10, 60, 60);
        const SkRect clipRect = SkRect::MakeXYWH(0, 0, 100, 100);
        const SkRect bounds = SkRect::MakeIWH(fImage->width(), fImage->height());

        {
            // Draw an unscaled bitmap.
            sk_sp<SkImageFilter> imageSource(SkImageSource::Make(fImage));
            fill_rect_filtered(canvas, clipRect, std::move(imageSource));
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            // Draw an unscaled subset of the source bitmap (srcRect -> srcRect).
            sk_sp<SkImageFilter> imageSourceSrcRect(
                SkImageSource::Make(fImage, srcRect, srcRect, kHigh_SkFilterQuality));
            fill_rect_filtered(canvas, clipRect, std::move(imageSourceSrcRect));
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            // Draw a subset of the bitmap scaled to a destination rect (srcRect -> dstRect).
            sk_sp<SkImageFilter> imageSourceSrcRectDstRect(
                SkImageSource::Make(fImage, srcRect, dstRect, kHigh_SkFilterQuality));
            fill_rect_filtered(canvas, clipRect, std::move(imageSourceSrcRectDstRect));
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            // Draw the entire bitmap scaled to a destination rect (bounds -> dstRect).
            sk_sp<SkImageFilter> imageSourceDstRectOnly(
                SkImageSource::Make(fImage, bounds, dstRect, kHigh_SkFilterQuality));
            fill_rect_filtered(canvas, clipRect, std::move(imageSourceDstRectOnly));
            canvas->translate(SkIntToScalar(100), 0);
        }
    }

private:
    sk_sp<SkImage> fImage;
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ImageSourceGM; )
