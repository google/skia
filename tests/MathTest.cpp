/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkEndian.h"
#include "src/base/SkHalf.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkRandom.h"
#include "tests/Test.h"

#include <array>
#include <cinttypes>
#include <cmath>
#include <cstddef>
#include <cstdint>

static void test_clz(skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, 32 == SkCLZ(0));
    REPORTER_ASSERT(reporter, 31 == SkCLZ(1));
    REPORTER_ASSERT(reporter, 1 == SkCLZ(1 << 30));
    REPORTER_ASSERT(reporter, 1 == SkCLZ((1 << 30) | (1 << 24) | 1));
    REPORTER_ASSERT(reporter, 0 == SkCLZ(~0U));

    SkRandom rand;
    for (int i = 0; i < 1000; ++i) {
        uint32_t mask = rand.nextU();
        // need to get some zeros for testing, but in some obscure way so the
        // compiler won't "see" that, and work-around calling the functions.
        mask >>= (mask & 31);
        int intri = SkCLZ(mask);
        int porta = SkCLZ_portable(mask);
        REPORTER_ASSERT(reporter, intri == porta, "mask:%u intri:%d porta:%d", mask, intri, porta);
    }
}

static void test_ctz(skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, 32 == SkCTZ(0));
    REPORTER_ASSERT(reporter, 0 == SkCTZ(1));
    REPORTER_ASSERT(reporter, 30 == SkCTZ(1 << 30));
    REPORTER_ASSERT(reporter, 2 == SkCTZ((1 << 30) | (1 << 24) | (1 << 2)));
    REPORTER_ASSERT(reporter, 0 == SkCTZ(~0U));

    SkRandom rand;
    for (int i = 0; i < 1000; ++i) {
        uint32_t mask = rand.nextU();
        // need to get some zeros for testing, but in some obscure way so the
        // compiler won't "see" that, and work-around calling the functions.
        mask >>= (mask & 31);
        int intri = SkCTZ(mask);
        int porta = SkCTZ_portable(mask);
        REPORTER_ASSERT(reporter, intri == porta, "mask:%u intri:%d porta:%d", mask, intri, porta);
    }
}

///////////////////////////////////////////////////////////////////////////////

static float sk_fsel(float pred, float result_ge, float result_lt) {
    return pred >= 0 ? result_ge : result_lt;
}

static float fast_floor(float x) {
//    float big = sk_fsel(x, 0x1.0p+23, -0x1.0p+23);
    float big = sk_fsel(x, (float)(1 << 23), -(float)(1 << 23));
    return (float)(x + big) - big;
}

static float std_floor(float x) {
    return std::floor(x);
}

static void test_floor_value(skiatest::Reporter* reporter, float value) {
    float fast = fast_floor(value);
    float std = std_floor(value);
    if (std != fast) {
        ERRORF(reporter, "fast_floor(%.9g) == %.9g != %.9g == std_floor(%.9g)",
               value, fast, std, value);
    }
}

static void test_floor(skiatest::Reporter* reporter) {
    static const float gVals[] = {
        0, 1, 1.1f, 1.01f, 1.001f, 1.0001f, 1.00001f, 1.000001f, 1.0000001f
    };

    for (size_t i = 0; i < std::size(gVals); ++i) {
        test_floor_value(reporter, gVals[i]);
//        test_floor_value(reporter, -gVals[i]);
    }
}

///////////////////////////////////////////////////////////////////////////////

static float float_blend(int src, int dst, float unit) {
    return dst + (src - dst) * unit;
}

static int blend31(int src, int dst, int a31) {
    return dst + ((src - dst) * a31 * 2114 >> 16);
    //    return dst + ((src - dst) * a31 * 33 >> 10);
}

static int blend31_slow(int src, int dst, int a31) {
    int prod = src * a31 + (31 - a31) * dst + 16;
    prod = (prod + (prod >> 5)) >> 5;
    return prod;
}

