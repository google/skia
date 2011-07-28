
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkGeometry.h"

static bool nearly_equal(const SkPoint& a, const SkPoint& b) {
    return SkScalarNearlyEqual(a.fX, b.fX) && SkScalarNearlyEqual(a.fY, b.fY);
}

static void TestGeometry(skiatest::Reporter* reporter) {
    SkPoint pts[3], dst[5];

    pts[0].set(0, 0);
    pts[1].set(100, 50);
    pts[2].set(0, 100);

    int count = SkChopQuadAtMaxCurvature(pts, dst);
    REPORTER_ASSERT(reporter, count == 1 || count == 2);

    pts[0].set(0, 0);
    pts[1].set(SkIntToScalar(3), 0);
    pts[2].set(SkIntToScalar(3), SkIntToScalar(3));
    SkConvertQuadToCubic(pts, dst);
    const SkPoint cubic[] = {
        { 0, 0, },
        { SkIntToScalar(2), 0, },
        { SkIntToScalar(3), SkIntToScalar(1), },
        { SkIntToScalar(3), SkIntToScalar(3) },
    };
    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, nearly_equal(cubic[i], dst[i]));
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Geometry", GeometryTestClass, TestGeometry)
