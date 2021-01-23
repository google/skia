/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"

static SkBitmap make_alpha_image(int w, int h) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::MakeA8(w, h));
    bm.eraseARGB(10, 0, 0 , 0);
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = y; x < bm.width(); ++x) {
            *bm.getAddr8(x, y) = 0xFF;
        }
    }
    bm.setImmutable();
    return bm;
}

static sk_sp<SkColorFilter> make_color_filter() {
    float colorMatrix[20] = {
        1, 0, 0,   0,   0,
        0, 1, 0,   0,   0,
        0, 0, 0.5, 0.5, 0,
        0, 0, 0.5, 0.5, 0}; // mix G and A.
    return SkColorFilters::Matrix(colorMatrix);
}

DEF_SIMPLE_GM(alpha_image, canvas, 256, 256) {
    auto image = make_alpha_image(96, 96).asImage();
    SkPaint paint;

    paint.setColorFilter(make_color_filter());
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 10.0f));
    canvas->drawImage(image.get(), 16, 16, SkSamplingOptions(), &paint);

    paint.setColorFilter(nullptr);
    paint.setShader(SkShaders::Color(SK_ColorCYAN));
    canvas->drawImage(image.get(), 144, 16, SkSamplingOptions(), &paint);

    paint.setColorFilter(make_color_filter());
    canvas->drawImage(image.get(), 16, 144, SkSamplingOptions(), &paint);

    paint.setMaskFilter(nullptr);
    canvas->drawImage(image.get(), 144, 144, SkSamplingOptions(), &paint);
}

// Created to demonstrate skbug.com/10556 - GPU backend was failing to apply paint alpha to
// alpha-only image shaders. The two boxes should look the same.
DEF_SIMPLE_GM(alpha_image_alpha_tint, canvas, 152, 80) {
    canvas->clear(SK_ColorGRAY);

    SkBitmap bm;
    bm.allocPixels(SkImageInfo::MakeA8(64, 64));
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            *bm.getAddr8(x, y) = y * 4;
        }
    }
    bm.setImmutable();
    auto image = bm.asImage();

    SkPaint paint;
    paint.setColor4f({ 0, 1, 0, 0.5f });

    canvas->translate(8, 8);
    canvas->drawImage(image.get(), 0, 0, SkSamplingOptions(), &paint);

    canvas->translate(72, 0);
    paint.setShader(image->makeShader(SkSamplingOptions()));
    canvas->drawRect({ 0, 0, 64, 64 }, paint);
}
