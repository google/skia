/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkColorSpacePriv.h"
#include "SkICC.h"
#include "Test.h"

DEF_TEST(WriteICCProfile, r) {
    struct {
        SkColorSpaceTransferFn fn;
        const float*           toXYZD50;
        sk_sp<SkColorSpace>    want;
    } tests[] = {
        {g2Dot2_TransferFn, gAdobeRGB_toXYZD50, nullptr },
        { gSRGB_TransferFn,     gSRGB_toXYZD50, SkColorSpace::MakeSRGB()},
    };

    for (auto test : tests) {
        sk_sp<SkData> profile = SkWriteICCProfile(test.fn, test.toXYZD50);
        REPORTER_ASSERT(r, profile);

        sk_sp<SkColorSpace> got = SkColorSpace::MakeICC(profile->data(), profile->size());
        REPORTER_ASSERT(r, got);
        if (test.want) {
            REPORTER_ASSERT(r, SkColorSpace::Equals(got.get(), test.want.get()));
        }
    }
}

DEF_TEST(AdobeRGB, r) {
    if (sk_sp<SkData> data = GetResourceAsData("icc_profiles/AdobeRGB1998.icc")) {
        auto parsed = SkColorSpace::MakeICC(data->data(), data->size());
        auto manual = SkColorSpace::MakeRGB(g2Dot2_TransferFn, SkColorSpace::kAdobeRGB_Gamut);
        REPORTER_ASSERT(r, SkColorSpace::Equals(parsed.get(), manual.get()));
    }
}
