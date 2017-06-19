/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkColorSpace.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpace_XYZ.h"
#include "SkData.h"
#include "SkICC.h"
#include "SkICCPriv.h"
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
                   SkColorSpace_Base::MakeNamed(SkColorSpace_Base::kAdobeRGB_Named).get(), false);

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
    test_write_icc(r, srgbFn, srgbMatrix, SkColorSpace::MakeSRGB().get(),
                   false);

    SkString adobeTag = SkICCGetColorProfileTag(adobeFn, adobeMatrix);
    SkString srgbTag = SkICCGetColorProfileTag(srgbFn, srgbMatrix);
    REPORTER_ASSERT(r, adobeTag != srgbTag);
    REPORTER_ASSERT(r, srgbTag.equals("sRGB"));
    REPORTER_ASSERT(r, adobeTag.equals("AdobeRGB"));
}

static inline void test_raw_transfer_fn(skiatest::Reporter* r, SkICC* icc) {
    SkICC::Tables tables;
    bool result = icc->rawTransferFnData(&tables);
    REPORTER_ASSERT(r, result);

    REPORTER_ASSERT(r, 0.0f == tables.red()[0]);
    REPORTER_ASSERT(r, 0.0f == tables.green()[0]);
    REPORTER_ASSERT(r, 0.0f == tables.blue()[0]);
    REPORTER_ASSERT(r, 1.0f == tables.red()[tables.fRed.fCount - 1]);
    REPORTER_ASSERT(r, 1.0f == tables.green()[tables.fGreen.fCount - 1]);
    REPORTER_ASSERT(r, 1.0f == tables.blue()[tables.fBlue.fCount - 1]);
}

class ICCTest {
public:
    static sk_sp<SkICC> MakeICC(sk_sp<SkColorSpace> space) {
        return sk_sp<SkICC>(new SkICC(std::move(space)));
    }
    static sk_sp<SkICC> MakeICC(sk_sp<SkGammas> gammas) {
        return MakeICC(sk_sp<SkColorSpace>(new SkColorSpace_XYZ(
                kNonStandard_SkGammaNamed, std::move(gammas),
                SkMatrix44(SkMatrix44::kIdentity_Constructor), nullptr)));
    }
};

