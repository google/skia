/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
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
    // Apply these to an image with identity matrix
    const SkSamplingOptions nearest,                    // no change
                            catmullrom({0, 0.5f}),        // no change
                            mitchell({1.0f/3, 1.0f/3});   // should change

    for (auto name : names) {
        auto src = GetResourceAsImage(name);
        auto surf = SkSurface::MakeRasterN32Premul(src->width(), src->height());
        auto canvas = surf->getCanvas();

        canvas->drawImage(src.get(), 0, 0, nearest, nullptr);
        auto dst0 = surf->makeImageSnapshot();

        canvas->clear(0);
        canvas->drawImage(src.get(), 0, 0, catmullrom, nullptr);
        auto dst1 = surf->makeImageSnapshot();

        canvas->clear(0);
        canvas->drawImage(src.get(), 0, 0, mitchell, nullptr);
        auto dst2 = surf->makeImageSnapshot();

        // No resampling means we expect the same pixels
        REPORTER_ASSERT(r, ToolUtils::equal_pixels(src.get(), dst0.get()));
        // B==0 means we expect the same pixels (https://entropymine.com/imageworsener/bicubic/)
        REPORTER_ASSERT(r, ToolUtils::equal_pixels(src.get(), dst1.get()));
        // B!=0 means the resampler should affect the pixels
        REPORTER_ASSERT(r, !ToolUtils::equal_pixels(src.get(), dst2.get()));
    }
}
