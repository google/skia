// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(image_to_bitmap_does_not_preserve_genid, 128, 128, true, 5) {
// image_to_bitmap_does_not_preserve_genid
void draw(SkCanvas* canvas) {
    SkBitmap bm1, bm2;
    image->asLegacyBitmap(&bm1, SkImage::kRO_LegacyBitmapMode);
    image->asLegacyBitmap(&bm2, SkImage::kRO_LegacyBitmapMode);
    SkDebugf("%u\n%u\n", bm1.getGenerationID(), bm2.getGenerationID());
}
}  // END FIDDLE
