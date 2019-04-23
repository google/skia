// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=211ec89418011aa6e54aa2cc9567e003
REG_FIDDLE(Bitmap_isNull, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    SkDebugf("empty bitmap does %shave pixels\n", bitmap.isNull() ? "not " : "");
    bitmap.setInfo(SkImageInfo::MakeA8(8, 8));
    SkDebugf("bitmap with dimensions does %shave pixels\n", bitmap.isNull() ? "not " : "");
    bitmap.allocPixels();
    SkDebugf("allocated bitmap does %shave pixels\n", bitmap.isNull() ? "not " : "");
}
}  // END FIDDLE
