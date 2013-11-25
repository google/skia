
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
// Unit tests for src/core/SkPoint.cpp and its header

#include "SkPoint.h"
#include "SkRect.h"
#include "Test.h"

static void test_casts(skiatest::Reporter* reporter) {
    SkPoint p = { 0, 0 };
    SkRect  r = { 0, 0, 0, 0 };

    const SkScalar* pPtr = SkTCast<const SkScalar*>(&p);
    const SkScalar* rPtr = SkTCast<const SkScalar*>(&r);

    REPORTER_ASSERT(reporter, p.asScalars() == pPtr);
    REPORTER_ASSERT(reporter, r.asScalars() == rPtr);
}

// Tests SkPoint::Normalize() for this (x,y)
static void test_Normalize(skiatest::Reporter* reporter,
                           SkScalar x, SkScalar y) {
    SkPoint point;
    point.set(x, y);
    SkScalar oldLength = point.length();
    SkScalar returned = SkPoint::Normalize(&point);
    SkScalar newLength = point.length();
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(returned, oldLength));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(newLength, SK_Scalar1));
}

// Tests that SkPoint::length() and SkPoint::Length() both return
// approximately expectedLength for this (x,y).
static void test_length(skiatest::Reporter* reporter, SkScalar x, SkScalar y,
                        SkScalar expectedLength) {
    SkPoint point;
    point.set(x, y);
    SkScalar s1 = point.length();
    SkScalar s2 = SkPoint::Length(x, y);
    //The following should be exactly the same, but need not be.
    //See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(s1, s2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(s1, expectedLength));

    test_Normalize(reporter, x, y);
}

// Ugh. Windows compiler can dive into other .cpp files, and sometimes
// notices that I will generate an overflow... which is exactly the point
// of this test!
//
// To avoid this warning, I need to convince the compiler that I might not
// use that big value, hence this hacky helper function: reporter is
// ALWAYS non-null. (shhhhhh, don't tell the compiler that).
template <typename T> T get_value(skiatest::Reporter* reporter, T value) {
    return reporter ? value : 0;
}

// On linux gcc, 32bit, we are seeing the compiler propagate up the value
// of SkPoint::length() as a double (which we use sometimes to avoid overflow
// during the computation), even though the signature says float (SkScalar).
//
// force_as_float is meant to capture our latest technique (horrible as
// it is) to force the value to be a float, so we can test whether it was
// finite or not.
static float force_as_float(skiatest::Reporter* reporter, float value) {
    uint32_t storage;
    memcpy(&storage, &value, 4);
    // even the pair of memcpy calls are not sufficient, since those seem to
    // be no-op'd, so we add a runtime tests (just like get_value) to force
    // the compiler to give us an actual float.
    if (NULL == reporter) {
        storage = ~storage;
    }
    memcpy(&value, &storage, 4);
    return value;
}

// test that we handle very large values correctly. i.e. that we can
// successfully normalize something whose mag overflows a float.
static void test_overflow(skiatest::Reporter* reporter) {
    SkScalar bigFloat = get_value(reporter, 3.4e38f);
    SkPoint pt = { bigFloat, bigFloat };

    SkScalar length = pt.length();
    length = force_as_float(reporter, length);

    // expect this to be non-finite, but dump the results if not.
    if (SkScalarIsFinite(length)) {
        SkDebugf("length(%g, %g) == %g\n", pt.fX, pt.fY, length);
        REPORTER_ASSERT(reporter, !SkScalarIsFinite(length));
    }

    // this should succeed, even though we can't represent length
    REPORTER_ASSERT(reporter, pt.setLength(SK_Scalar1));

    // now that pt is normalized, we check its length
    length = pt.length();
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(length, SK_Scalar1));
}

// test that we handle very small values correctly. i.e. that we can
// report failure if we try to normalize them.
static void test_underflow(skiatest::Reporter* reporter) {
    SkPoint pt = { 1.0e-37f, 1.0e-37f };
    SkPoint copy = pt;

    REPORTER_ASSERT(reporter, 0 == SkPoint::Normalize(&pt));
    REPORTER_ASSERT(reporter, pt == copy);  // pt is unchanged

    REPORTER_ASSERT(reporter, !pt.setLength(SK_Scalar1));
    REPORTER_ASSERT(reporter, pt == copy);  // pt is unchanged
}

#include "TestClassDef.h"
DEF_TEST(Point, reporter) {
    test_casts(reporter);

    static const struct {
        SkScalar fX;
        SkScalar fY;
        SkScalar fLength;
    } gRec[] = {
        { SkIntToScalar(3), SkIntToScalar(4), SkIntToScalar(5) },
        { 0.6f, 0.8f, SK_Scalar1 },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        test_length(reporter, gRec[i].fX, gRec[i].fY, gRec[i].fLength);
    }

    test_underflow(reporter);
    test_overflow(reporter);
}

DEF_TEST(Point_setLengthFast, reporter) {
    // Scale a (1,1) point to a bunch of different lengths,
    // making sure the slow and fast paths are within 0.1%.
    const float tests[] = { 1.0f, 0.0f, 1.0e-37f, 3.4e38f, 42.0f, 0.00012f };

    const SkPoint kOne = {1.0f, 1.0f};
    for (unsigned i = 0; i < SK_ARRAY_COUNT(tests); i++) {
        SkPoint slow = kOne, fast = kOne;

        slow.setLength(tests[i]);
        fast.setLengthFast(tests[i]);

        if (slow.length() < FLT_MIN && fast.length() < FLT_MIN) continue;

        SkScalar ratio = slow.length() / fast.length();
        REPORTER_ASSERT(reporter, ratio > 0.999f);
        REPORTER_ASSERT(reporter, ratio < 1.001f);
    }
}
