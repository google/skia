/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkColorSpace.h"
#include "SkColorSpacePriv.h"
#include "SkData.h"
#include "SkICC.h"
#include "SkMatrix44.h"
#include "SkStream.h"
#include "Test.h"

static bool almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.001f;
}

static inline void test_to_xyz_d50(skiatest::Reporter* r, SkICC* icc, bool shouldSucceed,
                                   const float* reference) {
    SkMatrix44 result(SkMatrix44::kUninitialized_Constructor);
    REPORTER_ASSERT(r, shouldSucceed == icc->toXYZD50(&result));
    if (shouldSucceed) {
        float resultVals[16];
        result.asColMajorf(resultVals);
        for (int i = 0; i < 16; i++) {
            REPORTER_ASSERT(r, almost_equal(resultVals[i], reference[i]));
        }
    }
}

DEF_TEST(ICC_ToXYZD50, r) {
    const float z30Reference[16] = {
        0.59825f, 0.27103f, 0.00603f, 0.0f, 0.22243f, 0.67447f, 0.07368f, 0.0f, 0.14352f, 0.05449f,
        0.74519f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    sk_sp<SkData> data = SkData::MakeFromFileName(
            GetResourcePath("icc_profiles/HP_ZR30w.icc").c_str());
    sk_sp<SkICC> z30 = SkICC::Make(data->data(), data->size());
    test_to_xyz_d50(r, z30.get(), true, z30Reference);

    const float z32Reference[16] = {
        0.61583f, 0.28789f, 0.00513f, 0.0f, 0.20428f, 0.66972f, 0.06609f, 0.0f, 0.14409f, 0.04237f,
        0.75368f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    data = SkData::MakeFromFileName( GetResourcePath("icc_profiles/HP_Z32x.icc").c_str());
    sk_sp<SkICC> z32 = SkICC::Make(data->data(), data->size());
    test_to_xyz_d50(r, z32.get(), true, z32Reference);

    data = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperLeft.icc").c_str());
    sk_sp<SkICC> upperLeft = SkICC::Make(data->data(), data->size());
    test_to_xyz_d50(r, upperLeft.get(), false, z32Reference);

    data = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperRight.icc").c_str());
    sk_sp<SkICC> upperRight = SkICC::Make(data->data(), data->size());
    test_to_xyz_d50(r, upperRight.get(), false, z32Reference);
}

static inline void test_is_numerical_transfer_fn(skiatest::Reporter* r, SkICC* icc,
                                                 bool shouldSucceed,
                                                 const SkColorSpaceTransferFn& reference) {
    SkColorSpaceTransferFn result;
    REPORTER_ASSERT(r, shouldSucceed == icc->isNumericalTransferFn(&result));
    if (shouldSucceed) {
        REPORTER_ASSERT(r, 0 == memcmp(&result, &reference, sizeof(SkColorSpaceTransferFn)));
    }
}

DEF_TEST(ICC_IsNumericalTransferFn, r) {
    SkColorSpaceTransferFn referenceFn;
    referenceFn.fA = 1.0f;
    referenceFn.fB = 0.0f;
    referenceFn.fC = 0.0f;
    referenceFn.fD = 0.0f;
    referenceFn.fE = 0.0f;
    referenceFn.fF = 0.0f;
    referenceFn.fG = 2.2f;

    sk_sp<SkData> data = SkData::MakeFromFileName(
            GetResourcePath("icc_profiles/HP_ZR30w.icc").c_str());
    sk_sp<SkICC> z30 = SkICC::Make(data->data(), data->size());
    test_is_numerical_transfer_fn(r, z30.get(), true, referenceFn);

    data = SkData::MakeFromFileName( GetResourcePath("icc_profiles/HP_Z32x.icc").c_str());
    sk_sp<SkICC> z32 = SkICC::Make(data->data(), data->size());
    test_is_numerical_transfer_fn(r, z32.get(), true, referenceFn);

    data = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperLeft.icc").c_str());
    sk_sp<SkICC> upperLeft = SkICC::Make(data->data(), data->size());
    test_is_numerical_transfer_fn(r, upperLeft.get(), false, referenceFn);

    data = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperRight.icc").c_str());
    sk_sp<SkICC> upperRight = SkICC::Make(data->data(), data->size());
    test_is_numerical_transfer_fn(r, upperRight.get(), false, referenceFn);
}

static inline void test_write_icc(skiatest::Reporter* r, const SkColorSpaceTransferFn& fn,
                                  const SkMatrix44& toXYZD50, SkColorSpace* reference,
                                  bool writeToFile) {
    sk_sp<SkData> profile = SkICC::WriteToICC(fn, toXYZD50);
    if (writeToFile) {
        SkFILEWStream stream("out.icc");
        stream.write(profile->data(), profile->size());
    }

    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeICC(profile->data(), profile->size());
    REPORTER_ASSERT(r, SkColorSpace::Equals(reference, colorSpace.get()));
}

DEF_TEST(ICC_WriteICC, r) {
    SkColorSpaceTransferFn adobeFn;
    adobeFn.fA = 1.0f;
    adobeFn.fB = 0.0f;
    adobeFn.fC = 0.0f;
    adobeFn.fD = 0.0f;
    adobeFn.fE = 0.0f;
    adobeFn.fF = 0.0f;
    adobeFn.fG = 2.2f;
    SkMatrix44 adobeMatrix(SkMatrix44::kUninitialized_Constructor);
    adobeMatrix.set3x3RowMajorf(gAdobeRGB_toXYZD50);
    test_write_icc(r, adobeFn, adobeMatrix,
                   SkColorSpace::MakeNamed(SkColorSpace::kAdobeRGB_Named).get(), false);

    SkColorSpaceTransferFn srgbFn;
    srgbFn.fA = 1.0f / 1.055f;
    srgbFn.fB = 0.055f / 1.055f;
    srgbFn.fC = 1.0f / 12.92f;
    srgbFn.fD = 0.04045f;
    srgbFn.fE = 0.0f;
    srgbFn.fF = 0.0f;
    srgbFn.fG = 2.4f;
    SkMatrix44 srgbMatrix(SkMatrix44::kUninitialized_Constructor);
    srgbMatrix.set3x3RowMajorf(gSRGB_toXYZD50);
    test_write_icc(r, srgbFn, srgbMatrix, SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named).get(),
                   false);
}
