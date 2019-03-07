// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=69369cff2f5b145a6f616092513266a0
REG_FIDDLE(Paint_034, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    canvas->scale(.2f, .2f);
    for (SkFilterQuality q : { kNone_SkFilterQuality, kLow_SkFilterQuality,
                               kMedium_SkFilterQuality, kHigh_SkFilterQuality } ) {
        paint.setFilterQuality(q);
        canvas->drawImage(image.get(), 0, 0, &paint);
        canvas->translate(550, 0);
        if (kLow_SkFilterQuality == q) canvas->translate(-1100, 550);
    }
}
}  // END FIDDLE
