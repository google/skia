/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkRandom.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/Rectanizer.h"
#include "src/gpu/RectanizerPow2.h"
#include "src/gpu/RectanizerSkyline.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

struct GrContextOptions;

using namespace skgpu;

static const int kWidth = 1024;
static const int kHeight = 1024;

// Basic test of a Rectanizer-derived class' functionality
static void test_rectanizer_basic(skiatest::Reporter* reporter, Rectanizer* rectanizer) {
    REPORTER_ASSERT(reporter, kWidth == rectanizer->width());
    REPORTER_ASSERT(reporter, kHeight == rectanizer->height());

    SkIPoint16 loc;

    REPORTER_ASSERT(reporter, rectanizer->addRect(50, 50, &loc));
    REPORTER_ASSERT(reporter, rectanizer->percentFull() > 0.0f);
    rectanizer->reset();
    REPORTER_ASSERT(reporter, rectanizer->percentFull() == 0.0f);
}

static void test_rectanizer_inserts(skiatest::Reporter*,
                                    Rectanizer* rectanizer,
                                    const SkTDArray<SkISize>& rects) {
    int i;
    for (i = 0; i < rects.size(); ++i) {
        SkIPoint16 loc;
        if (!rectanizer->addRect(rects[i].fWidth, rects[i].fHeight, &loc)) {
            break;
        }
    }

    //SkDebugf("\n***%d %f\n", i, rectanizer->percentFull());
}

static void test_skyline(skiatest::Reporter* reporter, const SkTDArray<SkISize>& rects) {
    RectanizerSkyline skylineRectanizer(kWidth, kHeight);

    test_rectanizer_basic(reporter, &skylineRectanizer);
    test_rectanizer_inserts(reporter, &skylineRectanizer, rects);
}

static void test_pow2(skiatest::Reporter* reporter, const SkTDArray<SkISize>& rects) {
    RectanizerPow2 pow2Rectanizer(kWidth, kHeight);

    test_rectanizer_basic(reporter, &pow2Rectanizer);
    test_rectanizer_inserts(reporter, &pow2Rectanizer, rects);
}

DEF_GANESH_TEST(GpuRectanizer, reporter, factory, CtsEnforcement::kNever) {
    SkTDArray<SkISize> rects;
    SkRandom rand;

    for (int i = 0; i < 50; i++) {
        rects.push_back(SkISize::Make(rand.nextRangeU(1, kWidth / 2),
                                      rand.nextRangeU(1, kHeight / 2)));
    }

    test_skyline(reporter, rects);
    test_pow2(reporter, rects);
}
