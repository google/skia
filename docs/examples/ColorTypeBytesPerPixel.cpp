// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=09ef49d07cb7005ba3e34d5ea53896f5
REG_FIDDLE(ColorTypeBytesPerPixel, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkFont font(SkTypeface::MakeFromName("monospace", SkFontStyle()), 10);
    int y = 15;
    canvas->drawString("    colorType  bytes", 10, y, font, paint);
    for (SkColorType colorType : {
    kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
    kARGB_4444_SkColorType, kRGBA_8888_SkColorType, kRGB_888x_SkColorType,
    kBGRA_8888_SkColorType, kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
    kGray_8_SkColorType, kRGBA_F16_SkColorType
                                 } ) {
        int result = SkColorTypeBytesPerPixel(colorType);
        SkString string;
        string.printf("%13s %4d", SkColorTypeToString(colorType), result);
        canvas->drawString(string, 10, y += 14, font, paint);
    }
}
}  // END FIDDLE
