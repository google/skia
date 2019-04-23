// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c204b38b3fc08914b0a634aa4eaec894
REG_FIDDLE(Image_bounds, 256, 128, false, 4) {
void draw(SkCanvas* canvas) {
    SkIRect bounds = image->bounds();
    for (int x : { 0, bounds.width() } ) {
        for (int y : { 0, bounds.height() } ) {
            canvas->drawImage(image, x, y);
        }
    }
}
}  // END FIDDLE