DEF_TEST(ICC_RawTransferFns, r) {
    sk_sp<SkICC> srgb = ICCTest::MakeICC(SkColorSpace::MakeSRGB());
    test_raw_transfer_fn(r, srgb.get());

    sk_sp<SkICC> adobe =
            ICCTest::MakeICC(SkColorSpace_Base::MakeNamed(SkColorSpace_Base::kAdobeRGB_Named));
    test_raw_transfer_fn(r, adobe.get());

    // Lookup-table based gamma curves
    constexpr size_t tableSize = 10;
    void* memory = sk_malloc_throw(sizeof(SkGammas) + sizeof(float) * tableSize);
    sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new (memory) SkGammas(3));
    for (int i = 0; i < 3; ++i) {
        gammas->fType[i] = SkGammas::Type::kTable_Type;
        gammas->fData[i].fTable.fSize = tableSize;
        gammas->fData[i].fTable.fOffset = 0;
    }

    float* table = SkTAddOffset<float>(memory, sizeof(SkGammas));
    table[0] = 0.00f;
    table[1] = 0.05f;
    table[2] = 0.10f;
    table[3] = 0.15f;
    table[4] = 0.25f;
    table[5] = 0.35f;
    table[6] = 0.45f;
    table[7] = 0.60f;
    table[8] = 0.75f;
    table[9] = 1.00f;
    sk_sp<SkICC> tbl = ICCTest::MakeICC(gammas);
    test_raw_transfer_fn(r, tbl.get());

    // Parametric gamma curves
    memory = sk_malloc_throw(sizeof(SkGammas) + sizeof(SkColorSpaceTransferFn));
    gammas = sk_sp<SkGammas>(new (memory) SkGammas(3));
    for (int i = 0; i < 3; ++i) {
        gammas->fType[i] = SkGammas::Type::kParam_Type;
        gammas->fData[i].fParamOffset = 0;
    }

    SkColorSpaceTransferFn* params = SkTAddOffset<SkColorSpaceTransferFn>
            (memory, sizeof(SkGammas));

    // Interval.
    params->fD = 0.04045f;

    // First equation:
    params->fC = 1.0f / 12.92f;
    params->fF = 0.0f;

    // Second equation:
    // Note that the function is continuous (it's actually sRGB).
    params->fA = 1.0f / 1.055f;
    params->fB = 0.055f / 1.055f;
    params->fE = 0.0f;
    params->fG = 2.4f;
    sk_sp<SkICC> param = ICCTest::MakeICC(gammas);
    test_raw_transfer_fn(r, param.get());

    // Exponential gamma curves
    gammas = sk_sp<SkGammas>(new SkGammas(3));
    for (int i = 0; i < 3; ++i) {
        gammas->fType[i] = SkGammas::Type::kValue_Type;
        gammas->fData[i].fValue = 1.4f;
    }
    sk_sp<SkICC> exp = ICCTest::MakeICC(gammas);
    test_raw_transfer_fn(r, exp.get());

    gammas = sk_sp<SkGammas>(new SkGammas(3));
    gammas->fType[0] = gammas->fType[1] = gammas->fType[2] = SkGammas::Type::kNamed_Type;
    gammas->fData[0].fNamed = kSRGB_SkGammaNamed;
    gammas->fData[1].fNamed = k2Dot2Curve_SkGammaNamed;
    gammas->fData[2].fNamed = kLinear_SkGammaNamed;
    sk_sp<SkICC> named = ICCTest::MakeICC(gammas);
    test_raw_transfer_fn(r, named.get());

    memory = sk_malloc_throw(sizeof(SkGammas) + sizeof(float) * tableSize +
                                   sizeof(SkColorSpaceTransferFn));
    gammas = sk_sp<SkGammas>(new (memory) SkGammas(3));

    table = SkTAddOffset<float>(memory, sizeof(SkGammas));
    table[0] = 0.00f;
    table[1] = 0.15f;
    table[2] = 0.20f;
    table[3] = 0.25f;
    table[4] = 0.35f;
    table[5] = 0.45f;
    table[6] = 0.55f;
    table[7] = 0.70f;
    table[8] = 0.85f;
    table[9] = 1.00f;

    params = SkTAddOffset<SkColorSpaceTransferFn>(memory,
            sizeof(SkGammas) + sizeof(float) * tableSize);
    params->fA = 1.0f / 1.055f;
    params->fB = 0.055f / 1.055f;
    params->fC = 1.0f / 12.92f;
    params->fD = 0.04045f;
    params->fE = 0.0f;
    params->fF = 0.0f;
    params->fG = 2.4f;

    gammas->fType[0] = SkGammas::Type::kValue_Type;
    gammas->fData[0].fValue = 1.2f;

    gammas->fType[1] = SkGammas::Type::kTable_Type;
    gammas->fData[1].fTable.fSize = tableSize;
    gammas->fData[1].fTable.fOffset = 0;

    gammas->fType[2] = SkGammas::Type::kParam_Type;
    gammas->fData[2].fParamOffset = sizeof(float) * tableSize;
    sk_sp<SkICC> nonstd = ICCTest::MakeICC(gammas);
    test_raw_transfer_fn(r, nonstd.get());

    // Reverse order of table and exponent
    gammas->fType[1] = SkGammas::Type::kValue_Type;
    gammas->fData[1].fValue = 1.2f;

    gammas->fType[0] = SkGammas::Type::kTable_Type;
    gammas->fData[0].fTable.fSize = tableSize;
    gammas->fData[0].fTable.fOffset = 0;
    sk_sp<SkICC> nonstd2 = ICCTest::MakeICC(gammas);
    test_raw_transfer_fn(r, nonstd2.get());
}
