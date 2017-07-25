/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkPM4fPriv.h"
#include "SkToSRGBColorFilter.h"

DEF_SIMPLE_GM_BG(tosrgb_colorfilter, canvas, 130, 130, SK_ColorBLACK) {
    // Src bitmap with some colors that we're going to interpret as being in a few different spaces
    SkBitmap bmp;
    bmp.allocN32Pixels(3, 2);
    SkPMColor* pixels = reinterpret_cast<SkPMColor*>(bmp.getPixels());
    pixels[0] = SkPackARGB32(0xFF, 0xA0, 0x00, 0x00);
    pixels[1] = SkPackARGB32(0xFF, 0x00, 0xA0, 0x00);
    pixels[2] = SkPackARGB32(0xFF, 0x00, 0x00, 0xA0);
    pixels[3] = SkPackARGB32(0xFF, 0x00, 0xA0, 0xA0);
    pixels[4] = SkPackARGB32(0xFF, 0xA0, 0x00, 0xA0);
    pixels[5] = SkPackARGB32(0xFF, 0xA0, 0xA0, 0x00);

    // Reference image
    canvas->drawBitmapRect(bmp, SkRect::MakeXYWH(10, 10, 50, 50), nullptr);

    auto srgb = SkColorSpace::MakeSRGB();
    auto rec2020 = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                         SkColorSpace::kRec2020_Gamut);

    // NarrowGamut RGB (an artifically smaller than sRGB gamut)
    SkColorSpacePrimaries narrowPrimaries = {
        0.54f, 0.33f,     // Rx, Ry
        0.33f, 0.50f,     // Gx, Gy
        0.25f, 0.20f,     // Bx, By
        0.3127f, 0.3290f, // Wx, Wy
    };
    SkMatrix44 narrowGamutRGBMatrix(SkMatrix44::kUninitialized_Constructor);
    narrowPrimaries.toXYZD50(&narrowGamutRGBMatrix);
    auto narrow = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                        narrowGamutRGBMatrix);

    SkPaint paint;

    // Transforming sRGB -> sRGB should do nothing. Top two squares should look identical.
    paint.setColorFilter(SkToSRGBColorFilter::Make(srgb));
    canvas->drawBitmapRect(bmp, SkRect::MakeXYWH(70, 10, 50, 50), &paint);

    // Rec2020 -> sRGB should produce more vivid colors.
    paint.setColorFilter(SkToSRGBColorFilter::Make(rec2020));
    canvas->drawBitmapRect(bmp, SkRect::MakeXYWH(10, 70, 50, 50), &paint);

    // Narrow -> sRGB should produce more muted colors.
    paint.setColorFilter(SkToSRGBColorFilter::Make(narrow));
    canvas->drawBitmapRect(bmp, SkRect::MakeXYWH(70, 70, 50, 50), &paint);
}
