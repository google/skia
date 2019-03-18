// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6e0f558bf7fabc655041116288559134
REG_FIDDLE(Pixmap_info, 256, 256, true, 3) {
void draw(SkCanvas* canvas) {
    std::vector<int32_t> pixels;
    pixels.resize(image->height() * image->width() * 4);
    SkPixmap pixmap(SkImageInfo::Make(image->width(), image->height(), kN32_SkColorType,
            image->alphaType()), (const void*) &pixels.front(), image->width() * 4);
    image->readPixels(pixmap, 0, 0);
    SkPixmap inset;
    if (pixmap.extractSubset(&inset, {128, 128, 512, 512})) {
        const SkImageInfo& info = inset.info();
        const char* alphas[] = {"Unknown", "Opaque", "Premul", "Unpremul"};
        const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888",
                "RGB_888x", "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                            "RGBA_F16"};
        SkDebugf("width: %d height: %d color: %s alpha: %s\n", info.width(), info.height(),
                 colors[info.colorType()], alphas[info.alphaType()]);
    }
}
}  // END FIDDLE
