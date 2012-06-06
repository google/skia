
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
// Unit tests for src/core/SkPoint.cpp and its header

#include "SkPoint.h"
#include "Test.h"

// Tests that SkPoint::length() and SkPoint::Length() both return
// approximately expectedLength for this (x,y).
static void test_length(skiatest::Reporter* reporter, SkScalar x, SkScalar y,
                        SkScalar expectedLength) {
    SkPoint point;
    point.set(x, y);
    SkScalar s1 = point.length();
    SkScalar s2 = SkPoint::Length(x, y);
    REPORTER_ASSERT(reporter, s1 == s2);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(s1, expectedLength));
}

// Tests SkPoint::Normalize() for this (x,y)
static void test_Normalize(skiatest::Reporter* reporter,
                           SkScalar x, SkScalar y) {
    SkPoint point;
    point.set(x, y);
    SkScalar oldLength = point.length();
    SkScalar returned = SkPoint::Normalize(&point);
    SkScalar newLength = point.length();
    REPORTER_ASSERT(reporter, returned == oldLength);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(newLength, SK_Scalar1));
}

static void PointTest(skiatest::Reporter* reporter) {
    test_length(reporter, SkIntToScalar(3), SkIntToScalar(4), SkIntToScalar(5));
    test_length(reporter, SkFloatToScalar(0.6f), SkFloatToScalar(0.8f),
                SK_Scalar1);
    test_Normalize(reporter, SkIntToScalar(3), SkIntToScalar(4));
    test_Normalize(reporter, SkFloatToScalar(0.6f), SkFloatToScalar(0.8f));
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Point", PointTestClass, PointTest)
