// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=daacf43394ce4045a362a48b5774deed
REG_FIDDLE(Bitmap_020, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    for (int w : { 0, 8 } ) {
        for (bool allocate : { false, true} ) {
            bitmap.setInfo(SkImageInfo::MakeA8(w, 8));
            allocate ? bitmap.allocPixels() : (void) 0 ;
            SkDebugf("empty:%s isNull:%s drawsNothing:%s\n", bitmap.empty() ? "true " : "false",
                     bitmap.isNull() ? "true " : "false", bitmap.drawsNothing() ? "true" : "false");
        }
    }
}
}  // END FIDDLE
