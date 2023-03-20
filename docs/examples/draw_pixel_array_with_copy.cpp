// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(draw_pixel_array_with_copy, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    constexpr int width = 4;
    constexpr int height = 4;
    const uint32_t pixels[height * width] = {
        0xFF000000, 0xFF005500, 0xFF00AA00, 0xFF00FF00,
        0xFF000055, 0xFF005555, 0xFF00AA55, 0xFF00FF55,
        0xFF0000AA, 0xFF0055AA, 0xFF00AAAA, 0xFF00FFAA,
        0xFF0000FF, 0xFF0055FF, 0xFF00AAFF, 0xFF00FFFF,
    };
    SkPixmap pixmap(SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType),
                    pixels, sizeof(uint32_t) * width);
    sk_sp<SkImage> img = SkImage::MakeRasterCopy(pixmap);

    canvas->scale(16,16);
    canvas->drawImage(img, 6, 6);
}
}  // END FIDDLE
