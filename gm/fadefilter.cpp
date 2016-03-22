/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorMatrixFilter.h"
#include "SkColorFilterImageFilter.h"

// This GM renders correctly in 8888, but fails in PDF
DEF_SIMPLE_GM(fadefilter, canvas, 256, 256) {
    SkScalar matrix[20] = { 1, 0, 0, 0, 128.0f,
                            0, 1, 0, 0, 128.0f,
                            0, 0, 1, 0, 128.0f,
                            0, 0, 0, 1, 0 };
    SkAutoTUnref<SkColorFilter> colorFilter(
            SkColorMatrixFilter::Create(matrix));
    SkAutoTUnref<SkImageFilter> filter(
            SkColorFilterImageFilter::Create(colorFilter));
    SkPaint layerPaint;
    layerPaint.setImageFilter(filter);
    canvas->drawRect(SkRect::MakeLTRB(64, 64, 192, 192), layerPaint);
}
