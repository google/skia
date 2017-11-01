/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkSurface.h"
#include "SkGradientShader.h"

// This GM should make dithering somewhat visible when drawn into 565.

DEF_SIMPLE_GM(dither, canvas, 256,256) {
    // Create a compatible surface that's 8x8, as large as the largest dither pattern we use.
    auto surface = canvas->makeSurface(canvas->imageInfo().makeWH(8,8));
    if (!surface) {
        surface = SkSurface::MakeRasterN32Premul(8,8);
    }

    // Draw a vertical red->green gradient.
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setDither(true);
    SkPoint    pts[] = { {0,0}, {0,8} };
    SkColor colors[] = { 0xffff0000, 0xff00ff00 };
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                                 SkShader::kClamp_TileMode));
    surface->getCanvas()->drawPaint(paint);

    // Snap that and draw with a 32x zoom.
    canvas->scale(32,32);
    canvas->drawImage(surface->makeImageSnapshot(), 0,0);
}
