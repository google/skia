// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=193d1f6d8a43b7a8e9f27ba21de38617
REG_FIDDLE(Bitmap_getColor, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int w = 4;
    const int h = 4;
    SkColor colors[][w] = {
        { 0x00000000, 0x2a0e002a, 0x55380055, 0x7f7f007f },
        { 0x2a000e2a, 0x551c1c55, 0x7f542a7f, 0xaaaa38aa },
        { 0x55003855, 0x7f2a547f, 0xaa7171aa, 0xd4d48dd4 },
        { 0x7f007f7f, 0xaa38aaaa, 0xd48dd4d4, 0xffffffff }
    };
    SkDebugf("Premultiplied:\n");
    for (int y = 0; y < h; ++y) {
        SkDebugf("(0, %d) ", y);
        for (int x = 0; x < w; ++x) {
            SkDebugf("0x%08x%c", colors[y][x], x == w - 1 ? '\n' : ' ');
        }
    }
    SkPixmap pixmap(SkImageInfo::MakeN32(w, h, kPremul_SkAlphaType), colors, w * 4);
    SkBitmap bitmap;
    bitmap.installPixels(pixmap);
    SkDebugf("Unpremultiplied:\n");
    for (int y = 0; y < h; ++y) {
        SkDebugf("(0, %d) ", y);
        for (int x = 0; x < w; ++x) {
            SkDebugf("0x%08x%c", bitmap.getColor(x, y), x == w - 1 ? '\n' : ' ');
        }
    }
}
}  // END FIDDLE
