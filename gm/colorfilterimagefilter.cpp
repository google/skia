/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "tools/Resources.h"

#include <string.h>
#include <utility>

#define FILTER_WIDTH    SkIntToScalar(30)
#define FILTER_HEIGHT   SkIntToScalar(30)
#define MARGIN          SkIntToScalar(10)

static sk_sp<SkColorFilter> cf_make_brightness(float brightness) {
    float matrix[20] = {
        1, 0, 0, 0, brightness,
        0, 1, 0, 0, brightness,
        0, 0, 1, 0, brightness,
        0, 0, 0, 1, 0 };
    return SkColorFilters::Matrix(matrix);
}

static sk_sp<SkColorFilter> cf_make_grayscale() {
    float matrix[20];
    memset(matrix, 0, 20 * sizeof(float));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    return SkColorFilters::Matrix(matrix);
}

static sk_sp<SkColorFilter> cf_make_colorize(SkColor color) {
    return SkColorFilters::Blend(color, SkBlendMode::kSrc);
}

static void sk_gm_get_colorfilters(SkTArray<sk_sp<SkColorFilter>>* array) {
    array->push_back(cf_make_brightness(0.5f));
    array->push_back(cf_make_grayscale());
    array->push_back(cf_make_colorize(SK_ColorBLUE));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkShader> sh_make_lineargradient0() {
    const SkPoint pts[] = { { 0, 0 }, { 100, 100 } };
    const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 3, SkTileMode::kRepeat);
}

static sk_sp<SkShader> sh_make_lineargradient1() {
    const SkPoint pts[] = { { 0, 0 }, { 100, 100 } };
    const SkColor colors[] = { SK_ColorRED, 0x0000FF00, SK_ColorBLUE };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 3, SkTileMode::kRepeat);
}

static sk_sp<SkShader> sh_make_image() {
    sk_sp<SkImage> image(GetResourceAsImage("images/mandrill_128.png"));
    if (!image) {
        return nullptr;
    }
    return image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions());
}

static void sk_gm_get_shaders(SkTDArray<SkShader*>* array) {
    if (auto shader = sh_make_lineargradient0()) {
        *array->append() = shader.release();
    }
    if (auto shader = sh_make_lineargradient1()) {
        *array->append() = shader.release();
    }
    if (auto shader = sh_make_image()) {
        *array->append() = shader.release();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImageFilter> make_blur(float amount, sk_sp<SkImageFilter> input) {
    return SkImageFilters::Blur(amount, amount, std::move(input));
}

static sk_sp<SkImageFilter> make_brightness(float amount, sk_sp<SkImageFilter> input) {
    return SkImageFilters::ColorFilter(cf_make_brightness(amount), std::move(input));
}

static sk_sp<SkImageFilter> make_grayscale(sk_sp<SkImageFilter> input) {
    return SkImageFilters::ColorFilter(cf_make_grayscale(), std::move(input));
}

static sk_sp<SkImageFilter> make_mode_blue(sk_sp<SkImageFilter> input) {
    return SkImageFilters::ColorFilter(cf_make_colorize(SK_ColorBLUE), std::move(input));
}

static void draw_clipped_rect(SkCanvas* canvas,
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
            sk_sp<SkImageFilter> dim(make_brightness(-brightness, nullptr));
            sk_sp<SkImageFilter> bright(make_brightness(brightness, std::move(dim)));
            paint.setImageFilter(std::move(bright));
            draw_clipped_rect(canvas, r, paint);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
        canvas->restore();
        canvas->translate(0, FILTER_HEIGHT + MARGIN);
        {
            sk_sp<SkImageFilter> brightness(make_brightness(0.9f, nullptr));
            sk_sp<SkImageFilter> grayscale(make_grayscale(std::move(brightness)));
            paint.setImageFilter(std::move(grayscale));
            draw_clipped_rect(canvas, r, paint);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
        {
            sk_sp<SkImageFilter> grayscale(make_grayscale(nullptr));
            sk_sp<SkImageFilter> brightness(make_brightness(0.9f, std::move(grayscale)));
            paint.setImageFilter(std::move(brightness));
            draw_clipped_rect(canvas, r, paint);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
        {
            sk_sp<SkImageFilter> blue(make_mode_blue(nullptr));
            sk_sp<SkImageFilter> brightness(make_brightness(1.0f, std::move(blue)));
            paint.setImageFilter(std::move(brightness));
            draw_clipped_rect(canvas, r, paint);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
        {
            sk_sp<SkImageFilter> brightness(make_brightness(1.0f, nullptr));
            sk_sp<SkImageFilter> blue(make_mode_blue(std::move(brightness)));
            paint.setImageFilter(std::move(blue));
            draw_clipped_rect(canvas, r, paint);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
        {
            sk_sp<SkImageFilter> blur(make_blur(3.0f, nullptr));
            sk_sp<SkImageFilter> brightness(make_brightness(0.5f, std::move(blur)));
            paint.setImageFilter(std::move(brightness));
            draw_clipped_rect(canvas, r, paint, 3);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
        {
            sk_sp<SkImageFilter> blue(make_mode_blue(nullptr));
            paint.setImageFilter(std::move(blue));
            draw_clipped_rect(canvas, r, paint, 5);
            canvas->translate(FILTER_WIDTH + MARGIN, 0);
        }
}

DEF_SIMPLE_GM(colorfilterimagefilter_layer, canvas, 32, 32) {
    SkAutoCanvasRestore autoCanvasRestore(canvas, false);
    SkColorMatrix cm;
    cm.setSaturation(0.0f);
    sk_sp<SkColorFilter> cf(SkColorFilters::Matrix(cm));
    SkPaint p;
    p.setImageFilter(SkImageFilters::ColorFilter(std::move(cf), nullptr));
    canvas->saveLayer(nullptr, &p);
    canvas->clear(SK_ColorRED);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> class SkTRefArray : public SkTDArray<T> {
public:
    ~SkTRefArray() { this->unrefAll(); }
};

DEF_SIMPLE_GM(colorfiltershader, canvas, 610, 610) {
    SkTArray<sk_sp<SkColorFilter>> filters;
    sk_gm_get_colorfilters(&filters);

    SkTRefArray<SkShader*> shaders;
    sk_gm_get_shaders(&shaders);

    const SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
    *shaders.append() = SkGradientShader::MakeTwoPointConical({0, 0}, 50, {0, 0}, 150,
                                                              colors, nullptr, 2,
                                                              SkTileMode::kClamp).release();

    SkPaint paint;
    SkRect r = SkRect::MakeWH(120, 120);

    canvas->translate(20, 20);
    for (int y = 0; y < shaders.count(); ++y) {
        SkShader* shader = shaders[y];

        canvas->save();
        for (int x = -1; x < filters.count(); ++x) {
            sk_sp<SkColorFilter> filter = x >= 0 ? filters[x] : nullptr;

            paint.setShader(shader->makeWithColorFilter(filter));
            canvas->drawRect(r, paint);
            canvas->translate(150, 0);
        }
        canvas->restore();
        canvas->translate(0, 150);
    }
}
