/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/private/SkFloatingPoint.h"
#include "include/utils/SkRandom.h"
#include "tests/Test.h"

static void test_roundtoint(skiatest::Reporter* reporter) {
    SkScalar x = 0.49999997f;
    int ix = SkScalarRoundToInt(x);
    // We "should" get 0, since x < 0.5, but we don't due to float addition rounding up the low
    // bit after adding 0.5.
    REPORTER_ASSERT(reporter, 1 == ix);

    // This version explicitly performs the +0.5 step using double, which should avoid losing the
    // low bits.
    ix = SkDScalarRoundToInt(x);
    REPORTER_ASSERT(reporter, 0 == ix);
}

struct PointSet {
    const SkPoint* fPts;
    size_t         fCount;
    bool           fIsFinite;
};

static void test_isRectFinite(skiatest::Reporter* reporter) {
    static const SkPoint gF0[] = {
        { 0, 0 }, { 1, 1 }
    };
    static const SkPoint gF1[] = {
        { 0, 0 }, { 1, 1 }, { 99.234f, -42342 }
    };

    static const SkPoint gI0[] = {
        { 0, 0 }, { 1, 1 }, { 99.234f, -42342 }, { SK_ScalarNaN, 3 }, { 2, 3 },
    };
    static const SkPoint gI1[] = {
        { 0, 0 }, { 1, 1 }, { 99.234f, -42342 }, { 3, SK_ScalarNaN }, { 2, 3 },
    };
    static const SkPoint gI2[] = {
        { 0, 0 }, { 1, 1 }, { 99.234f, -42342 }, { SK_ScalarInfinity, 3 }, { 2, 3 },
    };
    static const SkPoint gI3[] = {
        { 0, 0 }, { 1, 1 }, { 99.234f, -42342 }, { 3, SK_ScalarInfinity }, { 2, 3 },
    };

    static const struct {
        const SkPoint* fPts;
        int            fCount;
        bool           fIsFinite;
    } gSets[] = {
        { gF0, SK_ARRAY_COUNT(gF0), true },
        { gF1, SK_ARRAY_COUNT(gF1), true },

        { gI0, SK_ARRAY_COUNT(gI0), false },
        { gI1, SK_ARRAY_COUNT(gI1), false },
        { gI2, SK_ARRAY_COUNT(gI2), false },
        { gI3, SK_ARRAY_COUNT(gI3), false },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gSets); ++i) {
        SkRect r;
        r.set(gSets[i].fPts, gSets[i].fCount);
        bool rectIsFinite = !r.isEmpty();
        REPORTER_ASSERT(reporter, gSets[i].fIsFinite == rectIsFinite);
    }
}

static bool isFinite_int(float x) {
    uint32_t bits = SkFloat2Bits(x);    // need unsigned for our shifts
    int exponent = bits << 1 >> 24;
    return exponent != 0xFF;
}

static bool isFinite_float(float x) {
    return SkToBool(sk_float_isfinite(x));
}

static bool isFinite_mulzero(float x) {
    float y = x * 0;
    return y == y;
}

// return true if the float is finite
typedef bool (*IsFiniteProc1)(float);

static bool isFinite2_and(float x, float y, IsFiniteProc1 proc) {
    return proc(x) && proc(y);
}

static bool isFinite2_mulzeroadd(float x, float y, IsFiniteProc1 proc) {
    return proc(x * 0 + y * 0);
}

// return true if both floats are finite
typedef bool (*IsFiniteProc2)(float, float, IsFiniteProc1);

enum FloatClass {
    kFinite,
    kInfinite,
    kNaN
};

static void test_floatclass(skiatest::Reporter* reporter, float value, FloatClass fc) {
    // our sk_float_is... function may return int instead of bool,
    // hence the double ! to turn it into a bool
    REPORTER_ASSERT(reporter, !!sk_float_isfinite(value) == (fc == kFinite));
    REPORTER_ASSERT(reporter, !!sk_float_isinf(value) == (fc == kInfinite));
    REPORTER_ASSERT(reporter, !!sk_float_isnan(value) == (fc == kNaN));
}

#if defined _WIN32
#pragma warning ( push )
// we are intentionally causing an overflow here
//      (warning C4756: overflow in constant arithmetic)
#pragma warning ( disable : 4756 )
#endif

static void test_isfinite(skiatest::Reporter* reporter) {
    struct Rec {
        float   fValue;
        bool    fIsFinite;
    };

    float max = 3.402823466e+38f;
    float inf = max * max;
    float nan = inf * 0;

    test_floatclass(reporter,    0, kFinite);
    test_floatclass(reporter,  max, kFinite);
    test_floatclass(reporter, -max, kFinite);
    test_floatclass(reporter,  inf, kInfinite);
    test_floatclass(reporter, -inf, kInfinite);
    test_floatclass(reporter,  nan, kNaN);
    test_floatclass(reporter, -nan, kNaN);

    const Rec data[] = {
        {   0,           true    },
        {   1,           true    },
        {  -1,           true    },
        {  max * 0.75f,  true    },
        {  max,          true    },
        {  -max * 0.75f, true    },
        {  -max,         true    },
        {  inf,          false   },
        { -inf,          false   },
        {  nan,          false   },
    };

    const IsFiniteProc1 gProc1[] = {
        isFinite_int,
        isFinite_float,
        isFinite_mulzero
    };
    const IsFiniteProc2 gProc2[] = {
        isFinite2_and,
        isFinite2_mulzeroadd
    };

    size_t i, n = SK_ARRAY_COUNT(data);

    for (i = 0; i < n; ++i) {
        for (size_t k = 0; k < SK_ARRAY_COUNT(gProc1); ++k) {
            const Rec& rec = data[i];
            bool finite = gProc1[k](rec.fValue);
            REPORTER_ASSERT(reporter, rec.fIsFinite == finite);
        }
    }

    for (i = 0; i < n; ++i) {
        const Rec& rec0 = data[i];
        for (size_t j = 0; j < n; ++j) {
            const Rec& rec1 = data[j];
            for (size_t k = 0; k < SK_ARRAY_COUNT(gProc1); ++k) {
                IsFiniteProc1 proc1 = gProc1[k];

                for (size_t m = 0; m < SK_ARRAY_COUNT(gProc2); ++m) {
                    bool finite = gProc2[m](rec0.fValue, rec1.fValue, proc1);
                    bool finite2 = rec0.fIsFinite && rec1.fIsFinite;
                    REPORTER_ASSERT(reporter, finite2 == finite);
                }
            }
        }
    }

    test_isRectFinite(reporter);
}

#if defined _WIN32
#pragma warning ( pop )
#endif

DEF_TEST(Scalar, reporter) {
    test_isfinite(reporter);
    test_roundtoint(reporter);
}
