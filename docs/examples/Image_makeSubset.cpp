// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=889e495ce3e3b3bacc96e8230932331c
REG_FIDDLE(Image_makeSubset, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->scale(.5f, .5f);
    const int width = 64;
    const int height = 64;
    for (int y = 0; y < 512; y += height ) {
        for (int x = 0; x < 512; x += width ) {
            sk_sp<SkImage> subset(image->makeSubset({x, y, x + width, y + height}));
            canvas->drawImage(subset, x * 3 / 2, y * 3 / 2);
        }
    }
}
}  // END FIDDLE
