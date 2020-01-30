/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkSize.h"
#include "include/private/SkTDArray.h"
#include "include/utils/SkRandom.h"
#include "src/gpu/GrRectanizerSkyline.h"
#include "tests/Test.h"

static const int kWidth = 1024;
static const int kHeight = 1024;

// Basic test of a GrRectanizer-derived class' functionality
static void test_rectanizer_basic(skiatest::Reporter* reporter, GrRectanizerSkyline* rectanizer) {
    REPORTER_ASSERT(reporter, kWidth == rectanizer->width());
    REPORTER_ASSERT(reporter, kHeight == rectanizer->height());

    SkIPoint16 loc;

    REPORTER_ASSERT(reporter, rectanizer->addRect(50, 50, &loc));
    rectanizer->reset();
}

static void test_rectanizer_inserts(skiatest::Reporter*,
                                    GrRectanizerSkyline* rectanizer,
                                    const SkTDArray<SkISize>& rects) {
    int i;
    for (i = 0; i < rects.count(); ++i) {
        SkIPoint16 loc;
        if (!rectanizer->addRect(rects[i].fWidth, rects[i].fHeight, &loc)) {
            break;
        }
    }

    //SkDebugf("\n***%d %f\n", i, rectanizer->percentFull());
}

static void test_skyline(skiatest::Reporter* reporter, const SkTDArray<SkISize>& rects) {
    GrRectanizerSkyline skylineRectanizer(kWidth, kHeight);

    test_rectanizer_basic(reporter, &skylineRectanizer);
    test_rectanizer_inserts(reporter, &skylineRectanizer, rects);
}

DEF_GPUTEST(GpuRectanizer, reporter, factory) {
    SkTDArray<SkISize> rects;
    SkRandom rand;

    for (int i = 0; i < 50; i++) {
        rects.push_back(SkISize::Make(rand.nextRangeU(1, kWidth / 2),
                                      rand.nextRangeU(1, kHeight / 2)));
    }

    test_skyline(reporter, rects);
}
