/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkToSRGBColorFilter.h"
#include "SkColorSpace.h"

DEF_SIMPLE_GM(tosrgb, canvas, 300, 300) {
    // We'll draw medium red with a cross product of various gammas and gamuts.

    SkColorSpaceTransferFn gammas[] = {
        { 1.2f,1.0f, 0,0,0,0,0 },
        { 2.2f,1.0f, 0,0,0,0,0 },
        { 3.2f,1.0f, 0,0,0,0,0 },
    };

    SkColorSpace::Gamut gamuts[] = {
        SkColorSpace::kSRGB_Gamut,
        SkColorSpace::kAdobeRGB_Gamut,
        SkColorSpace::kRec2020_Gamut,
    };

    SkPaint paint;
    paint.setColor(0xff7f0000);

    for (const auto& gamma : gammas) {
        for (const auto& gamut : gamuts) {
            paint.setColorFilter(SkToSRGBColorFilter::Make(SkColorSpace::MakeRGB(gamma, gamut)));

            canvas->drawRect({25,25,75,75}, paint);

            canvas->translate(100,0);
        }
        canvas->translate(-300,100);
    }
}
