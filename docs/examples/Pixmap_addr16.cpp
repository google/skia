// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=9b16012d265c954c6de13f3fc960da52
REG_FIDDLE(Pixmap_addr16, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    uint16_t storage[w * h];
    SkPixmap pixmap(SkImageInfo::Make(w, h, kARGB_4444_SkColorType, kPremul_SkAlphaType),
                    storage, w * sizeof(storage[0]));
    SkDebugf("pixmap.addr16() %c= storage\n",
              pixmap.addr16()  == storage ? '=' : '!');
}
}  // END FIDDLE
