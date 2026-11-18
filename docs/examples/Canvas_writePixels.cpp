// Copyright 2019 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_writePixels, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo imageInfo = SkImageInfo::MakeN32(256, 1, kPremul_SkAlphaType);
    for (int y = 0; y < 256; ++y) {
        std::array<uint32_t, 256> pixels;
        for (int x = 0; x < 256; ++x) {
            pixels[x] = SkColorSetARGB(x, (x + y) % 256, x, (x - y) & 0xFF);
        }
        canvas->writePixels(imageInfo, pixels.data(), pixels.size() * sizeof(pixels[0]), 0, y);
    }
}
}  // END FIDDLE
