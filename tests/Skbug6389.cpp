/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageSource.h"
#include "tests/Test.h"
#include "tools/Resources.h"

DEF_TEST(skbug_6389, r) {
    auto s = SkSurface::MakeRasterN32Premul(100, 100);
    SkPaint p;
    p.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 5));
    p.setImageFilter(SkImageSource::Make(GetResourceAsImage("images/mandrill_512.png"), {0, 0, 0, 0},
                                         {0, 0, 0, 0}, (SkFilterQuality)0));
    s->getCanvas()->drawPaint(p);
}
