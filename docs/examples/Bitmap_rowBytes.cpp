// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Bitmap_rowBytes, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    for (int rowBytes : { 2, 8 } ) {
        bool result = bitmap.setInfo(SkImageInfo::MakeA8(4, 4), rowBytes);
        SkDebugf("setInfo returned:%s rowBytes:%zu\n",
                 result ? "true " : "false", bitmap.rowBytes());
     }
}
}  // END FIDDLE
