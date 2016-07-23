/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Unit tests for src/core/SkPoint3.cpp and its header

#include "SkPoint3.h"
#include "Test.h"

static void test_eq_ops(skiatest::Reporter* reporter) {
    const SkPoint3 p0 = SkPoint3::Make(0, 0, 0);
    const SkPoint3 p1 = SkPoint3::Make(1, 1, 1);
    const SkPoint3 p2 = SkPoint3::Make(1, 1, 1);

    REPORTER_ASSERT(reporter, p0 != p1);
    REPORTER_ASSERT(reporter, p1 == p2);
}

static void test_ops(skiatest::Reporter* reporter) {
    SkPoint3 v = SkPoint3::Make(1, 1, 1);
    v.normalize();
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(v.length(), SK_Scalar1));

    // scale
    SkPoint3 p = v.makeScale(3.0f);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(p.length(), 3.0f));

    p.scale(1.0f/3.0f);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(p.length(), SK_Scalar1));

    SkPoint3 p1 = SkPoint3::Make(20.0f, 2.0f, 10.0f);
    SkPoint3 p2 = -p1;

    // -
    p = p1 - p1;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(p.x(), 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(p.y(), 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(p.z(), 0.0f));

    // +
    p = p1 + p2;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(p.x(), 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(p.y(), 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(p.z(), 0.0f));
}

static void test_dot(skiatest::Reporter* reporter) {
    const SkPoint3 xAxis = SkPoint3::Make(1.0f, 0.0f, 0.0f);
    const SkPoint3 yAxis = SkPoint3::Make(0.0f, 1.0f, 0.0f);
    const SkPoint3 zAxis = SkPoint3::Make(0.0f, 0.0f, 1.0f);

    SkScalar dot = xAxis.dot(yAxis);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dot, 0.0f));

    dot = yAxis.dot(zAxis);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dot, 0.0f));

    dot = zAxis.dot(xAxis);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dot, 0.0f));

    SkPoint3 v = SkPoint3::Make(13.0f, 2.0f, 7.0f);
    v.normalize();

    dot = v.dot(v);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dot, 1.0f));

    v = SkPoint3::Make(SK_ScalarRoot2Over2, SK_ScalarRoot2Over2, 0.0f);

    dot = xAxis.dot(v);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dot, SK_ScalarRoot2Over2));

    dot = yAxis.dot(v);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dot, SK_ScalarRoot2Over2));
}

static void test_length(skiatest::Reporter* reporter,
                        SkScalar x, SkScalar y, SkScalar z, SkScalar expectedLen) {
    SkPoint3 point = SkPoint3::Make(x, y, z);

    SkScalar s1 = point.length();
    SkScalar s2 = SkPoint3::Length(x, y, z);

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(s1, s2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(s1, expectedLen));
}

static void test_normalize(skiatest::Reporter* reporter,
                           SkScalar x, SkScalar y, SkScalar z, SkScalar expectedLen) {
    SkPoint3 point = SkPoint3::Make(x, y, z);

    bool result = point.normalize();
    SkScalar newLength = point.length();

    if (0 == expectedLen) {
        const SkPoint3 empty = SkPoint3::Make(0.0f, 0.0f, 0.0f);

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(newLength, 0));
        REPORTER_ASSERT(reporter, !result);
        REPORTER_ASSERT(reporter, point == empty);
    } else {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(newLength, SK_Scalar1));
        REPORTER_ASSERT(reporter, result);
    }
}

DEF_TEST(Point3, reporter) {
    test_eq_ops(reporter);
    test_ops(reporter);
    test_dot(reporter);

    static const struct {
        SkScalar fX;
        SkScalar fY;
        SkScalar fZ;
        SkScalar fLength;
    } gRec[] = {
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 0.3f, 0.4f, 0.5f, SK_ScalarRoot2Over2 },
        { 1.0e-37f, 1.0e-37f, 1.0e-37f, 0.0f },  // underflows
        { 3.4e38f, 0.0f, 0.0f, 3.4e38f }         // overflows
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        test_length(reporter, gRec[i].fX, gRec[i].fY, gRec[i].fZ, gRec[i].fLength);
        test_normalize(reporter, gRec[i].fX, gRec[i].fY, gRec[i].fZ, gRec[i].fLength);
    }
}
