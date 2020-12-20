/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/utils/SkRandom.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

// In general, cubic resampling affects the pixels, even with no transformation (identity matrix)
// This tests that, plus the special case for cubics where B == 0
//
DEF_TEST(sampling_cubic_identity, r) {
    const char* names[] = {
        "images/mandrill_128.png", "images/color_wheel.jpg",
    };

    constexpr int N = 20;   // try a bunch of random values

    SkRandom rand;
    for (auto name : names) {
        auto src = GetResourceAsImage(name);
        auto surf = SkSurface::MakeRasterN32Premul(src->width(), src->height());
        auto canvas = surf->getCanvas();

        canvas->drawImage(src.get(), 0, 0, SkSamplingOptions(), nullptr);
        auto dst = surf->makeImageSnapshot();
        // easy case -- make sure drawing with no sampling results in no changes
        REPORTER_ASSERT(r, ToolUtils::equal_pixels(src.get(), dst.get()));

        // Exercise variants where B == 0 -- expecting no changes
        for (int i = 0; i < N; ++i) {
            canvas->clear(0);
            canvas->drawImage(src.get(), 0, 0, SkSamplingOptions({0, rand.nextF()}), nullptr);
            dst = surf->makeImageSnapshot();
            REPORTER_ASSERT(r, ToolUtils::equal_pixels(src.get(), dst.get()));
        }

        // Exercise variants where B != 0 -- expecting changes
        for (int i = 0; i < N; ++i) {
            float B = rand.nextF() * 0.9f + 0.05f;  // non-zero but still within (0,,,1]
            canvas->clear(0);
            canvas->drawImage(src.get(), 0, 0, SkSamplingOptions({B, rand.nextF()}), nullptr);
            dst = surf->makeImageSnapshot();
            REPORTER_ASSERT(r, !ToolUtils::equal_pixels(src.get(), dst.get()));
        }
    }
}
