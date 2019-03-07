// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=647056bcc12c27fb4413f212f33a2898
REG_FIDDLE(Bitmap_035, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeN32(33, 55, kOpaque_SkAlphaType));
    SkISize dimensions = bitmap.dimensions();
    SkRect bounds;
    bitmap.getBounds(&bounds);
    SkRect dimensionsAsBounds = SkRect::Make(dimensions);
    SkDebugf("dimensionsAsBounds %c= bounds\n", dimensionsAsBounds == bounds ? '=' : '!');
}
}  // END FIDDLE
