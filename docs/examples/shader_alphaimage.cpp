// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(shader_alphaimage, 256, 256, false, 0) {
sk_sp<SkImage> alpha_image() {
    auto s = SkSurface::MakeRaster(SkImageInfo::MakeA8(128, 128));
    s->getCanvas()->clear(SkColorSetARGB(0xF0, 0x00, 0x00, 0x00));
    return s->makeImageSnapshot();
}
sk_sp<SkShader> linear_gradient() {
    SkPoint gpts[2] = {{0, 0}, {256, 256}};
    SkColor gc[6] = {SK_ColorCYAN, SK_ColorBLUE,   SK_ColorMAGENTA,
                     SK_ColorRED,  SK_ColorYELLOW, SK_ColorGREEN};
    return SkGradientShader::MakeLinear(gpts, gc, nullptr, 6, SkTileMode::kClamp);
}

void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorLTGRAY);
    SkPaint p;
    p.setShader(linear_gradient());
    auto i = alpha_image();
    canvas->drawImage(i.get(), 128, 0, SkSamplingOptions(), &p);
    canvas->drawRect({0, 128, 128, 256}, p);
}
}  // END FIDDLE
