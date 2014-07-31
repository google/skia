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
    virtual SkString onShortName() SK_OVERRIDE {
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

    virtual SkISize onISize() SK_OVERRIDE { return SkISize::Make(500, 150); }

    virtual void onOnceBeforeDraw() SK_OVERRIDE {
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

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->clear(0x00000000);
        {
            SkRect srcRect = SkRect::MakeXYWH(20, 20, 30, 30);
            SkRect emptyRect = SkRect::MakeXYWH(20, 20, 0, 0);
            SkRect bounds = SkRect::MakeXYWH(0, 0, 100, 100);
            SkAutoTUnref<SkImageFilter> pictureSource(SkPictureImageFilter::Create(fPicture));
            SkAutoTUnref<SkImageFilter> pictureSourceSrcRect(SkPictureImageFilter::Create(fPicture, srcRect));
            SkAutoTUnref<SkImageFilter> pictureSourceEmptyRect(SkPictureImageFilter::Create(fPicture, emptyRect));

            // Draw the picture unscaled.
            fillRectFiltered(canvas, bounds, pictureSource);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw an unscaled subset of the source picture.
            fillRectFiltered(canvas, bounds, pictureSourceSrcRect);
            canvas->translate(SkIntToScalar(100), 0);

            // Draw the picture to an empty rect (should draw nothing).
            fillRectFiltered(canvas, bounds, pictureSourceEmptyRect);
            canvas->translate(SkIntToScalar(100), 0);
        }
    }

    // SkPictureImageFilter doesn't support serialization yet.
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        return kSkipPicture_Flag            |
               kSkipPipe_Flag               |
               kSkipPipeCrossProcess_Flag   |
               kSkipTiled_Flag              |
               kSkipScaledReplay_Flag;
    }

private:
    SkAutoTUnref<SkPicture> fPicture;
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new PictureImageFilterGM; )
