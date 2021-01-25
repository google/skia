// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=67277b0a1003f340473a35982533561c
REG_FIDDLE(Bitmap_eraseARGB, 256, 80, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32(1, 1, kPremul_SkAlphaType));
    bitmap.eraseARGB(0x7f, 0xff, 0x7f, 0x3f);
    canvas->scale(50, 50);
    canvas->drawImage(bitmap.asImage(), 0, 0);
    canvas->drawImage(bitmap.asImage(), .5f, .5f);
}
}  // END FIDDLE
