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

#ifdef SK_SUPPORT_LEGACY_DRAWFILTER

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
    auto surface(SkSurface::MakeRasterN32Premul(10, 10));
    SkCanvas* canvas = surface->getCanvas();

    sk_sp<TestFilter> df(new TestFilter);

    REPORTER_ASSERT(reporter, nullptr == canvas->getDrawFilter());

    canvas->save();
    canvas->setDrawFilter(df.get());
    REPORTER_ASSERT(reporter, nullptr != canvas->getDrawFilter());
    canvas->restore();

    REPORTER_ASSERT(reporter, nullptr == canvas->getDrawFilter());
}

DEF_TEST(DrawFilter, reporter) {
    test_saverestore(reporter);
}

#endif
