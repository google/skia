/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkColorSpace_New.h"
#include <initializer_list>

DEF_TEST(SkColorSpace_New_TransferFnBasics, r) {
    auto gamut = SkMatrix44::I();
    auto blending = SkColorSpace_New::Blending::AsEncoded;

    SkColorSpace_New linearA{SkColorSpace_New::TransferFn::MakeLinear(),    gamut, blending},
                     linearB{SkColorSpace_New::TransferFn::MakeGamma(1),    gamut, blending},
                        srgb{SkColorSpace_New::TransferFn::MakeSRGB(),      gamut, blending},
                       gamma{SkColorSpace_New::TransferFn::MakeGamma(2.2f), gamut, blending};

    REPORTER_ASSERT(r,  linearA.gammaIsLinear());
    REPORTER_ASSERT(r,  linearB.gammaIsLinear());
    REPORTER_ASSERT(r, !   srgb.gammaIsLinear());
    REPORTER_ASSERT(r, !  gamma.gammaIsLinear());

    REPORTER_ASSERT(r, !linearA.gammaCloseToSRGB());
    REPORTER_ASSERT(r, !linearB.gammaCloseToSRGB());
    REPORTER_ASSERT(r,     srgb.gammaCloseToSRGB());
    REPORTER_ASSERT(r, !  gamma.gammaCloseToSRGB());

    REPORTER_ASSERT(r,  linearA.transferFn().equals(linearB.transferFn()));
    REPORTER_ASSERT(r, !linearA.transferFn().equals(   srgb.transferFn()));
    REPORTER_ASSERT(r, !linearA.transferFn().equals(  gamma.transferFn()));
    REPORTER_ASSERT(r, !linearB.transferFn().equals(   srgb.transferFn()));
    REPORTER_ASSERT(r, !linearB.transferFn().equals(  gamma.transferFn()));
    REPORTER_ASSERT(r, !   srgb.transferFn().equals(  gamma.transferFn()));
}

DEF_TEST(SkColorSpace_New_TransferFnStages, r) {

}
