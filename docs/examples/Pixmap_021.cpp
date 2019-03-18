// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6e6e29e860eafed77308c973400cc84d
REG_FIDDLE(Pixmap_addr_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    std::vector<SkPMColor> storage;
    storage.resize(w * h);
    SkPixmap pixmap(SkImageInfo::MakeN32(w, h, kPremul_SkAlphaType), &storage.front(), w * 4);
    SkDebugf("pixmap.addr(1, 2) %c= &storage[1 + 2 * w]\n",
              pixmap.addr(1, 2)  == &storage[1 + 2 * w] ? '=' : '!');
}
}  // END FIDDLE
