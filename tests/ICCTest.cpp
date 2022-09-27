/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include "include/core/SkICC.h"
#include "include/core/SkString.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkColorSpacePriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"

DEF_TEST(AdobeRGB, r) {
    if (sk_sp<SkData> profile = GetResourceAsData("icc_profiles/AdobeRGB1998.icc")) {
        skcms_ICCProfile parsed;
        REPORTER_ASSERT(r, skcms_Parse(profile->data(), profile->size(), &parsed));
        REPORTER_ASSERT(r, !parsed.has_CICP);

        auto got  = SkColorSpace::Make(parsed);
        auto want = SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB);
        REPORTER_ASSERT(r, SkColorSpace::Equals(got.get(), want.get()));
    }
}

DEF_TEST(PQ, r) {
    SK_API sk_sp<SkData> profile =
            SkWriteICCProfile(SkNamedTransferFn::kPQ, SkNamedGamut::kRec2020);
    skcms_ICCProfile parsed;
    REPORTER_ASSERT(r, skcms_Parse(profile->data(), profile->size(), &parsed));
    REPORTER_ASSERT(r, parsed.has_CICP);
    REPORTER_ASSERT(r, parsed.CICP.color_primaries == 9);
    REPORTER_ASSERT(r, parsed.CICP.transfer_characteristics == 16);
    REPORTER_ASSERT(r, parsed.CICP.matrix_coefficients == 0);
    REPORTER_ASSERT(r, parsed.CICP.video_full_range_flag == 1);
}

DEF_TEST(HLG, r) {
    SK_API sk_sp<SkData> profile =
            SkWriteICCProfile(SkNamedTransferFn::kHLG, SkNamedGamut::kDisplayP3);
    skcms_ICCProfile parsed;
    REPORTER_ASSERT(r, skcms_Parse(profile->data(), profile->size(), &parsed));
    REPORTER_ASSERT(r, parsed.has_CICP);
    REPORTER_ASSERT(r, parsed.CICP.color_primaries == 12);
    REPORTER_ASSERT(r, parsed.CICP.transfer_characteristics == 18);
    REPORTER_ASSERT(r, parsed.CICP.matrix_coefficients == 0);
    REPORTER_ASSERT(r, parsed.CICP.video_full_range_flag == 1);
}
