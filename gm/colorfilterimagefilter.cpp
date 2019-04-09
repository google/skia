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
#include "SkMixer.h"
#include "SkShader.h"

#include "SkBlurImageFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkTDArray.h"

#define FILTER_WIDTH    SkIntToScalar(30)
#define FILTER_HEIGHT   SkIntToScalar(30)
#define MARGIN          SkIntToScalar(10)

static sk_sp<SkColorFilter> cf_make_brightness(float brightness) {
    SkScalar amount255 = brightness * 255;
    SkScalar matrix[20] = {
        1, 0, 0, 0, amount255,
        0, 1, 0, 0, amount255,
        0, 0, 1, 0, amount255,
        0, 0, 0, 1, 0 };
    return SkColorFilters::MatrixRowMajor255(matrix);
}

static sk_sp<SkColorFilter> cf_make_grayscale() {
    SkScalar matrix[20];
    memset(matrix, 0, 20 * sizeof(SkScalar));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    return SkColorFilters::MatrixRowMajor255(matrix);
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

#include "SkGradientShader.h"
#include "SkImage.h"
#include "Resources.h"

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
    return image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat);
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
    return SkBlurImageFilter::Make(amount, amount, std::move(input));
}

static sk_sp<SkImageFilter> make_brightness(float amount, sk_sp<SkImageFilter> input) {
    return SkColorFilterImageFilter::Make(cf_make_brightness(amount), std::move(input));
}

static sk_sp<SkImageFilter> make_grayscale(sk_sp<SkImageFilter> input) {
    return SkColorFilterImageFilter::Make(cf_make_grayscale(), std::move(input));
}

static sk_sp<SkImageFilter> make_mode_blue(sk_sp<SkImageFilter> input) {
    return SkColorFilterImageFilter::Make(cf_make_colorize(SK_ColorBLUE), std::move(input));
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
    sk_sp<SkColorFilter> cf(SkColorFilters::MatrixRowMajor255(cm.fMat));
    SkPaint p;
    p.setImageFilter(SkColorFilterImageFilter::Make(std::move(cf), nullptr));
    canvas->saveLayer(nullptr, &p);
    canvas->clear(SK_ColorRED);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkGradientShader.h"

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

template <typename Maker> void do_mixershader(SkCanvas* canvas, Maker&& maker) {
    auto shaderA = GetResourceAsImage("images/mandrill_128.png")->makeShader(SkTileMode::kClamp,
                                                                             SkTileMode::kClamp);
    const SkColor colors[] = { SK_ColorGREEN, 0 };
    auto shaderB = SkGradientShader::MakeRadial({60, 60}, 55, colors, nullptr, 2,
                                                SkTileMode::kClamp,
                                                SkGradientShader::kInterpolateColorsInPremul_Flag,
                                                nullptr);
    const SkBlendMode modes[] = {
        SkBlendMode::kSrc, SkBlendMode::kModulate, SkBlendMode::kColorBurn, SkBlendMode::kPlus,
        SkBlendMode::kDstATop,
    };
    SkPaint paint;
    SkRect r = SkRect::MakeWH(120, 120);

    canvas->translate(10, 10);
    for (auto mode : modes) {
        canvas->save();
        const int count = 6;
        for (int x = 0; x < count; ++x) {
            const float t = x * 1.0f / (count - 1);
            paint.setShader(maker(shaderA, shaderB, mode, t));
            canvas->drawRect(r, paint);
            canvas->translate(r.width() + 10, 0);
        }
        canvas->restore();
        canvas->translate(0, r.height() + 20);
    }
}

DEF_SIMPLE_GM(mixershader, canvas, 800, 700) {
    do_mixershader(canvas, [](sk_sp<SkShader> a, sk_sp<SkShader> b, SkBlendMode mode, float t) {
        auto sh = SkShaders::Blend(mode, a, b);
        return SkShaders::Lerp(t, a, sh);
    });
}

#include "SkMixerBase.h"

DEF_SIMPLE_GM(mixershader2, canvas, 800, 700) {
    do_mixershader(canvas, [](sk_sp<SkShader> a, sk_sp<SkShader> b, SkBlendMode mode, float t) {
        return SkShaders::Lerp(t, a, SkShaders::Blend(mode, a, b));
    });
}
