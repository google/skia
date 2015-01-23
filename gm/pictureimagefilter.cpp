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
    SkString onShortName() SK_OVERRIDE {
        return SkString("pictureimagefilter");
    }

    void makePicture() {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(100, 100, NULL, 0);
        canvas->clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setColor(0xFFFFFFFF);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "e";
        canvas->drawText(str, strlen(str), SkIntToScalar(20), SkIntToScalar(70), paint);
        fPicture.reset(recorder.endRecording());
    }

    SkISize onISize() SK_OVERRIDE { return SkISize::Make(600, 300); }

    void onOnceBeforeDraw() SK_OVERRIDE {
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

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->clear(0x00000000);
        {
            SkRect srcRect = SkRect::MakeXYWH(20, 20, 30, 30);
            SkRect emptyRect = SkRect::MakeXYWH(20, 20, 0, 0);
            SkRect bounds = SkRect::MakeXYWH(0, 0, 100, 100);
            SkAutoTUnref<SkPictureImageFilter> pictureSource(
                SkPictureImageFilter::Create(fPicture));
            SkAutoTUnref<SkPictureImageFilter> pictureSourceSrcRect(
                SkPictureImageFilter::Create(fPicture, srcRect));
            SkAutoTUnref<SkPictureImageFilter> pictureSourceEmptyRect(
                SkPictureImageFilter::Create(fPicture, emptyRect));
            SkAutoTUnref<SkPictureImageFilter> pictureSourceResampled(
                SkPictureImageFilter::CreateForLocalSpace(fPicture, fPicture->cullRect(),
                    SkPaint::kLow_FilterLevel));
            SkAutoTUnref<SkPictureImageFilter> pictureSourcePixelated(
                SkPictureImageFilter::CreateForLocalSpace(fPicture, fPicture->cullRect(),
                    SkPaint::kNone_FilterLevel));

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
