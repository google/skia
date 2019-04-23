// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=070b1a60232be499eb10c6ea62371804
REG_FIDDLE(Bitmap_012, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* alphas[] = {"Unknown", "Opaque", "Premul", "Unpremul"};
    SkPixmap pixmap(SkImageInfo::MakeA8(16, 32), nullptr, 64);
    SkDebugf("alpha type: k" "%s" "_SkAlphaType\n", alphas[pixmap.alphaType()]);
}
}  // END FIDDLE
