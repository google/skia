// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Bitmap_computeByteSize, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    for (int width : { 1, 1000, 1000000 } ) {
        for (int height: { 1, 1000, 1000000 } ) {
            SkImageInfo imageInfo = SkImageInfo::MakeN32(width, height, kPremul_SkAlphaType);
            bitmap.setInfo(imageInfo, width * 5);
            SkDebugf("width: %7d height: %7d computeByteSize: %13zu\n", width, height,
                     bitmap.computeByteSize());
        }
    }
}
}  // END FIDDLE
