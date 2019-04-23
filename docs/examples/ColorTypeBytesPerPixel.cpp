// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=09ef49d07cb7005ba3e34d5ea53896f5
REG_FIDDLE(ColorTypeBytesPerPixel, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    const char* colors[] = { "Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                             "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                             "RGBA_F16" };
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
        string.printf("%13s %4d", colors[(int) colorType], result);
        canvas->drawString(string, 10, y += 14, font, paint);
    }
}
}  // END FIDDLE
