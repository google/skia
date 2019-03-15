// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6231bb212d0c231b5bc44eac626fbcb5
REG_FIDDLE(Pixmap_016, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    for (int rowBytes : { 4, 5, 6, 7, 8} ) {
        SkPixmap pixmap(SkImageInfo::MakeN32(1, 1, kPremul_SkAlphaType), nullptr, rowBytes);
        SkDebugf("rowBytes: %d rowBytesAsPixels: %d\n", rowBytes, pixmap.rowBytesAsPixels());
    }
}
}  // END FIDDLE
