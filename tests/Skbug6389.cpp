/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "tests/Test.h"
#include "tools/DecodeUtils.h"

DEF_TEST(skbug_6389, r) {
    auto s = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));
    SkPaint p;
    p.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 5));
    p.setImageFilter(SkImageFilters::Image(ToolUtils::GetResourceAsImage("images/mandrill_512.png"),
                                           {0, 0, 0, 0},
                                           {0, 0, 0, 0},
                                           SkSamplingOptions()));
    s->getCanvas()->drawPaint(p);
}
