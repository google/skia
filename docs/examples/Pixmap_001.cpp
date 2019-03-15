// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9a00774be57d7308313b3a9073e6e696
REG_FIDDLE(Pixmap_001, 256, 256, true, 3) {
void draw(SkCanvas* canvas) {
    SkDebugf("image alpha only = %s\n", image->isAlphaOnly() ? "true" : "false");
    SkPMColor pmColors = 0;
    sk_sp<SkImage> copy = SkImage::MakeRasterCopy({SkImageInfo::MakeA8(1, 1),
                                                  (uint8_t*)&pmColors,
                                                  1});
    SkDebugf("copy alpha only = %s\n", copy->isAlphaOnly() ? "true" : "false");
}
}  // END FIDDLE
