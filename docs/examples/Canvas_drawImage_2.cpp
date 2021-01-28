// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a4e877e891b1be5faa2b7fd07f673a10
REG_FIDDLE(Canvas_drawImage_2, 256, 64, false, 4) {
void draw(SkCanvas* canvas) {
   // sk_sp<SkImage> image;
   canvas->drawImage(image, 0, 0);
   SkPaint paint;
   canvas->drawImage(image, 80, 0, SkSamplingOptions(), &paint);
   paint.setAlpha(0x80);
   canvas->drawImage(image, 160, 0, SkSamplingOptions(), &paint);
}
}  // END FIDDLE
