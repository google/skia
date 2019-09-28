// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3e9126152ff1cc592aef6facbcb5fc96
REG_FIDDLE(Bitmap_bounds, 256, 64, false, 4) {
void draw(SkCanvas* canvas) {
    canvas->scale(.5f, .5f);
    SkIRect bounds = source.bounds();
    for (int x : { 0, bounds.width() } ) {
        for (int y : { 0, bounds.height() } ) {
            canvas->drawBitmap(source, x, y);
        }
    }
}
}  // END FIDDLE
