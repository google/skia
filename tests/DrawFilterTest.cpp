/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDrawFilter.h"
#include "SkSurface.h"
#include "Test.h"

namespace {
class TestFilter : public SkDrawFilter {
public:
    bool filter(SkPaint* p, Type) override {
        return true;
    }
};
}

/**
 *  canvas.setDrawFilter is defined to be local to the save/restore block, such that if you
 *  do the following: save / modify-drawfilter / restore, the current drawfilter should be what
 *  it was before the save.
 */
static void test_saverestore(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(10, 10));
    SkCanvas* canvas = surface->getCanvas();


    SkAutoTUnref<TestFilter> df(SkNEW(TestFilter));

    REPORTER_ASSERT(reporter, NULL == canvas->getDrawFilter());

    canvas->save();
    canvas->setDrawFilter(df);
    REPORTER_ASSERT(reporter, NULL != canvas->getDrawFilter());
    canvas->restore();

    REPORTER_ASSERT(reporter, NULL == canvas->getDrawFilter());
}

DEF_TEST(DrawFilter, reporter) {
    test_saverestore(reporter);
}
