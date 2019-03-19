// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=837a2bcc9fb9ce617a3420956cefc64a
REG_FIDDLE(Bitmap_getAddr32, 256, 256, true, 3) {
void draw(SkCanvas* canvas) {
    uint32_t* row0 = source.getAddr32(0, 0);
    uint32_t* row1 = source.getAddr32(0, 1);
    size_t interval = (row1 - row0) * source.bytesPerPixel();
    SkDebugf("addr interval %c= rowBytes\n", interval == source.rowBytes() ? '=' : '!');
}
}  // END FIDDLE
