/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkGradientShader.h"

// All we're looking for here is that we see a smooth gradient.
DEF_SIMPLE_GM(radial_gradient_precision, canvas, 200, 200) {
    SkPoint  center   = {100000, 100000};
    SkScalar radius   = 40;
    SkColor  colors[] = {SK_ColorBLACK, SK_ColorWHITE};

    SkPaint p;
    p.setShader(SkGradientShader::MakeRadial(center, radius,
                                             colors, nullptr, SK_ARRAY_COUNT(colors),
                                             SkShader::kRepeat_TileMode));
    canvas->drawPaint(p);
}
