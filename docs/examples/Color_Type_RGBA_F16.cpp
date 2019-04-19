// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=dd81527bbdf5eaae7dd21ac04ab84f9e
REG_FIDDLE(Color_Type_RGBA_F16, 256, 96, false, 0) {
union FloatUIntUnion {
    uint32_t fUInt;
    float    fFloat;
};
uint16_t FloatToHalf(float f) {
    static const FloatUIntUnion magic = { 15 << 23 };
    static const uint32_t round_mask = ~0xfffu;
    FloatUIntUnion floatUnion;
    floatUnion.fFloat = f;
    uint32_t sign = floatUnion.fUInt & 0x80000000u;
    floatUnion.fUInt ^= sign;
    floatUnion.fUInt &= round_mask;
    floatUnion.fFloat *= magic.fFloat;
    floatUnion.fUInt -= round_mask;
    return (floatUnion.fUInt >> 13) | (sign >> 16);
}

void draw(SkCanvas* canvas) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(2, 2, kRGBA_F16_SkColorType, kPremul_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorGREEN);
    canvas->drawBitmap(bitmap, 0, 0);
    auto H = [](float c) -> uint16_t {
        return FloatToHalf(c);
    };
                             //     R        G        B        A
    uint16_t red_f16[][4] =  { { H(1.0), H(0.0), H(0.0), H(1.0) },
                               { H(.75), H(0.0), H(0.0), H(1.0) },
                               { H(.50), H(0.0), H(0.0), H(1.0) },
                               { H(.25), H(0.0), H(0.0), H(1.0) } };
    uint16_t blue_f16[][4] = { { H(0.0), H(0.0), H(1.0), H(1.0) },
                               { H(0.0), H(0.0), H(.75), H(1.0) },
                               { H(0.0), H(0.0), H(.50), H(1.0) },
                               { H(0.0), H(0.0), H(.25), H(1.0) } };
    SkPixmap redPixmap(imageInfo, red_f16, imageInfo.minRowBytes());
    if (bitmap.writePixels(redPixmap, 0, 0)) {
        canvas->drawBitmap(bitmap, 2, 2);
    }
    SkPixmap bluePixmap(imageInfo, blue_f16, imageInfo.minRowBytes());
    if (bitmap.writePixels(bluePixmap, 0, 0)) {
        canvas->drawBitmap(bitmap, 4, 4);
    }
}
}  // END FIDDLE
