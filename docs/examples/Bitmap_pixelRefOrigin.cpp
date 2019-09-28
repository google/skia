// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6d31686c6c0829c70f284ae716526d6a
REG_FIDDLE(Bitmap_pixelRefOrigin, 256, 256, true, 3) {
void draw(SkCanvas* canvas) {
    SkBitmap subset;
    source.extractSubset(&subset, SkIRect::MakeXYWH(32, 64, 128, 256));
    SkIPoint sourceOrigin = source.pixelRefOrigin();
    SkIPoint subsetOrigin = subset.pixelRefOrigin();
    SkDebugf("source origin: %d, %d\n", sourceOrigin.fX, sourceOrigin.fY);
    SkDebugf("subset origin: %d, %d\n", subsetOrigin.fX, subsetOrigin.fY);
}
}  // END FIDDLE
