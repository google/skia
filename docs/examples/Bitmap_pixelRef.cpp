// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=5db2d30870a7cc45f28e22578d1880c3
REG_FIDDLE(Bitmap_pixelRef, 256, 256, true, 3) {
void draw(SkCanvas* canvas) {
    SkBitmap subset;
    source.extractSubset(&subset, SkIRect::MakeXYWH(32, 64, 128, 256));
    SkDebugf("src ref %c= sub ref\n", source.pixelRef() == subset.pixelRef() ? '=' : '!');
    SkDebugf("src pixels %c= sub pixels\n", source.getPixels() == subset.getPixels() ? '=' : '!');
    SkDebugf("src addr %c= sub addr\n", source.getAddr(32, 64) == subset.getAddr(0, 0) ? '=' : '!');
}
}  // END FIDDLE
