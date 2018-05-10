/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkHalf.h"
#include "SkSurface.h"
#include "SkCanvas.h"

DEF_TEST(NonlinearBlending, r) {

    // First check our familiar basics with linear F16.
    {
        auto info = SkImageInfo::Make(1,1, kRGBA_F16_SkColorType, kPremul_SkAlphaType,
                                      SkColorSpace::MakeSRGBLinear());

        auto surface = SkSurface::MakeRaster(info);
        surface->getCanvas()->clear(0xff808080);
        uint64_t pix;
        REPORTER_ASSERT(r, surface->readPixels(info, &pix, sizeof(pix),0,0));

        // 0x80 in sRGB is ≈ 0.22 linear.
        REPORTER_ASSERT(r, SkHalfToFloat(pix & 0xffff) < 0.25f);
    }

    // Test that we support sRGB-encoded F16.  This is somewhat new.
    {
        auto info = SkImageInfo::Make(1,1, kRGBA_F16_SkColorType, kPremul_SkAlphaType,
                                      SkColorSpace::MakeSRGB());

        auto surface = SkSurface::MakeRaster(info);
        surface->getCanvas()->clear(0xff808080);
        uint64_t pix;
        REPORTER_ASSERT(r, surface->readPixels(info, &pix, sizeof(pix),0,0));

        // 0x80 sRGB is ≈ 0.501.
        REPORTER_ASSERT(r, SkHalfToFloat(pix & 0xffff) >= 0.5f);
    }

}
