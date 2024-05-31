// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Pixmap_const_SkImageInfo_const_star, 256, 256, true, 3) {
void draw(SkCanvas* canvas) {
    SkDebugf("image alpha only = %s\n", image->isAlphaOnly() ? "true" : "false");
    SkPMColor pmColors = 0;
    sk_sp<SkImage> copy =
            SkImages::RasterFromPixmapCopy({SkImageInfo::MakeA8(1, 1), (uint8_t*)&pmColors, 1});
    SkDebugf("copy alpha only = %s\n", copy->isAlphaOnly() ? "true" : "false");
}
}  // END FIDDLE
