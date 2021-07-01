// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_SRGB(gpu4444diff, 260, 260, false, 3, 0, false) {
void draw(SkCanvas* canvas) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(2, 2, kARGB_4444_SkColorType, kPremul_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorGREEN);
    canvas->drawImage(bitmap.asImage(), 0, 0);
    offscreen.clear(SK_ColorGRAY);
    canvas->drawImage(bitmap.asImage(), 2, 2);
    auto pack4444 = [](unsigned a, unsigned r, unsigned g, unsigned b) -> uint16_t {
        return (a << 0) | (b << 4) | (g << 8) | (r << 12);
    };
    uint16_t red4444[] = { pack4444(0xF, 0xF, 0x0, 0x0), pack4444(0xF, 0xb, 0x0, 0x0),
                           pack4444(0xF, 0x9, 0x0, 0x0), pack4444(0xF, 0x5, 0x0, 0x0) };
    uint16_t blue4444[] = { pack4444(0xF, 0x0, 0x0, 0x0F), pack4444(0xF, 0x0, 0x0, 0x0b),
                            pack4444(0xF, 0x0, 0x0, 0x09), pack4444(0xF, 0x0, 0x0, 0x05) };
    SkImageInfo pixInfo = SkImageInfo::Make(2, 2, kARGB_4444_SkColorType, kPremul_SkAlphaType);
    SkPixmap redPixmap(imageInfo, &red4444, 4);
    if (bitmap.writePixels(redPixmap, 0, 0)) {
        canvas->drawImage(bitmap.asImage(), 4, 4);
    }
    SkPixmap bluePixmap(imageInfo, &blue4444, 4);
    if (bitmap.writePixels(bluePixmap, 0, 0)) {
        canvas->drawImage(bitmap.asImage(), 6, 6);
    }
}
}  // END FIDDLE
