// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_SRGB(50_percent_gray, 530, 150, false, 0, 0, false) {
static sk_sp<SkShader> make_bw_dither() {
    auto surf = SkSurface::MakeRasterN32Premul(2, 2);
    surf->getCanvas()->drawColor(SK_ColorWHITE);
    surf->getCanvas()->drawRect({0, 0, 1, 1}, SkPaint());
    surf->getCanvas()->drawRect({1, 1, 2, 2}, SkPaint());
    return surf->makeImageSnapshot()->makeShader(SkTileMode::kRepeat,
                                                 SkTileMode::kRepeat
                                                 SkSamplingOptions(SkFilterMode::kLinear));
}

void draw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorWHITE);
    SkFont font(nullptr, 12);

    // BW Dither
    canvas->translate(5, 5);
    SkPaint p;
    p.setShader(make_bw_dither());
    canvas->drawRect({0, 0, 100, 100}, p);
    SkPaint black;
    canvas->drawString("BW Dither", 0, 125, font, black);

    // Scaled BW Dither
    canvas->translate(105, 0);
    canvas->save();
    canvas->scale(0.5, 0.5);
    canvas->drawRect({0, 0, 200, 200}, p);
    canvas->restore();
    canvas->drawString("Scaled Dither", 0, 125, font, black);

    // Blend black on to white
    canvas->translate(105, 0);
    p.setColor(0x80000000);
    p.setShader(nullptr);
    canvas->drawRect({0, 0, 100, 100}, p);
    p.setShader(nullptr);
    drawString(canvas,"Blend", 0, 125, font, black);

    // Opaque color (0xFFBCBCBC)
    canvas->translate(105, 0);
    p.setColor(0xFFBCBCBC);
    canvas->drawRect({0, 0, 100, 100}, p);
    canvas->drawString("0xFFBCBCBC", 0, 125, font, black);

    // Opaque color (0xFF808080)
    canvas->translate(105, 0);
    p.setColor(0xFF808080);
    canvas->drawRect({0, 0, 100, 100}, p);
    canvas->drawString("0xFF808080", 0, 125, font, black);
}
}  // END FIDDLE
