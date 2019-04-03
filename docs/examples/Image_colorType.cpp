// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=50396fad4a128f58e400ca00fe09711f
REG_FIDDLE(Image_colorType, 256, 96, false, 4) {
static const char* color_type_to_string(SkColorType v) {
    switch (v) {
        case kUnknown_SkColorType: return "Unknown";
        case kAlpha_8_SkColorType: return "Alpha_8";
        case kRGB_565_SkColorType: return "RGB_565";
        case kARGB_4444_SkColorType: return "ARGB_4444";
        case kRGBA_8888_SkColorType: return "RGBA_8888";
        case kRGB_888x_SkColorType: return "RGB_888x";
        case kBGRA_8888_SkColorType: return "BGRA_8888";
        case kRGBA_1010102_SkColorType: return "RGBA_1010102";
        case kRGB_101010x_SkColorType: return "RGB_101010x";
        case kGray_8_SkColorType: return "Gray_8";
        case kRGBA_F16Norm_SkColorType: return "RGBA_F16Norm";
        case kRGBA_F16_SkColorType: return "RGBA_F16";
        case kRGBA_F32_SkColorType: return "RGBA_F32";
    }
}

void draw(SkCanvas* canvas) {
    SkColorType colorType = image->colorType();
    canvas->drawImage(image, 16, 0);
    canvas->drawString(color_type_to_string(colorType), 20, image->height() + 20, SkFont(), SkPaint());
}
}  // END FIDDLE
