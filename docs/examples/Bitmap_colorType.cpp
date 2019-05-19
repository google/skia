// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ceb77fab7326b57822a147b04aa0960e
REG_FIDDLE(Bitmap_colorType, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                            "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                            "RGBA_F16"};
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeA8(16, 32));
    SkDebugf("color type: k" "%s" "_SkColorType\n", colors[bitmap.colorType()]);
}
}  // END FIDDLE
