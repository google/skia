// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=9b3eb5aaa0dfea9feee54e7650fa5446
REG_FIDDLE(ColorTypeIsAlwaysOpaque, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    const char* colors[] = { "Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                             "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                             "RGBA_F16" };
    SkPaint paint;
    SkFont font(fontMgr->matchFamilyStyle("monospace", SkFontStyle()), 10);
    int y = 15;
    canvas->drawString("    colorType  bytes", 10, y, font, paint);
    for (SkColorType colorType : {
    kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
    kARGB_4444_SkColorType, kRGBA_8888_SkColorType, kRGB_888x_SkColorType,
    kBGRA_8888_SkColorType, kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
    kGray_8_SkColorType, kRGBA_F16_SkColorType
                                 } ) {
        bool result = SkColorTypeIsAlwaysOpaque(colorType);
        SkString string;
        string.printf("%13s %6s", colors[(int) colorType], result ? "true" : "false");
        canvas->drawString(string, 10, y += 14, font, paint);
    }
}
}  // END FIDDLE
