/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/effects/SkImageFilters.h"
#include "tests/Test.h"

static void test_flattenable(skiatest::Reporter* r,
                             const SkFlattenable* f,
                             const char* desc) {
    if (f) {
        SkFlattenable::Factory factory = f->getFactory();
        REPORTER_ASSERT(r, factory);
        if (factory) {
            if (!SkFlattenable::FactoryToName(factory)) {
                ERRORF(r, "SkFlattenable::FactoryToName() fails with %s.", desc);
            }
        }
    }
}

DEF_TEST(FlattenableFactoryToName, r) {
    SkIRect rects[2];
    rects[0] = SkIRect::MakeXYWH(0, 150, 500, 200);
    rects[1] = SkIRect::MakeXYWH(150, 0, 200, 500);
    SkRegion region;
    region.setRects(rects, 2);
    sk_sp<SkImageFilter> filter(SkImageFilters::AlphaThreshold(region, 0.2f, 0.7f, nullptr));
    test_flattenable(r, filter.get(), "SkImageFilters::AlphaThreshold()");

    SkBitmap bm;
    bm.allocN32Pixels(8, 8);
    bm.eraseColor(SK_ColorCYAN);
    sk_sp<SkImage> image(bm.asImage());
    test_flattenable(r, image->makeShader(SkSamplingOptions()).get(), "SkImage::newShader()");
}