static int blend31_round(int src, int dst, int a31) {
    int prod = (src - dst) * a31 + 16;
    prod = (prod + (prod >> 5)) >> 5;
    return dst + prod;
}

static int blend31_old(int src, int dst, int a31) {
    a31 += a31 >> 4;
    return dst + ((src - dst) * a31 >> 5);
}

// suppress unused code warning
static int (*blend_functions[])(int, int, int) = {
    blend31,
    blend31_slow,
    blend31_round,
    blend31_old
};

static void test_blend31() {
    int failed = 0;
    int death = 0;
    if ((false)) { // avoid bit rot, suppress warning
        failed = (*blend_functions[0])(0,0,0);
    }
    for (int src = 0; src <= 255; src++) {
        for (int dst = 0; dst <= 255; dst++) {
            for (int a = 0; a <= 31; a++) {
//                int r0 = blend31(src, dst, a);
//                int r0 = blend31_round(src, dst, a);
//                int r0 = blend31_old(src, dst, a);
                int r0 = blend31_slow(src, dst, a);

                float f = float_blend(src, dst, a / 31.f);
                int r1 = (int)f;
                int r2 = SkScalarRoundToInt(f);

                if (r0 != r1 && r0 != r2) {
                    SkDebugf("src:%d dst:%d a:%d result:%d float:%g\n",
                                 src,   dst, a,        r0,      f);
                    failed += 1;
                }
                if (r0 > 255) {
                    death += 1;
                    SkDebugf("death src:%d dst:%d a:%d result:%d float:%g\n",
                                        src,   dst, a,        r0,      f);
                }
            }
        }
    }
    SkDebugf("---- failed %d death %d\n", failed, death);
}

static void check_length(skiatest::Reporter* reporter,
                         const SkPoint& p, SkScalar targetLen) {
    float x = p.fX;
    float y = p.fY;
    float len = std::sqrt(x*x + y*y);

    len /= targetLen;

    REPORTER_ASSERT(reporter, len > 0.999f && len < 1.001f);
}

template <typename T>
static void unittest_isfinite(skiatest::Reporter* reporter) {
    const T zero = T(0);
    const T plain = T(123);
    const T inf = std::numeric_limits<T>::infinity();
    const T big = std::numeric_limits<T>::max();
    const T nan = inf * zero;

    REPORTER_ASSERT(reporter, !SkIsNaN(inf));
    REPORTER_ASSERT(reporter, !SkIsNaN(-inf));
    REPORTER_ASSERT(reporter, !SkIsFinite(inf));
    REPORTER_ASSERT(reporter, !SkIsFinite(-inf));

    REPORTER_ASSERT(reporter,  SkIsNaN(nan));
    REPORTER_ASSERT(reporter, !SkIsNaN(big));
    REPORTER_ASSERT(reporter, !SkIsNaN(-big));
    REPORTER_ASSERT(reporter, !SkIsNaN(zero));

    REPORTER_ASSERT(reporter, !SkIsFinite(nan));
    REPORTER_ASSERT(reporter,  SkIsFinite(big));
    REPORTER_ASSERT(reporter,  SkIsFinite(-big));
    REPORTER_ASSERT(reporter,  SkIsFinite(zero));

    // SkIsFinite supports testing multiple values at once.
    REPORTER_ASSERT(reporter, !SkIsFinite(inf, plain));
    REPORTER_ASSERT(reporter, !SkIsFinite(plain, -inf));
    REPORTER_ASSERT(reporter, !SkIsFinite(nan, plain));
    REPORTER_ASSERT(reporter,  SkIsFinite(plain, big));
    REPORTER_ASSERT(reporter,  SkIsFinite(-big, plain));
    REPORTER_ASSERT(reporter,  SkIsFinite(plain, zero));

    REPORTER_ASSERT(reporter, !SkIsFinite(inf, plain, plain));
    REPORTER_ASSERT(reporter, !SkIsFinite(plain, -inf, plain));
    REPORTER_ASSERT(reporter, !SkIsFinite(plain, plain, nan));
    REPORTER_ASSERT(reporter,  SkIsFinite(big, plain, plain));
    REPORTER_ASSERT(reporter,  SkIsFinite(plain, -big, plain));
    REPORTER_ASSERT(reporter,  SkIsFinite(plain, plain, zero));
}

