/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/effects/SkImageFilters.h"

static void drawOne(SkCanvas* canvas, SkRect rect, float saveBorder, float sigma, SkColor c) {
    SkRect borderRect = rect.makeOutset(saveBorder, saveBorder);

    SkPaint p;
    p.setColor(c);
    p.setImageFilter(
        SkImageFilters::Blur(sigma, sigma,
            // The blur's input is forced to have transparent padding because 'borderRect' is outset
            // from the non-transparent content ('rect') that's drawn into the layer.
            SkImageFilters::Crop(borderRect, SkTileMode::kClamp, nullptr),
            // The blur's output crop visually won't affect the output because the transparent
            // padding is blurred out by the edge of 3*sigma.
            borderRect.makeOutset(3 * sigma, 3 * sigma)));
    p.setAntiAlias(true);

    canvas->drawRect(rect, p);
}

DEF_SIMPLE_GM(crbug_1156804, canvas, 250, 250) {
    drawOne(canvas, SkRect::MakeXYWH( 64,  64, 25, 25),  1,  3, SK_ColorGREEN);
    drawOne(canvas, SkRect::MakeXYWH(164,  64, 25, 25), 30,  3, SK_ColorGREEN);
    // This one would draw incorrectly because the large sigma causes downscaling of the source
    // and the one-pixel border would make the downscaled image not contain trans-black at the
    // edges. Combined with the clamp mode on the blur filter it would "harden" the edge.
    drawOne(canvas, SkRect::MakeXYWH( 64, 164, 25, 25),  1, 20, SK_ColorRED);
    drawOne(canvas, SkRect::MakeXYWH(164, 164, 25, 25), 30, 20, SK_ColorGREEN);
}
