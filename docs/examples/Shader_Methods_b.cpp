// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=fe80fd80b98a20823db7fb9a077243c7
REG_FIDDLE(Shader_Methods_b, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkBitmap bitmap;
   bitmap.setInfo(SkImageInfo::MakeA8(5, 1), 5);  // bitmap only contains alpha
   uint8_t pixels[5] = { 0x22, 0x55, 0x88, 0xBB, 0xFF };
   bitmap.setPixels(pixels);
   paint.setShader(bitmap.makeShader(SkTileMode::kMirror, SkTileMode::kMirror,
                                     SkSamplingOptions()));
   for (SkColor c : { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN } ) {
       paint.setColor(c);  // all components in color affect shader
       canvas->drawCircle(50, 50, 50, paint);
       canvas->translate(70, 70);
   }
}
}  // END FIDDLE
