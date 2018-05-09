/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkColorSpacePriv.h"
#include "SkICC.h"
#include "SkString.h"
#include "Test.h"

DEF_TEST(WriteICCProfile, r) {
    auto adobeRGB = SkColorSpace::MakeRGB(g2Dot2_TransferFn, SkColorSpace::kAdobeRGB_Gamut);

    struct {
        SkColorSpaceTransferFn fn;
        const float*           toXYZD50;
        const char*            desc;
        sk_sp<SkColorSpace>    want;
    } tests[] = {
        {g2Dot2_TransferFn, gAdobeRGB_toXYZD50, "AdobeRGB", adobeRGB},
        { gSRGB_TransferFn,     gSRGB_toXYZD50, "sRGB",     SkColorSpace::MakeSRGB()},
    };

    for (auto test : tests) {
        sk_sp<SkData> profile = SkWriteICCProfile(test.fn, test.toXYZD50);
        REPORTER_ASSERT(r, profile);

        sk_sp<SkColorSpace> got = SkColorSpace::MakeICC(profile->data(), profile->size());
        REPORTER_ASSERT(r, got);
        REPORTER_ASSERT(r, SkColorSpace::Equals(got.get(), test.want.get()));

        // Rather than really carefully break down the 'desc' tag,
        // just check our expected description is somewhere in there (as big-endian UTF-16).
        uint8_t big_endian_utf16[16];
        for (size_t i = 0; i < strlen(test.desc); i++) {
            big_endian_utf16[2*i+0] = 0;
            big_endian_utf16[2*i+1] = test.desc[i];
        }

        SkString haystack((const char*)profile->data(),  profile->size()),
                 needle  ((const char*)big_endian_utf16, 2*strlen(test.desc));
        REPORTER_ASSERT(r, haystack.contains(needle.c_str()));
    }
}

DEF_TEST(AdobeRGB, r) {
    if (sk_sp<SkData> data = GetResourceAsData("icc_profiles/AdobeRGB1998.icc")) {
        auto parsed = SkColorSpace::MakeICC(data->data(), data->size());
        auto manual = SkColorSpace::MakeRGB(g2Dot2_TransferFn, SkColorSpace::kAdobeRGB_Gamut);
        REPORTER_ASSERT(r, SkColorSpace::Equals(parsed.get(), manual.get()));
    }
}
