// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_skia, 256, 256, false, 0) {
// https://fiddle.skia.org/c/@skpaint_skia

void draw(SkCanvas* canvas) {
    SkPaint paint1, paint2, paint3;

    paint1.setAntiAlias(true);
    paint1.setColor(SkColorSetRGB(255, 0, 0));
    paint1.setStyle(SkPaint::kFill_Style);

    paint2.setAntiAlias(true);
    paint2.setColor(SkColorSetRGB(0, 136, 0));
    paint2.setStyle(SkPaint::kStroke_Style);
    paint2.setStrokeWidth(SkIntToScalar(3));

    paint3.setAntiAlias(true);
    paint3.setColor(SkColorSetRGB(136, 136, 136));

    sk_sp<SkTextBlob> blob1 =
            SkTextBlob::MakeFromString("Skia!", SkFont(nullptr, 64.0f, 1.0f, 0.0f));
    sk_sp<SkTextBlob> blob2 =
            SkTextBlob::MakeFromString("Skia!", SkFont(nullptr, 64.0f, 1.5f, 0.0f));

    canvas->clear(SK_ColorWHITE);
    canvas->drawTextBlob(blob1.get(), 20.0f, 64.0f, paint1);
    canvas->drawTextBlob(blob1.get(), 20.0f, 144.0f, paint2);
    canvas->drawTextBlob(blob2.get(), 20.0f, 224.0f, paint3);
}
}  // END FIDDLE
