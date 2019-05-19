// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=0ab5c7af272685f2ce177cc79e6b9457
REG_FIDDLE(Pixmap_colorType, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                            "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                            "RGBA_F16"};
    SkPixmap pixmap(SkImageInfo::MakeA8(16, 32), nullptr, 64);
    SkDebugf("color type: k" "%s" "_SkColorType\n", colors[pixmap.colorType()]);
}
}  // END FIDDLE
