/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGradientShader.h"
#include "SkSurface.h"
#include "gm.h"

// We'll test drawing into a couple of destinations with unusual transfer functions.
// When drawn into a legacy configuration, you should see 3 distinct gradients.

DEF_SIMPLE_GM(numerical_dst, canvas, 128*3, 160) {
    auto draw = [canvas](const char* label, SkImageInfo native, SkImageInfo unusual) {
        SkPaint gradient;
        SkPoint points[] = { {0,0}, {0,128} };
        SkColor colors[] = { SK_ColorBLACK, SK_ColorWHITE };
        gradient.setShader(
            SkGradientShader::MakeLinear(points, colors, nullptr, 2, SkShader::kClamp_TileMode));

        SkPaint text;
        text.setColor(SK_ColorBLACK);

        if (auto surface = canvas->makeSurface(unusual)) {
            // Draw into our unusual colorspace.
            surface->getCanvas()->drawRect({0,0,128,128}, gradient);

            // Read back unchanged to CPU memory.
            SkBitmap bm;
            bm.allocPixels(unusual);
            surface->readPixels(bm, 0,0);

            // Pun those pixels into native colorspace.
            SkBitmap pun;
            pun.setInfo(native);
            pun.setPixels(bm.getAddr(0,0));

            // Draw the punned pixels.
            canvas->drawBitmap(pun, 0,0);
        } else {
            canvas->drawString("Can't create surface.", 0,64, text);
        }
        canvas->drawString(label, 0,144, text);
        canvas->translate(128,0);
    };

    // gamma=2.9
    auto gamma_2_9 = SkColorSpace::MakeRGB(
            SkColorSpaceTransferFn{2.9f,1.0f,0,0,0,0,0},
            SkColorSpace::kSRGB_Gamut);
    SkASSERT(!gamma->gammaCloseToSRGB());

    // sRGB with a mildly different linear/exponential threshold (0.04045f -> 0.039f).
    auto numerical = SkColorSpace::MakeRGB(
            SkColorSpaceTransferFn{2.4f,0.948f,0.052f,0.077f,0.039f,0,0},
            SkColorSpace::kSRGB_Gamut);
    SkASSERT(!numerical->gammaCloseToSRGB());

    auto native = canvas->imageInfo();
    draw("native",    native, native);
    draw("gamma=2.9", native, native.makeColorSpace(gamma_2_9));
    draw("numerical", native, native.makeColorSpace(numerical));
}
