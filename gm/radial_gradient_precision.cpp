/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradient.h"

// All we're looking for here is that we see a smooth gradient.
DEF_SIMPLE_GM(radial_gradient_precision, canvas, 200, 200) {
    SkPoint  center   = {1000, 1000};
    SkScalar radius   = 40;
    SkColor4f colors[] = {SkColors::kBlack, SkColors::kGreen};

    SkPaint p;
    p.setShader(SkShaders::RadialGradient(center, radius, {{colors, {}, SkTileMode::kRepeat}, {}}));
    canvas->drawPaint(p);
}
