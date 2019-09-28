// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=94ad244056dc80ecd87daae004266334
REG_FIDDLE(Pixmap_getColor, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    std::vector<SkPMColor> storage;
    storage.resize(w * h);
    SkDebugf("Premultiplied:\n");
    for (int y = 0; y < h; ++y) {
        SkDebugf("(0, %d) ", y);
        for (int x = 0; x < w; ++x) {
            int a = 0xFF * (x + y) / (w - 1 + h - 1);
            storage[x + y * w] = SkPackARGB32(a, a * x / (w - 1), a * y / (h - 1), a);
            SkDebugf("0x%08x%c", storage[x + y * w], x == w - 1 ? '\n' : ' ');
        }
    }
    SkPixmap pixmap(SkImageInfo::MakeN32(w, h, kPremul_SkAlphaType), &storage.front(), w * 4);
    SkDebugf("Unpremultiplied:\n");
    for (int y = 0; y < h; ++y) {
        SkDebugf("(0, %d) ", y);
        for (int x = 0; x < w; ++x) {
            SkDebugf("0x%08x%c", pixmap.getColor(x, y), x == w - 1 ? '\n' : ' ');
        }
    }
}
}  // END FIDDLE
