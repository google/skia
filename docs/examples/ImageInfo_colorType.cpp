// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=06ecc3ce7f35cc7f930cbc2a662e3105
REG_FIDDLE(ImageInfo_colorType, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                            "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                            "RGBA_F16"};
    SkImageInfo info = SkImageInfo::MakeA8(16, 32);
    SkDebugf("color type: k" "%s" "_SkColorType\n", colors[info.colorType()]);
}
}  // END FIDDLE
