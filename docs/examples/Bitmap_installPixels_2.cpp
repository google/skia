// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a7e04447b2081010c50d7920e80a6bb2
REG_FIDDLE(Bitmap_installPixels_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
   SkRandom random;
   SkBitmap bitmap;
   const int width = 8;
   const int height = 8;
   uint32_t pixels[width * height];
   for (unsigned x = 0; x < width * height; ++x) {
       pixels[x] = random.nextU();
   }
   SkImageInfo info = SkImageInfo::MakeN32(width, height, kUnpremul_SkAlphaType);
   if (bitmap.installPixels(info, pixels, info.minRowBytes())) {
       canvas->scale(32, 32);
       canvas->drawBitmap(bitmap, 0, 0);
   }
}
}  // END FIDDLE
