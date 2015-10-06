/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkPictureImageFilter.h"
#include "SkPictureRecorder.h"

// This GM exercises the SkPictureImageFilter ImageFilter class.

class PictureImageFilterGM : public skiagm::GM {
public:
    PictureImageFilterGM() {
    }

protected:
    SkString onShortName() override {
        return SkString("pictureimagefilter");
    }

    void makePicture() {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(100, 100, nullptr, 0);
        canvas->clear(SK_ColorBLACK);
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "e";
        canvas->drawText(str, strlen(str), SkIntToScalar(20), SkIntToScalar(70), paint);
        fPicture.reset(recorder.endRecording());
    }

    SkISize onISize() override { return SkISize::Make(600, 300); }

    void onOnceBeforeDraw() override {
        this->makePicture();
    }

    static void fillRectFiltered(SkCanvas* canvas, const SkRect& clipRect, SkImageFilter* filter) {
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
            SkRect emptyRect = SkRect::MakeXYWH(20, 20, 0, 0);
            SkRect bounds = SkRect::MakeXYWH(0, 0, 100, 100);
            SkAutoTUnref<SkImageFilter> pictureSource(
                SkPictureImageFilter::Create(fPicture));
            SkAutoTUnref<SkImageFilter> pictureSourceSrcRect(
                SkPictureImageFilter::Create(fPicture, srcRect));
            SkAutoTUnref<SkImageFilter> pictureSourceEmptyRect(
                SkPictureImageFilter::Create(fPicture, emptyRect));
            SkAutoTUnref<SkImageFilter> pictureSourceResampled(
                SkPictureImageFilter::CreateForLocalSpace(fPicture, fPicture->cullRect(),
                    kLow_SkFilterQuality));
            SkAutoTUnref<SkImageFilter> pictureSourcePixelated(
                SkPictureImageFilter::CreateForLocalSpace(fPicture, fPicture->cullRect(),
                    kNone_SkFilterQuality));

            canvas->save();
            // Draw the picture unscaled.
            fillRectFiltered(canvas, bounds, pictureSource);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw an unscaled subset of the source picture.
            fillRectFiltered(canvas, bounds, pictureSourceSrcRect);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw the picture to an empty rect (should draw nothing).
            fillRectFiltered(canvas, bounds, pictureSourceEmptyRect);
            canvas->translate(SkIntToScalar(100), 0);

            canvas->restore();

            // Draw the picture scaled
            canvas->translate(0, SkIntToScalar(100));
            canvas->scale(200 / srcRect.width(), 200 / srcRect.height());
            canvas->translate(-srcRect.fLeft, -srcRect.fTop);
            fillRectFiltered(canvas, srcRect, pictureSource);

            // Draw the picture scaled, but rasterized at original resolution
            canvas->translate(srcRect.width(), 0);
            fillRectFiltered(canvas, srcRect, pictureSourceResampled);

            // Draw the picture scaled, pixelated
            canvas->translate(srcRect.width(), 0);
            fillRectFiltered(canvas, srcRect, pictureSourcePixelated);
        }
    }

private:
    SkAutoTUnref<SkPicture> fPicture;
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new PictureImageFilterGM; )
