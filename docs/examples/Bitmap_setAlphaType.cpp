// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=af3adcbea7b58bf90298ca5e0ea93030
REG_FIDDLE(Bitmap_setAlphaType, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    SkAlphaType alphaTypes[] = { kUnknown_SkAlphaType, kOpaque_SkAlphaType, kPremul_SkAlphaType,
    kUnpremul_SkAlphaType
                               };
    SkDebugf("%18s%15s%17s%18s%19s\n", "Canonical", "Unknown", "Opaque", "Premul", "Unpremul");
    for (SkColorType colorType : {
    kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
    kARGB_4444_SkColorType, kRGBA_8888_SkColorType, kRGB_888x_SkColorType,
    kBGRA_8888_SkColorType, kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
    kGray_8_SkColorType, kRGBA_F16_SkColorType
                                 } ) {
        for (SkAlphaType canonicalAlphaType : alphaTypes) {
            SkColorTypeValidateAlphaType(colorType, kUnknown_SkAlphaType, &canonicalAlphaType );
            SkDebugf("%12s %9s  ", SkColorTypeToString(colorType),
                    SkAlphaTypeToString(canonicalAlphaType));
            for (SkAlphaType alphaType : alphaTypes) {
                bitmap.setInfo(SkImageInfo::Make(4, 4, colorType, canonicalAlphaType));
                bool result = bitmap.setAlphaType(alphaType);
                SkDebugf("%s %s    ", result ? "true " : "false",
                        SkAlphaTypeToString(bitmap.alphaType()));
            }
            SkDebugf("\n");
        }
    }
}
}  // END FIDDLE