static void unittest_half(skiatest::Reporter* reporter) {
    static const float gFloats[] = {
        0.f, 1.f, 0.5f, 0.499999f, 0.5000001f, 1.f/3,
        -0.f, -1.f, -0.5f, -0.499999f, -0.5000001f, -1.f/3
    };

    for (size_t i = 0; i < std::size(gFloats); ++i) {
        SkHalf h = SkFloatToHalf(gFloats[i]);
        float f = SkHalfToFloat(h);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(f, gFloats[i]));
    }

    // check some special values
    union FloatUnion {
        uint32_t fU;
        float    fF;
    };

    static const FloatUnion largestPositiveHalf = { ((142 << 23) | (1023 << 13)) };
    SkHalf h = SkFloatToHalf(largestPositiveHalf.fF);
    float f = SkHalfToFloat(h);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(f, largestPositiveHalf.fF));

    static const FloatUnion largestNegativeHalf = { (1u << 31) | (142u << 23) | (1023u << 13) };
    h = SkFloatToHalf(largestNegativeHalf.fF);
    f = SkHalfToFloat(h);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(f, largestNegativeHalf.fF));

    static const FloatUnion smallestPositiveHalf = { 102 << 23 };
    h = SkFloatToHalf(smallestPositiveHalf.fF);
    f = SkHalfToFloat(h);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(f, smallestPositiveHalf.fF));

    static const FloatUnion overflowHalf = { ((143 << 23) | (1023 << 13)) };
    h = SkFloatToHalf(overflowHalf.fF);
    f = SkHalfToFloat(h);
    REPORTER_ASSERT(reporter, !SkIsFinite(f) );

    static const FloatUnion underflowHalf = { 101 << 23 };
    h = SkFloatToHalf(underflowHalf.fF);
    f = SkHalfToFloat(h);
    REPORTER_ASSERT(reporter, f == 0.0f );

    static const FloatUnion inf32 = { 255 << 23 };
    h = SkFloatToHalf(inf32.fF);
    f = SkHalfToFloat(h);
    REPORTER_ASSERT(reporter, !SkIsFinite(f) );

    static const FloatUnion nan32 = { 255 << 23 | 1 };
    h = SkFloatToHalf(nan32.fF);
    f = SkHalfToFloat(h);
    REPORTER_ASSERT(reporter, SkIsNaN(f) );

}

template <typename RSqrtFn>
static void test_rsqrt(skiatest::Reporter* reporter, RSqrtFn rsqrt) {
    const float maxRelativeError = 6.50196699e-4f;

    // test close to 0 up to 1
    float input = 0.000001f;
    for (int i = 0; i < 1000; ++i) {
        float exact = 1.0f/std::sqrt(input);
        float estimate = rsqrt(input);
        float relativeError = std::fabs(exact - estimate)/exact;
        REPORTER_ASSERT(reporter, relativeError <= maxRelativeError);
        input += 0.001f;
    }

    // test 1 to ~100
    input = 1.0f;
    for (int i = 0; i < 1000; ++i) {
        float exact = 1.0f/std::sqrt(input);
        float estimate = rsqrt(input);
        float relativeError = std::fabs(exact - estimate)/exact;
        REPORTER_ASSERT(reporter, relativeError <= maxRelativeError);
        input += 0.01f;
    }

    // test some big numbers
    input = 1000000.0f;
    for (int i = 0; i < 100; ++i) {
        float exact = 1.0f/std::sqrt(input);
        float estimate = rsqrt(input);
        float relativeError = std::fabs(exact - estimate)/exact;
        REPORTER_ASSERT(reporter, relativeError <= maxRelativeError);
        input += 754326.f;
    }
}

