// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=410d14ddc45d272598c5a4e52bb047de
REG_FIDDLE(Pixmap_computeByteSize, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPixmap pixmap;
    for (int width : { 1, 1000, 1000000 } ) {
        for (int height: { 1, 1000, 1000000 } ) {
            SkImageInfo imageInfo = SkImageInfo::MakeN32(width, height, kPremul_SkAlphaType);
            pixmap.reset(imageInfo, nullptr, width * 5);
            SkDebugf("width: %7d height: %7d computeByteSize: %13zu\n", width, height,
                     pixmap.computeByteSize());
        }
    }
}
}  // END FIDDLE
