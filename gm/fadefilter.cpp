/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/effects/SkColorFilterImageFilter.h"

#include <utility>

// This GM renders correctly in 8888, but fails in PDF
DEF_SIMPLE_GM(fadefilter, canvas, 256, 256) {
    float matrix[20] = { 1, 0, 0, 0, 0.5f,
                         0, 1, 0, 0, 0.5f,
                         0, 0, 1, 0, 0.5f,
                         0, 0, 0, 1, 0 };
    sk_sp<SkColorFilter> colorFilter(SkColorFilters::Matrix(matrix));
    SkPaint layerPaint;
    layerPaint.setImageFilter(SkColorFilterImageFilter::Make(std::move(colorFilter), nullptr));
    canvas->drawRect(SkRect::MakeLTRB(64, 64, 192, 192), layerPaint);
}
