// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6b90c7ae9f254fe4ea9ef638f893a3e6
REG_FIDDLE(Pixmap_024, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    uint32_t storage[w * h];
    SkPixmap pixmap(SkImageInfo::MakeN32(w, h, kPremul_SkAlphaType),
                    storage, w * sizeof(storage[0]));
    SkDebugf("pixmap.addr32() %c= storage\n",
              pixmap.addr32()  == storage ? '=' : '!');
}
}  // END FIDDLE
