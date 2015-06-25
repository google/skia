/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPMFloat.h"
#include "Test.h"

DEF_TEST(SkPMFloat, r) {
    // Test SkPMColor <-> SkPMFloat
    SkPMColor c = SkPreMultiplyColor(0xFFCC9933);
    SkPMFloat pmf(c);
    REPORTER_ASSERT(r, SkScalarNearlyEqual(255.0f, 255*pmf.a()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(204.0f, 255*pmf.r()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(153.0f, 255*pmf.g()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual( 51.0f, 255*pmf.b()));
    REPORTER_ASSERT(r, c == pmf.round());

    // Test rounding.
    pmf = SkPMFloat(254.5f/255, 203.5f/255, 153.1f/255, 50.8f/255);
    REPORTER_ASSERT(r, c == pmf.round());

    SkPMFloat clamped(SkPMFloat(510.0f/255, 153.0f/255, 1.0f/255, -0.2f/255).round());
    REPORTER_ASSERT(r, SkScalarNearlyEqual(255.0f, 255*clamped.a()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(153.0f, 255*clamped.r()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(  1.0f, 255*clamped.g()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(  0.0f, 255*clamped.b()));

    // Test SkPMFloat <-> Sk4f conversion.
    Sk4f fs = clamped;
    SkPMFloat scaled = fs * Sk4f(0.25f);
    REPORTER_ASSERT(r, SkScalarNearlyEqual(63.75f, 255*scaled.a()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual(38.25f, 255*scaled.r()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual( 0.25f, 255*scaled.g()));
    REPORTER_ASSERT(r, SkScalarNearlyEqual( 0.00f, 255*scaled.b()));
}
