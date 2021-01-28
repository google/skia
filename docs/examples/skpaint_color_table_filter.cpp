// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_color_table_filter, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->scale(0.5, 0.5);
    uint8_t ct[256];
    for (int i = 0; i < 256; ++i) {
        int x = (i - 96) * 255 / 64;
        ct[i] = x < 0 ? 0 : x > 255 ? 255 : x;
    }
    SkPaint paint;
    paint.setColorFilter(SkTableColorFilter::MakeARGB(nullptr, ct, ct, ct));
    canvas->drawImage(image, 0, 0, SkSamplingOptions(), &paint);
}
}  // END FIDDLE
