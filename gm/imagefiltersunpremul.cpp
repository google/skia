/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkImageFilters.h"

DEF_SIMPLE_GM_BG(imagefiltersunpremul, canvas, 64, 64, SK_ColorBLACK) {
    // Draw an kUnpremul_SkAlphaType image using SkImageFilters::Image() and
    // verify alpha channel was blended correctly.
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType));
    bitmap.eraseColor(SkColorSetARGB(50, 255, 0, 0));
    SkPaint paint;
    paint.setImageFilter(SkImageFilters::Image(SkImages::RasterFromBitmap(bitmap),
                                               SkCubicResampler::Mitchell()));
    canvas->drawPaint(paint);
}
