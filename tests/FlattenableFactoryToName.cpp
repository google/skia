/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAlphaThresholdFilter.h"
#include "SkImage.h"
#include "Test.h"

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
    SkAutoTUnref<SkImageFilter> filter( SkAlphaThresholdFilter::Create(region, 0.2f, 0.7f));
    test_flattenable(r, filter, "SkAlphaThresholdFilter()");

    SkBitmap bm;
    bm.allocN32Pixels(8, 8);
    bm.eraseColor(SK_ColorCYAN);
    SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bm));
    SkAutoTUnref<SkShader> shader(image->newShader(SkShader::kClamp_TileMode,
                                                   SkShader::kClamp_TileMode));
    test_flattenable(r, shader, "SkImage::newShader()");
}
