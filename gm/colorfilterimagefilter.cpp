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

static SkColorFilter* cf_make_brightness(float brightness) {
    SkScalar amount255 = SkScalarMul(brightness, SkIntToScalar(255));
    SkScalar matrix[20] = {
        1, 0, 0, 0, amount255,
        0, 1, 0, 0, amount255,
        0, 0, 1, 0, amount255,
        0, 0, 0, 1, 0 };
    return SkColorMatrixFilter::Create(matrix);
}

static SkColorFilter* cf_make_grayscale() {
    SkScalar matrix[20];
    memset(matrix, 0, 20 * sizeof(SkScalar));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    return SkColorMatrixFilter::Create(matrix);
}

static SkColorFilter* cf_make_colorize(SkColor color) {
    return SkColorFilter::CreateModeFilter(color, SkXfermode::kSrc_Mode);
}

static void sk_gm_get_colorfilters(SkTDArray<SkColorFilter*>* array) {
    *array->append() = cf_make_brightness(0.5f);
    *array->append() = cf_make_grayscale();
    *array->append() = cf_make_colorize(SK_ColorBLUE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkGradientShader.h"
#include "SkImage.h"
#include "Resources.h"

static SkShader* sh_make_lineargradient0() {
    const SkPoint pts[] = { { 0, 0 }, { 100, 100 } };
    const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    return SkGradientShader::CreateLinear(pts, colors, nullptr, 3, SkShader::kRepeat_TileMode);
}

static SkShader* sh_make_lineargradient1() {
    const SkPoint pts[] = { { 0, 0 }, { 100, 100 } };
    const SkColor colors[] = { SK_ColorRED, 0x0000FF00, SK_ColorBLUE };
    return SkGradientShader::CreateLinear(pts, colors, nullptr, 3, SkShader::kRepeat_TileMode);
}

static SkShader* sh_make_image() {
    SkAutoTUnref<SkImage> image(GetResourceAsImage("mandrill_128.png"));
    return image->newShader(SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);
}

static void sk_gm_get_shaders(SkTDArray<SkShader*>* array) {
    *array->append() = sh_make_lineargradient0();
    *array->append() = sh_make_lineargradient1();
    *array->append() = sh_make_image();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static SkImageFilter* make_blur(float amount, SkImageFilter* input = nullptr) {
    return SkBlurImageFilter::Create(amount, amount, input);
}

static SkImageFilter* make_brightness(float amount, SkImageFilter* input = nullptr) {
    SkAutoTUnref<SkColorFilter> filter(cf_make_brightness(amount));
    return SkColorFilterImageFilter::Create(filter, input);
}

static SkImageFilter* make_grayscale(SkImageFilter* input = nullptr) {
    SkAutoTUnref<SkColorFilter> filter(cf_make_grayscale());
    return SkColorFilterImageFilter::Create(filter, input);
}

static SkImageFilter* make_mode_blue(SkImageFilter* input = nullptr) {
    SkAutoTUnref<SkColorFilter> filter(cf_make_colorize(SK_ColorBLUE));
    return SkColorFilterImageFilter::Create(filter, input);
}

static void drawClippedRect(SkCanvas* canvas,
                            const SkRect& r,
                            const SkPaint& paint,
                            float outset = 0.0f) {
        canvas->save();
        SkRect clip(r);
        clip.outset(outset, outset);
        canvas->clipRect(clip);
        canvas->drawRect(r, paint);
        canvas->restore();
}

DEF_SIMPLE_GM(colorfilterimagefilter, canvas, 400, 100){
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
        {
            SkAutoTUnref<SkImageFilter> blue(make_mode_blue());
            paint.setImageFilter(blue.get());
            drawClippedRect(canvas, r, paint, 5);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
}

DEF_SIMPLE_GM(colorfilterimagefilter_layer, canvas, 32, 32) {
    SkAutoCanvasRestore autoCanvasRestore(canvas, false);
    SkColorMatrix cm;
    cm.setSaturation(0.0f);
    SkAutoTUnref<SkColorFilter> cf(SkColorMatrixFilter::Create(cm));
    SkAutoTUnref<SkImageFilter> imf(SkColorFilterImageFilter::Create(cf));
    SkPaint p;
    p.setImageFilter(imf);
    canvas->saveLayer(NULL, &p);
    canvas->clear(SK_ColorRED);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> class SkTRefArray : public SkTDArray<T> {
public:
    ~SkTRefArray() { this->unrefAll(); }
};

DEF_SIMPLE_GM(colorfiltershader, canvas, 800, 800) {
    SkTRefArray<SkColorFilter*> filters;
    sk_gm_get_colorfilters(&filters);

    SkTRefArray<SkShader*> shaders;
    sk_gm_get_shaders(&shaders);

    SkPaint paint;
    SkRect r = SkRect::MakeWH(120, 120);

    canvas->translate(20, 20);
    for (int y = 0; y < shaders.count(); ++y) {
        SkShader* shader = shaders[y];
        
        canvas->save();
        for (int x = -1; x < filters.count(); ++x) {
            SkColorFilter* filter = x >= 0 ? filters[x] : nullptr;

            paint.setShader(shader->newWithColorFilter(filter))->unref();
            canvas->drawRect(r, paint);
            canvas->translate(150, 0);
        }
        canvas->restore();
        canvas->translate(0, 150);
    }
}
