// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=185746dc0faa6f1df30c4afe098646ff
REG_FIDDLE(Canvas_087, 256, 64, false, 4) {
void draw(SkCanvas* canvas) {
   // sk_sp<SkImage> image;
   SkImage* imagePtr = image.get();
   canvas->drawImage(imagePtr, 0, 0);
   SkPaint paint;
   canvas->drawImage(imagePtr, 80, 0, &paint);
   paint.setAlpha(0x80);
   canvas->drawImage(imagePtr, 160, 0, &paint);
}
}  // END FIDDLE