static void test_muldiv255(skiatest::Reporter* reporter) {
    for (int a = 0; a <= 255; a++) {
        for (int b = 0; b <= 255; b++) {
            int ab = a * b;
            float s = ab / 255.0f;
            int round = (int)floorf(s + 0.5f);
            int trunc = (int)floorf(s);

            int iround = SkMulDiv255Round(a, b);
            int itrunc = SkMulDiv255Trunc(a, b);

            REPORTER_ASSERT(reporter, iround == round);
            REPORTER_ASSERT(reporter, itrunc == trunc);

            REPORTER_ASSERT(reporter, itrunc <= iround);
            REPORTER_ASSERT(reporter, iround <= a);
            REPORTER_ASSERT(reporter, iround <= b);
        }
    }
}

static void test_muldiv255ceiling(skiatest::Reporter* reporter) {
    for (int c = 0; c <= 255; c++) {
        for (int a = 0; a <= 255; a++) {
            int product = (c * a + 255);
            int expected_ceiling = (product + (product >> 8)) >> 8;
            int webkit_ceiling = (c * a + 254) / 255;
            REPORTER_ASSERT(reporter, expected_ceiling == webkit_ceiling);
            int skia_ceiling = SkMulDiv255Ceiling(c, a);
            REPORTER_ASSERT(reporter, skia_ceiling == webkit_ceiling);
        }
    }
}

static void test_copysign(skiatest::Reporter* reporter) {
    static const int32_t gTriples[] = {
        // x, y, expected result
        0, 0, 0,
        0, 1, 0,
        0, -1, 0,
        1, 0, 1,
        1, 1, 1,
        1, -1, -1,
        -1, 0, 1,
        -1, 1, 1,
        -1, -1, -1,
    };
    for (size_t i = 0; i < std::size(gTriples); i += 3) {
        REPORTER_ASSERT(reporter,
                        SkCopySign32(gTriples[i], gTriples[i+1]) == gTriples[i+2]);
        float x = (float)gTriples[i];
        float y = (float)gTriples[i+1];
        float expected = (float)gTriples[i+2];
        REPORTER_ASSERT(reporter, std::copysign(x, y) == expected);
    }

    SkRandom rand;
    for (int j = 0; j < 1000; j++) {
        int ix = rand.nextS();
        REPORTER_ASSERT(reporter, SkCopySign32(ix, ix) == ix);
        REPORTER_ASSERT(reporter, SkCopySign32(ix, -ix) == -ix);
        REPORTER_ASSERT(reporter, SkCopySign32(-ix, ix) == ix);
        REPORTER_ASSERT(reporter, SkCopySign32(-ix, -ix) == -ix);

        SkScalar sx = rand.nextSScalar1();
        REPORTER_ASSERT(reporter, SkScalarCopySign(sx, sx) == sx);
        REPORTER_ASSERT(reporter, SkScalarCopySign(sx, -sx) == -sx);
        REPORTER_ASSERT(reporter, SkScalarCopySign(-sx, sx) == sx);
        REPORTER_ASSERT(reporter, SkScalarCopySign(-sx, -sx) == -sx);
    }
}

static void huge_vector_normalize(skiatest::Reporter* reporter) {
    // these values should fail (overflow/underflow) trying to normalize
    const SkVector fail[] = {
        { 0, 0 },
        { SK_ScalarInfinity, 0 }, { 0, SK_ScalarInfinity },
        { 0, SK_ScalarNaN }, { SK_ScalarNaN, 0 },
    };
    for (SkVector v : fail) {
        SkVector v2 = v;
        if (v2.setLength(1.0f)) {
            REPORTER_ASSERT(reporter, !v.setLength(1.0f));
        }
    }
}

