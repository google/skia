// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=03a9e08082a23a98de17c3e24871d61a
REG_FIDDLE(Bitmap_016, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    for (int rowBytes : { 4, 5, 6, 7, 8} ) {
        bitmap.setInfo(SkImageInfo::MakeN32(1, 1, kPremul_SkAlphaType), rowBytes);
        SkDebugf("rowBytes: %d rowBytesAsPixels: %d\n", rowBytes, bitmap.rowBytesAsPixels());
    }
}
}  // END FIDDLE
