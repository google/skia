// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9adda80b2dd1b08ec5ccf66da7c8bd91
REG_FIDDLE(Pixmap_addr8, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    uint8_t storage[w * h];
    SkPixmap pixmap(SkImageInfo::Make(w, h, kGray_8_SkColorType, kPremul_SkAlphaType),
                    storage, w * sizeof(storage[0]));
    SkDebugf("pixmap.addr8() %c= storage\n",
              pixmap.addr8()  == storage ? '=' : '!');
}
}  // END FIDDLE
