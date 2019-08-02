/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"

DEF_SIMPLE_GM(crbug_905548, canvas, 100, 200) {
    auto surface = canvas->makeSurface(SkImageInfo::MakeN32Premul(100, 100));
    if (!surface) {
        surface = SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(100, 100));
    }
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawCircle(50, 50, 45, SkPaint());
    auto imageSource = SkImageFilters::Image(surface->makeImageSnapshot());

    auto blurred = SkImageFilters::Blur(15, 15, imageSource);
    auto eroded = SkImageFilters::Erode(0, 0, blurred);
    auto blended = SkImageFilters::Xfermode(SkBlendMode::kDstOut, eroded, imageSource, nullptr);

    SkPaint paint;
    paint.setImageFilter(blended);
    canvas->drawRect(SkRect::MakeWH(100, 100), paint);

    auto mult = SkImageFilters::Arithmetic(1, 0, 0, 0, false, eroded, imageSource, nullptr);
    paint.setImageFilter(mult);
    canvas->translate(0, 100);
    canvas->drawRect(SkRect::MakeWH(100, 100), paint);
}
