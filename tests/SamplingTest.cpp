/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "src/base/SkRandom.h"
#include "src/core/SkSamplingPriv.h"
#include "tests/Test.h"
#include "tools/DecodeUtils.h"
#include "tools/ToolUtils.h"

#include <initializer_list>

// In general, sampling under identity matrix should not affect the pixels. However,
// cubic resampling when B != 0 is expected to change pixels.
//
DEF_TEST(sampling_with_identity_matrix, r) {
    const char* names[] = {
        "images/mandrill_128.png", "images/color_wheel.jpg",
    };

    SkRandom rand;
    for (auto name : names) {
        auto src = ToolUtils::GetResourceAsImage(name);
        auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(src->width(), src->height()));
        auto canvas = surf->getCanvas();

        auto dotest = [&](const SkSamplingOptions& sampling, bool expect_same) {
            canvas->clear(0);
            canvas->drawImage(src.get(), 0, 0, sampling, nullptr);
            auto dst = surf->makeImageSnapshot();

            REPORTER_ASSERT(r, SkSamplingPriv::NoChangeWithIdentityMatrix(sampling) == expect_same);
            REPORTER_ASSERT(r, ToolUtils::equal_pixels(src.get(), dst.get()) == expect_same);
        };

        // Exercise all non-cubics -- expecting no changes
        for (auto m : {SkMipmapMode::kNone, SkMipmapMode::kNearest, SkMipmapMode::kLinear}) {
            for (auto f : {SkFilterMode::kNearest, SkFilterMode::kLinear}) {
                dotest(SkSamplingOptions(f, m), true);
            }
        }

        // Exercise cubic variants with B zero and non-zero
        constexpr int N = 30;   // try a bunch of random values
        for (int i = 0; i < N; ++i) {
            float C = rand.nextF();
            dotest(SkSamplingOptions({0, C}), true);

            float B = rand.nextF() * 0.9f + 0.05f;  // non-zero but still within (0,,,1]
            SkASSERT(B != 0);
            dotest(SkSamplingOptions({B, C}), false);
        }
    }
}