DEF_TEST(PopCount, reporter) {
    {
        uint32_t testVal = 0;
        REPORTER_ASSERT(reporter, SkPopCount(testVal) == 0);
    }

    for (int i = 0; i < 32; ++i) {
        uint32_t testVal = 0x1 << i;
        REPORTER_ASSERT(reporter, SkPopCount(testVal) == 1);

        testVal ^= 0xFFFFFFFF;
        REPORTER_ASSERT(reporter, SkPopCount(testVal) == 31);
    }

    {
        uint32_t testVal = 0xFFFFFFFF;
        REPORTER_ASSERT(reporter, SkPopCount(testVal) == 32);
    }

    SkRandom rand;
    for (int i = 0; i < 100; ++i) {
        int expectedNumSetBits = 0;
        uint32_t testVal = 0;

        int numTries = rand.nextULessThan(33);
        for (int j = 0; j < numTries; ++j) {
            int bit = rand.nextRangeU(0, 31);

            if (testVal & (0x1 << bit)) {
                continue;
            }

            ++expectedNumSetBits;
            testVal |= 0x1 << bit;
        }

        REPORTER_ASSERT(reporter, SkPopCount(testVal) == expectedNumSetBits);
    }
}

DEF_TEST(NthSet, reporter) {
    {
        uint32_t testVal = 0x1;
        uint32_t recreated = 0;
        int result = SkNthSet(testVal, 0);
        recreated |= (0x1 << result);
        REPORTER_ASSERT(reporter, testVal == recreated);
    }

    {
        uint32_t testVal = 0x80000000;
        uint32_t recreated = 0;
        int result = SkNthSet(testVal, 0);
        recreated |= (0x1 << result);
        REPORTER_ASSERT(reporter, testVal == recreated);
    }

    {
        uint32_t testVal = 0x55555555;
        uint32_t recreated = 0;
        for (int i = 0; i < 16; ++i) {
            int result = SkNthSet(testVal, i);
            REPORTER_ASSERT(reporter, result == 2*i);
            recreated |= (0x1 << result);
        }
        REPORTER_ASSERT(reporter, testVal == recreated);
    }

    SkRandom rand;
    for (int i = 0; i < 100; ++i) {
        int expectedNumSetBits = 0;
        uint32_t testVal = 0;

        int numTries = rand.nextULessThan(33);
        for (int j = 0; j < numTries; ++j) {
            int bit = rand.nextRangeU(0, 31);

            if (testVal & (0x1 << bit)) {
                continue;
            }

            ++expectedNumSetBits;
            testVal |= 0x1 << bit;
        }

        REPORTER_ASSERT(reporter, SkPopCount(testVal) == expectedNumSetBits);
        uint32_t recreated = 0;

        for (int j = 0; j < expectedNumSetBits; ++j) {
            int index = SkNthSet(testVal, j);
            recreated |= (0x1 << index);
        }

        REPORTER_ASSERT(reporter, recreated == testVal);
    }
}

