// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_SRGB(50_percent_srgb, 256, 256, false, 0, 0, false) {
static sk_sp<SkShader> make() {
    auto surf = SkSurface::MakeRasterN32Premul(2, 2);
    surf->getCanvas()->drawColor(SK_ColorWHITE);
    surf->getCanvas()->drawRect({0, 0, 1, 1}, SkPaint());
    surf->getCanvas()->drawRect({1, 1, 2, 2}, SkPaint());
    return surf->makeImageSnapshot()->makeShader(SkTileMode::kRepeat,
                                                 SkTileMode::kRepeat,
                                                 SkSamplingOptions(SkFilterMode::kLinear));
}

void draw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorWHITE);

    const SkRect r = { 0, 0, 100, 100 };
    SkPaint p;
    p.setShader(make());
    // this is a dither
    canvas->drawRect({0, 0, 50, 50}, p);

    canvas->scale(0.5, 0.5);
    canvas->translate(100, 0);
    canvas->drawRect(r, p);
    p.setShader(nullptr);

    p.setColor(0xFF808080);
    canvas->translate(100, 0);
    canvas->drawRect(r, p);

    p.setColor(0x80000000);
    canvas->translate(100, 0);
    canvas->drawRect(r, p);
}
}  // END FIDDLE
