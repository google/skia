/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkImageSource.h"
#include "SkSurface.h"
#include "Test.h"

DEF_TEST(skbug_6389, r) {
    auto s = SkSurface::MakeRasterN32Premul(100, 100);
    SkPaint p;
    p.setMaskFilter(SkBlurMaskFilter::Make(SkBlurStyle::kNormal_SkBlurStyle, 5,
                                           SkBlurMaskFilter::kHighQuality_BlurFlag));
    p.setImageFilter(SkImageSource::Make(GetResourceAsImage("mandrill_512.png"), {0, 0, 0, 0},
                                         {0, 0, 0, 0}, (SkFilterQuality)0));
    s->getCanvas()->drawPaint(p);
}