DEF_TEST(Math, reporter) {
    int         i;
    SkRandom    rand;

    // these should assert
#if 0
    SkToS8(128);
    SkToS8(-129);
    SkToU8(256);
    SkToU8(-5);

    SkToS16(32768);
    SkToS16(-32769);
    SkToU16(65536);
    SkToU16(-5);

    if (sizeof(size_t) > 4) {
        SkToS32(4*1024*1024);
        SkToS32(-4*1024*1024);
        SkToU32(5*1024*1024);
        SkToU32(-5);
    }
#endif

    test_muldiv255(reporter);
    test_muldiv255ceiling(reporter);
    test_copysign(reporter);

    {
        SkScalar x = SK_ScalarNaN;
        REPORTER_ASSERT(reporter, SkIsNaN(x));
    }

    for (i = 0; i < 10000; i++) {
        SkPoint p;

        // These random values are being treated as 32-bit-patterns, not as
        // ints; calling SkIntToScalar() here produces crashes.
        p.setLength((SkScalar) rand.nextS(),
                    (SkScalar) rand.nextS(),
                    SK_Scalar1);
        check_length(reporter, p, SK_Scalar1);
        p.setLength((SkScalar) (rand.nextS() >> 13),
                    (SkScalar) (rand.nextS() >> 13),
                    SK_Scalar1);
        check_length(reporter, p, SK_Scalar1);
    }

    {
        SkFixed result = SkFixedDiv(100, 100);
        REPORTER_ASSERT(reporter, result == SK_Fixed1);
        result = SkFixedDiv(1, SK_Fixed1);
        REPORTER_ASSERT(reporter, result == 1);
        result = SkFixedDiv(10 - 1, SK_Fixed1 * 3);
        REPORTER_ASSERT(reporter, result == 3);
    }

    {
        REPORTER_ASSERT(reporter, (SkFixedRoundToFixed(-SK_Fixed1 * 10) >> 1) == -SK_Fixed1 * 5);
        REPORTER_ASSERT(reporter, (SkFixedFloorToFixed(-SK_Fixed1 * 10) >> 1) == -SK_Fixed1 * 5);
        REPORTER_ASSERT(reporter, (SkFixedCeilToFixed(-SK_Fixed1 * 10) >> 1) == -SK_Fixed1 * 5);
    }

    huge_vector_normalize(reporter);
    unittest_isfinite<float>(reporter);
    unittest_isfinite<double>(reporter);
    unittest_half(reporter);
    test_rsqrt(reporter, sk_float_rsqrt);
    test_rsqrt(reporter, sk_float_rsqrt_portable);

    for (i = 0; i < 10000; i++) {
        SkFixed numer = rand.nextS();
        SkFixed denom = rand.nextS();
        SkFixed result = SkFixedDiv(numer, denom);
        int64_t check = SkLeftShift((int64_t)numer, 16) / denom;

        (void)SkCLZ(numer);
        (void)SkCLZ(denom);

        REPORTER_ASSERT(reporter, result != (SkFixed)SK_NaN32);
        if (check > SK_MaxS32) {
            check = SK_MaxS32;
        } else if (check < -SK_MaxS32) {
            check = SK_MinS32;
        }
        if (result != (int32_t)check) {
            ERRORF(reporter, "\nFixed Divide: %8x / %8x -> %8x %8" PRIx64 "\n",
                   (uint32_t)numer, (uint32_t)denom, (uint32_t)result, (uint64_t)check);
        }
        REPORTER_ASSERT(reporter, result == (int32_t)check);
    }

    if ((false)) test_floor(reporter);

    // disable for now
    if ((false)) test_blend31();  // avoid bit rot, suppress warning

    test_clz(reporter);
    test_ctz(reporter);
}

template <typename T> struct PairRec {
    T   fYin;
    T   fYang;
};

DEF_TEST(TestEndian, reporter) {
    static const PairRec<uint16_t> g16[] = {
        { 0x0,      0x0     },
        { 0xFFFF,   0xFFFF  },
        { 0x1122,   0x2211  },
    };
    static const PairRec<uint32_t> g32[] = {
        { 0x0,          0x0         },
        { 0xFFFFFFFF,   0xFFFFFFFF  },
        { 0x11223344,   0x44332211  },
    };
    static const PairRec<uint64_t> g64[] = {
        { 0x0,      0x0                             },
        { 0xFFFFFFFFFFFFFFFFULL,  0xFFFFFFFFFFFFFFFFULL  },
        { 0x1122334455667788ULL,  0x8877665544332211ULL  },
    };

    REPORTER_ASSERT(reporter, 0x1122 == SkTEndianSwap16<0x2211>::value);
    REPORTER_ASSERT(reporter, 0x11223344 == SkTEndianSwap32<0x44332211>::value);
    REPORTER_ASSERT(reporter, 0x1122334455667788ULL == SkTEndianSwap64<0x8877665544332211ULL>::value);

    for (size_t i = 0; i < std::size(g16); ++i) {
        REPORTER_ASSERT(reporter, g16[i].fYang == SkEndianSwap16(g16[i].fYin));
    }
    for (size_t i = 0; i < std::size(g32); ++i) {
        REPORTER_ASSERT(reporter, g32[i].fYang == SkEndianSwap32(g32[i].fYin));
    }
    for (size_t i = 0; i < std::size(g64); ++i) {
        REPORTER_ASSERT(reporter, g64[i].fYang == SkEndianSwap64(g64[i].fYin));
    }
}

