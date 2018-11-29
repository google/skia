/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkBlurImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkXfermodeImageFilter.h"
#include "SkSurface.h"
#include "SkImageSource.h"

DEF_SIMPLE_GM(crbug_905548, canvas, 100, 100) {
    auto surface = canvas->makeSurface(SkImageInfo::MakeN32Premul(100, 100));
    if (!surface) {
        surface = SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(100, 100));
    }
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawCircle(50, 50, 45, SkPaint());
    auto imageSource = SkImageSource::Make(surface->makeImageSnapshot());

    auto blurred = SkBlurImageFilter::Make(15, 15, imageSource);
    auto eroded = SkErodeImageFilter::Make(0, 0, blurred);
    auto blended = SkXfermodeImageFilter::Make(SkBlendMode::kDstOut, eroded, imageSource, nullptr);

    SkPaint paint;
    paint.setImageFilter(blended);
    canvas->drawPaint(paint);
}
