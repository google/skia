/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkColorPriv.h"
#include "SkColorSpace.h"
#include "SkColorSpaceXform.h"
#include "Test.h"

DEF_TEST(SkColorSpaceXform_LoadTail, r) {
    std::unique_ptr<uint64_t[]> srcPixel(new uint64_t[1]);
    srcPixel[0] = 0;
    uint32_t dstPixel;
    sk_sp<SkColorSpace> p3 = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                                   SkColorSpace::kDCIP3_D65_Gamut);
    sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();
    std::unique_ptr<SkColorSpaceXform> xform = SkColorSpaceXform::New(p3.get(), srgb.get());

    // ASAN will catch us if we read past the tail.
    bool success = xform->apply(SkColorSpaceXform::kRGBA_8888_ColorFormat, &dstPixel,
                                SkColorSpaceXform::kRGBA_U16_BE_ColorFormat, srcPixel.get(), 1,
                                kUnpremul_SkAlphaType);
    REPORTER_ASSERT(r, success);
}