template <typename T>
static void test_divmod(skiatest::Reporter* r) {
#if !defined(__MSVC_RUNTIME_CHECKS)
    const struct {
        T numer;
        T denom;
    } kEdgeCases[] = {
        {(T)17, (T)17},
        {(T)17, (T)4},
        {(T)0,  (T)17},
        // For unsigned T these negatives are just some large numbers.  Doesn't hurt to test them.
        {(T)-17, (T)-17},
        {(T)-17, (T)4},
        {(T)17,  (T)-4},
        {(T)-17, (T)-4},
    };

    for (size_t i = 0; i < std::size(kEdgeCases); i++) {
        const T numer = kEdgeCases[i].numer;
        const T denom = kEdgeCases[i].denom;
        T div, mod;
        SkTDivMod(numer, denom, &div, &mod);
        REPORTER_ASSERT(r, numer/denom == div);
        REPORTER_ASSERT(r, numer%denom == mod);
    }

    SkRandom rand;
    for (size_t i = 0; i < 10000; i++) {
        const T numer = (T)rand.nextS();
        T denom = 0;
        while (0 == denom) {
            denom = (T)rand.nextS();
        }
        T div, mod;
        SkTDivMod(numer, denom, &div, &mod);
        REPORTER_ASSERT(r, numer/denom == div);
        REPORTER_ASSERT(r, numer%denom == mod);
    }
#endif
}

DEF_TEST(divmod_u8, r) {
    test_divmod<uint8_t>(r);
}

DEF_TEST(divmod_u16, r) {
    test_divmod<uint16_t>(r);
}

DEF_TEST(divmod_u32, r) {
    test_divmod<uint32_t>(r);
}

DEF_TEST(divmod_u64, r) {
    test_divmod<uint64_t>(r);
}

DEF_TEST(divmod_s8, r) {
    test_divmod<int8_t>(r);
}

DEF_TEST(divmod_s16, r) {
    test_divmod<int16_t>(r);
}

DEF_TEST(divmod_s32, r) {
    test_divmod<int32_t>(r);
}

DEF_TEST(divmod_s64, r) {
    test_divmod<int64_t>(r);
}

static void test_nextsizepow2(skiatest::Reporter* r, size_t test, size_t expectedAns) {
    size_t ans = GrNextSizePow2(test);

    REPORTER_ASSERT(r, ans == expectedAns);
    //SkDebugf("0x%zx -> 0x%zx (0x%zx)\n", test, ans, expectedAns);
}

DEF_TEST(GrNextSizePow2, reporter) {
    constexpr int kNumSizeTBits = 8 * sizeof(size_t);

    size_t test = 0, expectedAns = 1;

    test_nextsizepow2(reporter, test, expectedAns);

    test = 1; expectedAns = 1;

    for (int i = 1; i < kNumSizeTBits; ++i) {
        test_nextsizepow2(reporter, test, expectedAns);

        test++;
        expectedAns <<= 1;

        test_nextsizepow2(reporter, test, expectedAns);

        test = expectedAns;
    }

    // For the remaining three tests there is no higher power (of 2)
    test = 0x1;
    test <<= kNumSizeTBits-1;
    test_nextsizepow2(reporter, test, test);

    test++;
    test_nextsizepow2(reporter, test, test);

    test_nextsizepow2(reporter, SIZE_MAX, SIZE_MAX);
}

