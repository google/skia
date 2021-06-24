// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=17bcabaaee2dbb7beba562e9ca50b55e
REG_FIDDLE(Pixmap_addr, 256, 256, true, 3) {
void draw(SkCanvas* canvas) {
    std::vector<int32_t> pixels;
    pixels.resize(image->height() * image->width() * 4);
    SkPixmap pixmap(SkImageInfo::Make(image->width(), image->height(), kN32_SkColorType,
            image->alphaType()), (const void*) &pixels.front(), image->width() * 4);
    image->readPixels(nullptr, pixmap, 0, 0);
    SkDebugf("pixels address: 0x%p\n", pixmap.addr());
    SkPixmap inset;
    if (pixmap.extractSubset(&inset, {128, 128, 512, 512})) {
        SkDebugf("inset address:  0x%p\n", inset.addr());
    }
}
}  // END FIDDLE
