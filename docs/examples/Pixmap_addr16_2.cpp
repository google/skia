// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2c0c88a546d4ef093ab63ff72dac00b9
REG_FIDDLE(Pixmap_addr16_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    uint16_t storage[w * h];
    SkPixmap pixmap(SkImageInfo::Make(w, h, kARGB_4444_SkColorType, kPremul_SkAlphaType),
                    storage, w * sizeof(storage[0]));
    SkDebugf("pixmap.addr16(1, 2) %c= &storage[1 + 2 * w]\n",
              pixmap.addr16(1, 2)  == &storage[1 + 2 * w] ? '=' : '!');
}
}  // END FIDDLE
