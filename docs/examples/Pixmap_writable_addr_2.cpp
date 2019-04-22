// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=559eaca89c765bc8466ea1ba3331d4db
REG_FIDDLE(Pixmap_writable_addr_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    SkPMColor storage[w * h * 4];
    SkPixmap pixmap(SkImageInfo::MakeN32(w, h, kPremul_SkAlphaType), storage, w * 4);
    SkDebugf("pixmap.writable_addr() %c= (void *)storage\n",
              pixmap.writable_addr()  == (void *)storage ? '=' : '!');
    pixmap.erase(0x00000000);
    *(SkPMColor*)pixmap.writable_addr(1, 2) = 0xFFFFFFFF;
    SkDebugf("pixmap.getColor(0, 0) %c= 0x00000000\n",
              pixmap.getColor(0, 0)  == 0x00000000 ? '=' : '!');
    SkDebugf("pixmap.getColor(1, 2) %c= 0xFFFFFFFF\n",
              pixmap.getColor(1, 2)  == 0xFFFFFFFF ? '=' : '!');
}
}  // END FIDDLE
