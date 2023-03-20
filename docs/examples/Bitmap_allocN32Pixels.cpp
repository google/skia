// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

#include <climits>
#include <random>

REG_FIDDLE(Bitmap_allocN32Pixels, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(64, 64);
    bitmap.eraseColor(SK_ColorTRANSPARENT);
    std::default_random_engine rng;
    const auto randUint = [&rng](uint32_t min, uint32_t max) -> uint32_t {
        return std::uniform_int_distribution<uint32_t>(min, max)(rng);
    };

    for (int y = 0; y < 256; y += 64) {
        for (int x = 0; x < 256; x += 64) {
            SkColor color = randUint(0, UINT_MAX);
            uint32_t w = randUint(4, 32);
            uint32_t cx = randUint(0, 64 - w);
            uint32_t h = randUint(4, 32);
            uint32_t cy = randUint(0, 64 - h);
            bitmap.erase(color, SkIRect::MakeXYWH(cx, cy, w, h));
            canvas->drawImage(bitmap.asImage(), x, y);
        }
    }
}
}  // END FIDDLE
