/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageInfo.h"

#include "Test.h"

struct ImageInfoRec {
    int         fWidth;
    int         fHeight;
    SkColorType fColorType;
    SkAlphaType fAlphaType;
    float       fGamma;
    bool        fIsSRGB;
};

static void check_info(skiatest::Reporter* reporter,
                       const ImageInfoRec& expected, const SkImageInfo& info) {
    REPORTER_ASSERT(reporter, info.width() == expected.fWidth);
    REPORTER_ASSERT(reporter, info.height() == expected.fHeight);
    REPORTER_ASSERT(reporter, info.colorType() == expected.fColorType);
    REPORTER_ASSERT(reporter, info.alphaType() == expected.fAlphaType);
    REPORTER_ASSERT(reporter, info.gamma() == expected.fGamma);
    REPORTER_ASSERT(reporter, info.isSRGB() == expected.fIsSRGB);
}

DEF_TEST(ImageInfo, reporter) {
    const float nan = SK_ScalarNaN;
    const float nice_gamma = 1.5f;
    const int W = 100;
    const int H = 200;
    SkImageInfo info;
    
    const ImageInfoRec rec[] = {
        { 0, 0, kUnknown_SkColorType,   kIgnore_SkAlphaType,   0, false },  // MakeUnknown()
        { W, H, kUnknown_SkColorType,   kIgnore_SkAlphaType,   0, false },  // MakeUnknown(...)
        { W, H, kN32_SkColorType,       kPremul_SkAlphaType,   1, false },  // MakeN32Premul(...)
        { W, H, kN32_SkColorType,       kOpaque_SkAlphaType,   1, false },  // MakeN32(...)
        { W, H, kAlpha_8_SkColorType,   kPremul_SkAlphaType,   0, false },  // MakeA8()
        { W, H, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, 1, false },  // Make()
        { W, H, kBGRA_8888_SkColorType, kPremul_SkAlphaType,   1, false },  // Make()
        { W, H, kBGRA_8888_SkColorType, kPremul_SkAlphaType,   0, true },  // MakeSRGB()
        { W, H, kN32_SkColorType,       kPremul_SkAlphaType,   1, false },  // MakeWithGamma() NaN
        { W, H, kAlpha_8_SkColorType,   kPremul_SkAlphaType,   0, false },  // MakeWithGamma() bad ct for gamma
        { W, H, kN32_SkColorType,       kPremul_SkAlphaType,   nice_gamma, false },  // MakeWithGamma() good
    };

    check_info(reporter, rec[ 0], SkImageInfo::MakeUnknown());
    check_info(reporter, rec[ 1], SkImageInfo::MakeUnknown(W, H));
    check_info(reporter, rec[ 2], SkImageInfo::MakeN32Premul(W, H));
    check_info(reporter, rec[ 3], SkImageInfo::MakeN32(W, H, rec[3].fAlphaType));
    check_info(reporter, rec[ 4], SkImageInfo::MakeA8(W, H));
    check_info(reporter, rec[ 5], SkImageInfo::Make(W, H, rec[5].fColorType, rec[5].fAlphaType));
    check_info(reporter, rec[ 6], SkImageInfo::Make(W, H, rec[6].fColorType, rec[6].fAlphaType));
    check_info(reporter, rec[ 7], SkImageInfo::MakeSRGB(W, H, rec[7].fColorType, rec[7].fAlphaType));
    check_info(reporter, rec[ 8], SkImageInfo::MakeWithGamma(W, H, rec[8].fColorType, rec[8].fAlphaType, nan));
    check_info(reporter, rec[ 9], SkImageInfo::MakeWithGamma(W, H, rec[9].fColorType, rec[9].fAlphaType, nice_gamma));
    check_info(reporter, rec[10], SkImageInfo::MakeWithGamma(W, H, rec[10].fColorType, rec[10].fAlphaType, rec[10].fGamma));
}

