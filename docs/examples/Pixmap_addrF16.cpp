// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=54e8525a592f05623c33b375aebc90c1
REG_FIDDLE(Pixmap_addrF16, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    uint16_t storage[w * h * 4];
    SkPixmap pixmap(SkImageInfo::Make(w, h, kRGBA_F16_SkColorType, kPremul_SkAlphaType),
                    storage, w * 4 * sizeof(storage[0]));
    SkDebugf("pixmap.addrF16() %c= storage\n",
              pixmap.addrF16()  == storage ? '=' : '!');
}
}  // END FIDDLE