DEF_TEST(FloatSaturate32, reporter) {
    const struct {
        float   fFloat;
        int     fExpectedInt;
    } recs[] = {
        { 0, 0 },
        { 100.5f, 100 },
        { (float)SK_MaxS32, SK_MaxS32FitsInFloat },
        { (float)SK_MinS32, SK_MinS32FitsInFloat },
        { SK_MaxS32 * 100.0f, SK_MaxS32FitsInFloat },
        { SK_MinS32 * 100.0f, SK_MinS32FitsInFloat },
        { SK_ScalarInfinity, SK_MaxS32FitsInFloat },
        { SK_ScalarNegativeInfinity, SK_MinS32FitsInFloat },
        { SK_ScalarNaN, SK_MaxS32FitsInFloat },
    };

    for (auto r : recs) {
        int i = sk_float_saturate2int(r.fFloat);
        REPORTER_ASSERT(reporter, r.fExpectedInt == i);

        // Ensure that SkTPin bounds even non-finite values (including NaN)
        SkScalar p = SkTPin<SkScalar>(r.fFloat, 0, 100);
        REPORTER_ASSERT(reporter, p >= 0 && p <= 100);
    }
}

DEF_TEST(FloatSaturate64, reporter) {
    const struct {
        float   fFloat;
        int64_t fExpected64;
    } recs[] = {
        { 0, 0 },
        { 100.5f, 100 },
        { (float)SK_MaxS64, SK_MaxS64FitsInFloat },
        { (float)SK_MinS64, SK_MinS64FitsInFloat },
        { SK_MaxS64 * 100.0f, SK_MaxS64FitsInFloat },
        { SK_MinS64 * 100.0f, SK_MinS64FitsInFloat },
        { SK_ScalarInfinity, SK_MaxS64FitsInFloat },
        { SK_ScalarNegativeInfinity, SK_MinS64FitsInFloat },
        { SK_ScalarNaN, SK_MaxS64FitsInFloat },
    };

    for (auto r : recs) {
        int64_t i = sk_float_saturate2int64(r.fFloat);
        REPORTER_ASSERT(reporter, r.fExpected64 == i);
    }
}

DEF_TEST(DoubleSaturate32, reporter) {
    const struct {
        double  fDouble;
        int     fExpectedInt;
    } recs[] = {
        { 0, 0 },
        { 100.5, 100 },
        { SK_MaxS32, SK_MaxS32 },
        { SK_MinS32, SK_MinS32 },
        { SK_MaxS32 - 1, SK_MaxS32 - 1 },
        { SK_MinS32 + 1, SK_MinS32 + 1 },
        { SK_MaxS32 * 100.0, SK_MaxS32 },
        { SK_MinS32 * 100.0, SK_MinS32 },
        { SK_ScalarInfinity, SK_MaxS32 },
        { SK_ScalarNegativeInfinity, SK_MinS32 },
        { SK_ScalarNaN, SK_MaxS32 },
    };

    for (auto r : recs) {
        int i = sk_double_saturate2int(r.fDouble);
        REPORTER_ASSERT(reporter, r.fExpectedInt == i);
    }
}

#if defined(__ARM_NEON)
    #include <arm_neon.h>

    DEF_TEST(NeonU16Div255, r) {

        for (int v = 0; v <= 255*255; v++) {
            int want = (v + 127)/255;

            uint16x8_t V = vdupq_n_u16(v);
            int got = vrshrq_n_u16(vrsraq_n_u16(V, V, 8), 8)[0];

            if (got != want) {
                SkDebugf("%d -> %d, want %d\n", v, got, want);
            }
            REPORTER_ASSERT(r, got == want);
        }
    }

#endif
