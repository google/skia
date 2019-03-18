// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e89c78ca992e2e789ed50944fe68f920
REG_FIDDLE(Bitmap_readyToDraw, 256, 160, false, 5) {
void draw(SkCanvas* canvas) {
    if (source.readyToDraw()) {
        canvas->drawBitmap(source, 10, 10);
    }
}
}  // END FIDDLE
