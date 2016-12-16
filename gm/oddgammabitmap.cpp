/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkColorSpace_Base.h"
#include "SkGradientShader.h"

DEF_SIMPLE_GM_BG(odd_gamma_bitmap, canvas, 640, 220, SK_ColorBLACK) {
    // Draw three boxes. Each should appear the same, assuming we're in color correct mode and that
    // we correctly handle non-standard transfer functions.
    const int N = 200;

    // First, a (true) linear gradient:
    SkPaint p;
    auto linearSrgbColorSpace = SkColorSpace::MakeNamed(SkColorSpace::kSRGBLinear_Named);
    SkPoint points[2] ={ SkPoint::Make(0, 0), SkPoint::Make(0, N) };
    SkColor4f colors[2] ={ { 0.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
    p.setShader(SkGradientShader::MakeLinear(points, colors, linearSrgbColorSpace, nullptr, 2,
                                             SkShader::kClamp_TileMode));
    canvas->drawRect(SkRect::MakeXYWH(10, 10, N, N), p);

    // Now construct a bitmap with an odd (x^8) gamma, that should represent the same gradient.

    SkColorSpaceTransferFn oddGamma;
    oddGamma.fA = 1.0f;
    oddGamma.fB = oddGamma.fC = oddGamma.fD = oddGamma.fE = oddGamma.fF = 0.0f;
    oddGamma.fG = 8.0f;

    sk_sp<SkColorSpace> oddCS =
        SkColorSpace::MakeRGB(oddGamma, *as_CSB(linearSrgbColorSpace)->toXYZD50());
    SkImageInfo info = SkImageInfo::Make(N, N, kN32_SkColorType, kOpaque_SkAlphaType, oddCS);
    SkBitmap bm;
    bm.allocPixels(info);

    for (int y = 0; y < N; y++) {
        float fY = static_cast<float>(y) / (N - 1);
        uint8_t v = SkToU8(sk_float_round2int(powf(fY, 1.0f / 8.0f) * 255));
        for (int x = 0; x < N; x++) {
            SkPMColor* s = bm.getAddr32(x, y);
            *s = (0xFF << 24) | (v << 16) | (v << 8) | (v << 0);
        }
    }

    // Draw the bitmap directly...
    canvas->drawBitmap(bm, N + 20, 10);

    // Wrap it in an image...
    sk_sp<SkImage> img = SkImage::MakeFromBitmap(bm);
    canvas->drawImage(img, 2 * N + 30, 10);
}
