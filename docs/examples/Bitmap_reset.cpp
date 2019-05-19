// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=52ccaeda67924373c5b55a2b89fe314d
REG_FIDDLE(Bitmap_reset, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
    bitmap.allocPixels();
    SkDebugf("width:%d height:%d isNull:%s\n", bitmap.width(), bitmap.height(),
             bitmap.isNull() ? "true" : "false");
    bitmap.reset();
    SkDebugf("width:%d height:%d isNull:%s\n", bitmap.width(), bitmap.height(),
             bitmap.isNull() ? "true" : "false");
}
}  // END FIDDLE
