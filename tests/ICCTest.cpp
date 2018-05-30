/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "Resources.h"
#include "SkColorSpacePriv.h"
#include "SkICC.h"
#include "SkString.h"
#include "Test.h"
#include "../third_party/skcms/skcms.h"

DEF_TEST(WriteICCProfile, r) {
    auto adobeRGB = SkColorSpace::MakeRGB(g2Dot2_TransferFn, SkColorSpace::kAdobeRGB_Gamut);

    struct {
        SkColorSpaceTransferFn fn;
        const float*           toXYZD50;
        const char*            desc;
        sk_sp<SkColorSpace>    want;
    } tests[] = {
        {g2Dot2_TransferFn, gAdobeRGB_toXYZD50, "AdobeRGB", adobeRGB},
        { gSRGB_TransferFn,     gSRGB_toXYZD50,     "sRGB", SkColorSpace::MakeSRGB()},
    };

    for (auto test : tests) {
        sk_sp<SkData> profile = SkWriteICCProfile(test.fn, test.toXYZD50);
        REPORTER_ASSERT(r, profile);

        skcms_ICCProfile parsed;
        REPORTER_ASSERT(r, skcms_Parse(profile->data(), profile->size(), &parsed));

        sk_sp<SkColorSpace> got = SkColorSpace::Make(parsed);
        REPORTER_ASSERT(r, got);
        REPORTER_ASSERT(r, SkColorSpace::Equals(got.get(), test.want.get()));

        skcms_ICCTag desc;
        REPORTER_ASSERT(r, skcms_GetTagBySignature(&parsed,
                                                   SkSetFourByteTag('d','e','s','c'),
                                                   &desc));

        // Rather than really carefully break down the 'desc' tag,
        // just check our expected description is somewhere in there (as big-endian UTF-16).
        uint8_t big_endian_utf16[16];
        for (size_t i = 0; i < strlen(test.desc); i++) {
            big_endian_utf16[2*i+0] = 0;
            big_endian_utf16[2*i+1] = test.desc[i];
        }

        SkString haystack((const char*)desc.buf, desc.size),
                 needle  ((const char*)big_endian_utf16, 2*strlen(test.desc));
        REPORTER_ASSERT(r, haystack.contains(needle.c_str()));
    }
}

DEF_TEST(AdobeRGB, r) {
    if (sk_sp<SkData> profile = GetResourceAsData("icc_profiles/AdobeRGB1998.icc")) {
        skcms_ICCProfile parsed;
        REPORTER_ASSERT(r, skcms_Parse(profile->data(), profile->size(), &parsed));

        auto got  = SkColorSpace::Make(parsed);
        auto want = SkColorSpace::MakeRGB(g2Dot2_TransferFn, SkColorSpace::kAdobeRGB_Gamut);
        REPORTER_ASSERT(r, SkColorSpace::Equals(got.get(), want.get()));
    }
}
