/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorSpace.h"
#include "SkImageInfo.h"

// Test to imitate android.graphics.cts.BitmapTest#testCopyConfigs. With ag/5803642, the test now
// fails when copying from F16 to 565. The expectation is that the 565 Bitmap's Color will match
// the Color we erased to.
DEF_TEST(Bitmap_copy, r) {
    // If the F16 Bitmap is SRGB (as it is in master without my patch), the test passes.
    auto sRGB = SkColorSpace::MakeSRGB();
    auto info = SkImageInfo::Make(1, 1, kRGBA_F16_SkColorType, kOpaque_SkAlphaType,
                                  SkColorSpace::MakeSRGBLinear());
    SkBitmap f16;
    f16.allocPixels(info);
    f16.eraseColor(SK_ColorWHITE);

    // This imitates Bitmap#copy. I added the change to the SkColorSpace, since currently, Android
    // treats/expects 565 to be SRGB.
    SkBitmap b565;
    b565.allocPixels(info.makeColorType(kRGB_565_SkColorType).makeColorSpace(sRGB));
    if (!f16.pixmap().readPixels(b565.pixmap())) {
        ERRORF(r, "Failed to copy pixels");
        return;
    }

    // This imitates Bitmap#getPixel.
    auto readInfo = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType, sRGB);
    SkColor color;
    b565.readPixels(readInfo, &color, readInfo.minRowBytes(), 0, 0);
    if (color != SK_ColorWHITE) {
        ERRORF(r, "Expected white, got %x", color);
    }
}
