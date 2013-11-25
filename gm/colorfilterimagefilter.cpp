/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkColorMatrixFilter.h"
#include "SkColorPriv.h"
#include "SkShader.h"

#include "SkBlurImageFilter.h"
#include "SkColorFilterImageFilter.h"

#define FILTER_WIDTH    SkIntToScalar(30)
#define FILTER_HEIGHT   SkIntToScalar(30)
#define MARGIN          SkIntToScalar(10)

static SkImageFilter* make_blur(float amount, SkImageFilter* input = NULL) {
    return new SkBlurImageFilter(amount, amount, input);
}

static SkImageFilter* make_brightness(float amount, SkImageFilter* input = NULL) {
    SkScalar amount255 = SkScalarMul(amount, SkIntToScalar(255));
    SkScalar matrix[20] = { 1, 0, 0, 0, amount255,
                            0, 1, 0, 0, amount255,
                            0, 0, 1, 0, amount255,
                            0, 0, 0, 1, 0 };
    SkAutoTUnref<SkColorFilter> filter(new SkColorMatrixFilter(matrix));
    return SkColorFilterImageFilter::Create(filter, input);
}

static SkImageFilter* make_grayscale(SkImageFilter* input = NULL) {
    SkScalar matrix[20];
    memset(matrix, 0, 20 * sizeof(SkScalar));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    SkAutoTUnref<SkColorFilter> filter(new SkColorMatrixFilter(matrix));
    return SkColorFilterImageFilter::Create(filter, input);
}

static SkImageFilter* make_mode_blue(SkImageFilter* input = NULL) {
    SkAutoTUnref<SkColorFilter> filter(
        SkColorFilter::CreateModeFilter(SK_ColorBLUE, SkXfermode::kSrcIn_Mode));
    return SkColorFilterImageFilter::Create(filter, input);
}

class ColorFilterImageFilterGM : public skiagm::GM {
public:
    ColorFilterImageFilterGM () {}

protected:

    virtual SkString onShortName() {
        return SkString("colorfilterimagefilter");
    }

    void drawClippedRect(SkCanvas* canvas, const SkRect& r, const SkPaint& paint, float outset = 0.0f) {
        canvas->save();
        SkRect clip(r);
        clip.outset(outset, outset);
        canvas->clipRect(clip);
        canvas->drawRect(r, paint);
        canvas->restore();
    }

    virtual SkISize onISize() { return SkISize::Make(400, 100); }

    virtual void onDraw(SkCanvas* canvas) {

        SkRect r = SkRect::MakeWH(FILTER_WIDTH, FILTER_HEIGHT);
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        canvas->save();
        for (float brightness = -1.0f; brightness <= 1.0f; brightness += 0.2f) {
            SkAutoTUnref<SkImageFilter> dim(make_brightness(-brightness));
            SkAutoTUnref<SkImageFilter> bright(make_brightness(brightness, dim));
            paint.setImageFilter(bright);
            drawClippedRect(canvas, r, paint);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
        canvas->restore();
        canvas->translate(0, FILTER_HEIGHT + MARGIN);
        {
            SkAutoTUnref<SkImageFilter> brightness(make_brightness(0.9f));
            SkAutoTUnref<SkImageFilter> grayscale(make_grayscale(brightness));
            paint.setImageFilter(grayscale);
            drawClippedRect(canvas, r, paint);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
        {
            SkAutoTUnref<SkImageFilter> grayscale(make_grayscale());
            SkAutoTUnref<SkImageFilter> brightness(make_brightness(0.9f, grayscale));
            paint.setImageFilter(brightness);
            drawClippedRect(canvas, r, paint);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
        {
            SkAutoTUnref<SkImageFilter> blue(make_mode_blue());
            SkAutoTUnref<SkImageFilter> brightness(make_brightness(1.0f, blue));
            paint.setImageFilter(brightness);
            drawClippedRect(canvas, r, paint);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
        {
            SkAutoTUnref<SkImageFilter> brightness(make_brightness(1.0f));
            SkAutoTUnref<SkImageFilter> blue(make_mode_blue(brightness));
            paint.setImageFilter(blue);
            drawClippedRect(canvas, r, paint);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
        {
            SkAutoTUnref<SkImageFilter> blur(make_blur(3.0f));
            SkAutoTUnref<SkImageFilter> brightness(make_brightness(0.5f, blur));
            paint.setImageFilter(brightness);
            drawClippedRect(canvas, r, paint, 3);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new ColorFilterImageFilterGM; }
static skiagm::GMRegistry reg(MyFactory);
