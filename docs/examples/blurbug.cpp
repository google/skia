// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(blurbug, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    uint8_t pixels[][8] = {{0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00},
                           {0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00},
                           {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00},
                           {0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF},
                           {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                           {0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00},
                           {0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00},
                           {0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00}};
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeA8(8, 8), (void*)pixels, sizeof(pixels[0]));
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kSolid_SkBlurStyle, 3));
    for (auto color : {SK_ColorRED, SK_ColorBLUE, 0xFF007F00}) {
        paint.setColor(color);
        canvas->drawImageRect(bitmap.asImage(), SkRect::MakeWH(8, 8), SkRect::MakeWH(32, 32),
                              SkSamplingOptions(), &paint,
                              SkCanvas::kStrict_SrcRectConstraint);
        canvas->translate(48, 0);
    }
}
}  // END FIDDLE
