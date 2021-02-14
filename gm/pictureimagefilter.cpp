/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"

// This GM exercises the SkPictureImageFilter ImageFilter class.

static void fill_rect_filtered(SkCanvas* canvas,
                               const SkRect& clipRect,
                               sk_sp<SkImageFilter> filter) {
    SkPaint paint;
    paint.setImageFilter(filter);
    canvas->save();
    canvas->clipRect(clipRect);
    canvas->drawPaint(paint);
    canvas->restore();
}

static sk_sp<SkPicture> make_picture() {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(100, 100);
    SkPaint paint;
    paint.setColor(0xFFFFFFFF);
    SkFont font(ToolUtils::create_portable_typeface(), 96.0f);
    canvas->drawString("e", 20.0f, 70.0f, font, paint);
    return recorder.finishRecordingAsPicture();
}

// Create a picture that will draw LCD text
static sk_sp<SkPicture> make_LCD_picture() {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(100, 100);
    canvas->clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    paint.setColor(0xFFFFFFFF);
    // this has to be small enough that it doesn't become a path
    SkFont font(ToolUtils::create_portable_typeface(), 36.0f);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
    canvas->drawString("e", 20.0f, 70.0f, font, paint);
    return recorder.finishRecordingAsPicture();
}

class PictureImageFilterGM : public skiagm::GM {
public:
    PictureImageFilterGM() { }

protected:
    SkString onShortName() override {
        return SkString("pictureimagefilter");
    }

    SkISize onISize() override { return SkISize::Make(600, 300); }

    void onOnceBeforeDraw() override {
        fPicture = make_picture();
        fLCDPicture = make_LCD_picture();
    }

    sk_sp<SkImageFilter> make(sk_sp<SkPicture> pic, SkRect r, const SkSamplingOptions& sampling) {
        SkISize dim = { SkScalarRoundToInt(r.width()), SkScalarRoundToInt(r.height()) };
        auto img = SkImage::MakeFromPicture(pic, dim, nullptr, nullptr,
                                            SkImage::BitDepth::kU8, SkColorSpace::MakeSRGB());
        return SkImageFilters::Image(img, r, r, sampling);
    }
    sk_sp<SkImageFilter> make(const SkSamplingOptions& sampling) {
        return make(fPicture, fPicture->cullRect(), sampling);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorGRAY);
        {
            SkRect srcRect = SkRect::MakeXYWH(20, 20, 30, 30);
            SkRect emptyRect = SkRect::MakeXYWH(20, 20, 0, 0);
            SkRect bounds = SkRect::MakeXYWH(0, 0, 100, 100);
            sk_sp<SkImageFilter> pictureSource(SkImageFilters::Picture(fPicture));
            sk_sp<SkImageFilter> pictureSourceSrcRect(SkImageFilters::Picture(fPicture, srcRect));
            sk_sp<SkImageFilter> pictureSourceEmptyRect(SkImageFilters::Picture(fPicture,
                                                                                emptyRect));
            sk_sp<SkImageFilter> pictureSourceResampled = make(SkSamplingOptions(SkFilterMode::kLinear));
            sk_sp<SkImageFilter> pictureSourcePixelated = make(SkSamplingOptions());

            canvas->save();
            // Draw the picture unscaled.
            fill_rect_filtered(canvas, bounds, pictureSource);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw an unscaled subset of the source picture.
            fill_rect_filtered(canvas, bounds, pictureSourceSrcRect);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw the picture to an empty rect (should draw nothing).
            fill_rect_filtered(canvas, bounds, pictureSourceEmptyRect);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw the LCD picture to a layer
            {
                SkPaint stroke;
                stroke.setStyle(SkPaint::kStroke_Style);

                canvas->drawRect(bounds, stroke);

                SkPaint paint;
                paint.setImageFilter(make(fLCDPicture, fPicture->cullRect(), SkSamplingOptions()));

                canvas->scale(4, 4);
                canvas->translate(-0.9f*srcRect.fLeft, -2.45f*srcRect.fTop);

                canvas->saveLayer(&bounds, &paint);
                canvas->restore();
            }

            canvas->restore();

            // Draw the picture scaled
            canvas->translate(0, SkIntToScalar(100));
            canvas->scale(200 / srcRect.width(), 200 / srcRect.height());
            canvas->translate(-srcRect.fLeft, -srcRect.fTop);
            fill_rect_filtered(canvas, srcRect, pictureSource);

            // Draw the picture scaled, but rasterized at original resolution
            canvas->translate(srcRect.width(), 0);
            fill_rect_filtered(canvas, srcRect, pictureSourceResampled);

            // Draw the picture scaled, pixelated
            canvas->translate(srcRect.width(), 0);
            fill_rect_filtered(canvas, srcRect, pictureSourcePixelated);
        }
    }

private:
    sk_sp<SkPicture> fPicture;
    sk_sp<SkPicture> fLCDPicture;

    using INHERITED = GM;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new PictureImageFilterGM; )
