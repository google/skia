// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(alphaimage_shader_colorfilter, 256, 256, false, 0) {
sk_sp<SkImage> alpha_image() {
    auto s = SkSurface::MakeRaster(SkImageInfo::MakeA8(64, 64));
    s->getCanvas()->clear(SkColorSetARGB(0x80, 0x00, 0x00, 0x00));
    return s->makeImageSnapshot();
}
sk_sp<SkShader> linear_gradient() {
    SkPoint gpts[2] = {{0, 0}, {64, 64}};
    SkColor gc[3] = {SK_ColorRED, SK_ColorGREEN, SK_ColorMAGENTA};
    return SkGradientShader::MakeLinear(gpts, gc, nullptr, 3, (SkTileMode)0);
}
sk_sp<SkColorFilter> color_filter() {
    SkScalar colorMatrix[20] = {
        1, 0, 0, 0, 0,
        0, 0, 1, 0, 0,
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0};
    return SkColorFilters::Matrix(colorMatrix);
}
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setShader(linear_gradient());
    p.setColorFilter(color_filter());
    auto i = alpha_image();
    canvas->scale(2, 2);
    canvas->drawImage(i.get(), 32, 32, SkSamplingOptions(), &p);
}
}  // END FIDDLE
