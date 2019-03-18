// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5449f65fd7673273b0b57807fd3117ff
REG_FIDDLE(Pixmap_addr64_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    uint64_t storage[w * h];
    SkPixmap pixmap(SkImageInfo::Make(w, h, kRGBA_F16_SkColorType, kPremul_SkAlphaType),
                    storage, w * sizeof(storage[0]));
    SkDebugf("pixmap.addr64(1, 2) %c= &storage[1 + 2 * w]\n",
              pixmap.addr64(1, 2)  == &storage[1 + 2 * w] ? '=' : '!');
}
}  // END FIDDLE
