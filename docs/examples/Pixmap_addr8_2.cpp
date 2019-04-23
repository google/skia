// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=5b986272268ef2c52045c1856f8b6107
REG_FIDDLE(Pixmap_addr8_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    uint8_t storage[w * h];
    SkPixmap pixmap(SkImageInfo::Make(w, h, kGray_8_SkColorType, kPremul_SkAlphaType),
                    storage, w * sizeof(storage[0]));
    SkDebugf("pixmap.addr8(1, 2) %c= &storage[1 + 2 * w]\n",
              pixmap.addr8(1, 2)  == &storage[1 + 2 * w] ? '=' : '!');
}
}  // END FIDDLE
