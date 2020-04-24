// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ec47c4dc23e2925ad565eaba55a91553
REG_FIDDLE(Bitmap_info, 256, 256, true, 4) {
void draw(SkCanvas* canvas) {
    // SkBitmap source;  // pre-populated with soccer ball by fiddle.skia.org
    const SkImageInfo& info = source.info();
    const char* alphas[] = {"Unknown", "Opaque", "Premul", "Unpremul"};
    const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                            "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                            "RGBA_F16"};
    SkDebugf("width: %d height: %d color: %s alpha: %s\n", info.width(), info.height(),
                colors[info.colorType()], alphas[info.alphaType()]);
}
}  // END FIDDLE
