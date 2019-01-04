/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace.h"
#include "SkToSRGBColorFilter.h"
#include "Test.h"


DEF_TEST(SkToSRGBColorFilter, r) {

    // sRGB -> sRGB is a no-op.
    REPORTER_ASSERT(r, nullptr == SkToSRGBColorFilter::Make(SkColorSpace::MakeSRGB()));

    // The transfer function matters just as much as the gamut.
    REPORTER_ASSERT(r, nullptr != SkToSRGBColorFilter::Make(SkColorSpace::MakeSRGBLinear()));

    // We generally interpret nullptr source spaces as sRGB.  See also chromium:787718.
    REPORTER_ASSERT(r, nullptr == SkToSRGBColorFilter::Make(nullptr));

    // Here's a realistic conversion.
    auto dci_p3 = SkColorSpace::MakeRGB(SkColorSpace::kLinear_RenderTargetGamma,
                                        SkColorSpace::kDCIP3_D65_Gamut);
    REPORTER_ASSERT(r, nullptr != SkToSRGBColorFilter::Make(dci_p3));

}
