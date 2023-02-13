// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

#include <climits>
#include <random>

REG_FIDDLE(Bitmap_installPixels_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    std::default_random_engine rng;
    const auto randUint = [&rng](uint32_t min, uint32_t max) -> uint32_t {
        return std::uniform_int_distribution<uint32_t>(min, max)(rng);
    };
    SkBitmap bitmap;
    const int width = 8;
    const int height = 8;
    uint32_t pixels[width * height];
    for (unsigned x = 0; x < width * height; ++x) {
       pixels[x] = randUint(0, UINT_MAX);
    }
    SkImageInfo info = SkImageInfo::MakeN32(width, height, kUnpremul_SkAlphaType);
    if (bitmap.installPixels(info, pixels, info.minRowBytes())) {
       canvas->scale(32, 32);
       canvas->drawImage(bitmap.asImage(), 0, 0);
    }
}
}  // END FIDDLE
