/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"

DEF_SIMPLE_GM(crbug_899512, canvas, 520, 520) {
    // comment this line below to solve the problem
    SkMatrix matrix;
    matrix.setAll(-1, 0, 220, 0, 1, 0, 0, 0, 1);
    canvas->concat(matrix);
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 6.2735f, false));
    paint.setColorFilter(SkColorFilters::Blend(SK_ColorBLACK, SkBlendMode::kSrcIn));
    canvas->drawRect(SkRect::MakeXYWH(0, 10, 200, 200), paint);
}
