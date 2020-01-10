// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(alpha_bitmap_color_filter_mask_filter, 256, 256, false, 0) {
static SkBitmap make_alpha_image(int w, int h) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::MakeA8(w, h));
    bm.eraseARGB(10, 0, 0, 0);
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = y; x < bm.width(); ++x) {
            *bm.getAddr8(x, y) = 0xFF;
        }
    }
    bm.setImmutable();
    return bm;
}

static sk_sp<SkColorFilter> make_color_filter() {
    SkScalar colorMatrix[20] = {
        1, 0, 0,   0,   0,
        0, 1, 0,   0,   0,
        0, 0, 0.5, 0.5, 0,
        0, 0, 0.5, 0.5, 0}; // mix G and A.
    return SkColorFilters::Matrix(colorMatrix);
}

void draw(SkCanvas* canvas) {
    auto image = SkImage::MakeFromBitmap(make_alpha_image(96, 96));
    SkPaint paint;

    paint.setColorFilter(make_color_filter());
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 10.0f, false));
    canvas->drawImage(image.get(), 16, 16, &paint);

    paint.setColorFilter(nullptr);
    paint.setShader(SkShaders::Color(SK_ColorCYAN));
    canvas->drawImage(image.get(), 144, 16, &paint);

    paint.setColorFilter(make_color_filter());
    canvas->drawImage(image.get(), 16, 144, &paint);

    paint.setMaskFilter(nullptr);
    canvas->drawImage(image.get(), 144, 144, &paint);
}
}  // END FIDDLE
