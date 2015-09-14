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
        fImage.reset(SkImage::NewFromBitmap(bm));
    }

    static void FillRectFiltered(SkCanvas* canvas, const SkRect& clipRect, SkImageFilter* filter) {
        SkPaint paint;
        paint.setImageFilter(filter);
        canvas->save();
        canvas->clipRect(clipRect);
        canvas->drawPaint(paint);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        {
            SkRect srcRect = SkRect::MakeXYWH(20, 20, 30, 30);
            SkRect dstRect = SkRect::MakeXYWH(0, 10, 60, 60);
            SkRect clipRect = SkRect::MakeXYWH(0, 0, 100, 100);
            SkRect bounds = SkRect::MakeIWH(fImage->width(), fImage->height());
            SkAutoTUnref<SkImageFilter> imageSource(SkImageSource::Create(fImage));
            SkAutoTUnref<SkImageFilter> imageSourceSrcRect(
                SkImageSource::Create(fImage, srcRect, srcRect, kHigh_SkFilterQuality));
            SkAutoTUnref<SkImageFilter> imageSourceSrcRectDstRect(
                SkImageSource::Create(fImage, srcRect, dstRect, kHigh_SkFilterQuality));
            SkAutoTUnref<SkImageFilter> imageSourceDstRectOnly(
                SkImageSource::Create(fImage, bounds, dstRect, kHigh_SkFilterQuality));

            // Draw an unscaled bitmap.
            FillRectFiltered(canvas, clipRect, imageSource);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw an unscaled subset of the source bitmap (srcRect -> srcRect).
            FillRectFiltered(canvas, clipRect, imageSourceSrcRect);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw a subset of the bitmap scaled to a destination rect (srcRect -> dstRect).
            FillRectFiltered(canvas, clipRect, imageSourceSrcRectDstRect);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw the entire bitmap scaled to a destination rect (bounds -> dstRect).
            FillRectFiltered(canvas, clipRect, imageSourceDstRectOnly);
            canvas->translate(SkIntToScalar(100), 0);
        }
    }

private:
    SkAutoTUnref<SkImage> fImage;
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ImageSourceGM; )
