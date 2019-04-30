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
#include "include/core/SkScalar.h"
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
    auto image = SkImage::MakeFromBitmap(make_alpha_image(96, 96));
    SkPaint paint;

    paint.setColorFilter(make_color_filter());
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 10.0f));
    canvas->drawImage(image.get(), 16, 16, &paint);

    paint.setColorFilter(nullptr);
    paint.setShader(SkShaders::Color(SK_ColorCYAN));
    canvas->drawImage(image.get(), 144, 16, &paint);

    paint.setColorFilter(make_color_filter());
    canvas->drawImage(image.get(), 16, 144, &paint);

    paint.setMaskFilter(nullptr);
    canvas->drawImage(image.get(), 144, 144, &paint);
}
