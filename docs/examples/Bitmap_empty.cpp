// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a3762c2722b56ba55e42689c527f146c
REG_FIDDLE(Bitmap_empty, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    for (int width : { 0, 2 } ) {
        for (int height : { 0, 2 } ) {
             bitmap.setInfo(SkImageInfo::MakeA8(width, height));
             SkDebugf("width: %d height: %d empty: %s\n", width, height,
                      bitmap.empty() ? "true" : "false");
        }
    }
}
}  // END FIDDLE
