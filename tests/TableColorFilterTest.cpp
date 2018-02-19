/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkColorSpace.h"
#include "SkTableColorFilter.h"
#include "SkToSRGBColorFilter.h"

// SkToSRGBColorFilter makes it easy to create out of range (>1, <0) color values.
// Those can be dangerous as inputs to naive implementation of SkTableColorFilter.
// This tests that our implementation is safe.

DEF_TEST(TableColorFilter, r) {
    // Using a wide source gamut will make saturated colors go well out of range of sRGB.
    auto rec2020 = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                         SkColorSpace::kRec2020_Gamut);
    sk_sp<SkColorFilter> to_srgb = SkToSRGBColorFilter::Make(rec2020);

    // Any table will work fine here.  An identity table makes testing easy.
    uint8_t identity[256];
    for (int i = 0; i < 256; i++) {
        identity[i] = i;
    }
    sk_sp<SkColorFilter> table = SkTableColorFilter::Make(identity);

    // The rec2020 primaries are not representable in sRGB, but will clamp to the sRGB primaries.
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    sk_sp<SkColorFilter> composed = table->makeComposed(to_srgb);
    for (auto color : colors) {
        REPORTER_ASSERT(r, composed->filterColor(color) == color);
    }
}
