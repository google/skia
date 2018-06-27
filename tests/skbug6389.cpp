/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCanvas.h"
#include "SkImageSource.h"
#include "SkMaskFilter.h"
#include "SkSurface.h"
#include "Test.h"

DEF_TEST(skbug_6389, r) {
    auto s = SkSurface::MakeRasterN32Premul(100, 100);
    SkPaint p;
    p.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 5));
    p.setImageFilter(SkImageSource::Make(GetResourceAsImage("images/mandrill_512.png"), {0, 0, 0, 0},
                                         {0, 0, 0, 0}, (SkFilterQuality)0));
    s->getCanvas()->drawPaint(p);
}
