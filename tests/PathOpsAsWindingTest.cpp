/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"
#include "Test.h"

DEF_TEST(PathOpsAsWinding, reporter) {
    SkPath test, result;
    test.addRect({1, 2, 3, 4});
    // if test is winding
    REPORTER_ASSERT(reporter, AsWinding(test, &result));
    REPORTER_ASSERT(reporter, test == result);
    // if test is empty
    test.reset();
    test.setFillType(SkPath::kEvenOdd_FillType);
    REPORTER_ASSERT(reporter, AsWinding(test, &result));
    REPORTER_ASSERT(reporter, result.isEmpty());
    REPORTER_ASSERT(reporter, result.getFillType() == SkPath::kWinding_FillType);
    // if test is convex
    test.addCircle(5, 5, 10);
    REPORTER_ASSERT(reporter, AsWinding(test, &result));
    REPORTER_ASSERT(reporter, result.isConvex());
    test.setFillType(SkPath::kWinding_FillType);
    REPORTER_ASSERT(reporter, test == result);
    // if test has infinity
    test.reset();
    test.addRect({1, 2, 3, SK_ScalarInfinity});
    test.setFillType(SkPath::kEvenOdd_FillType);
    REPORTER_ASSERT(reporter, !AsWinding(test, &result));
    // if test has only one contour
    test.reset();
    SkPoint ell[] = {{0, 0}, {4, 0}, {4, 1}, {1, 1}, {1, 4}, {0, 4}};
    test.addPoly(ell, SK_ARRAY_COUNT(ell), true);
    test.setFillType(SkPath::kEvenOdd_FillType);
    REPORTER_ASSERT(reporter, AsWinding(test, &result));
    REPORTER_ASSERT(reporter, !result.isConvex());
    test.setFillType(SkPath::kWinding_FillType);
    REPORTER_ASSERT(reporter, test == result);
    // test two contours that do not overlap or share bounds
    test.addRect({5, 2, 6, 3});
    test.setFillType(SkPath::kEvenOdd_FillType);
    REPORTER_ASSERT(reporter, AsWinding(test, &result));
    REPORTER_ASSERT(reporter, !result.isConvex());
    test.setFillType(SkPath::kWinding_FillType);
    REPORTER_ASSERT(reporter, test == result);
    // test two contours that do not overlap but share bounds
    test.reset();
    test.addPoly(ell, SK_ARRAY_COUNT(ell), true);
    test.addRect({2, 2, 3, 3});
    test.setFillType(SkPath::kEvenOdd_FillType);
    REPORTER_ASSERT(reporter, AsWinding(test, &result));
    REPORTER_ASSERT(reporter, !result.isConvex());
    test.setFillType(SkPath::kWinding_FillType);
    REPORTER_ASSERT(reporter, test == result);
    // test two contours that partially overlap
    test.reset();
    test.addRect({0, 0, 3, 3});
    test.addRect({1, 1, 4, 4});
    test.setFillType(SkPath::kEvenOdd_FillType);
    REPORTER_ASSERT(reporter, AsWinding(test, &result));
    REPORTER_ASSERT(reporter, !result.isConvex());
    test.setFillType(SkPath::kWinding_FillType);
    REPORTER_ASSERT(reporter, test == result);
    // test a in b, b in a, cw/ccw
    constexpr SkRect rectA = {0, 0, 3, 3};
    constexpr SkRect rectB = {1, 1, 2, 2};
    for (bool useA : {false, true}) {
        for (auto aDir : {SkPath::kCW_Direction, SkPath::kCCW_Direction}) {
            for (auto bDir : {SkPath::kCW_Direction, SkPath::kCCW_Direction}) {
                test.reset();
                test.setFillType(SkPath::kEvenOdd_FillType);
                if (useA) {
                    test.addRect(rectA, aDir);
                    test.addRect(rectB, bDir);
                } else {
                    test.addRect(rectB, bDir);
                    test.addRect(rectA, aDir);
                }
                REPORTER_ASSERT(reporter, AsWinding(test, &result));
                REPORTER_ASSERT(reporter, result.getFillType() == SkPath::kWinding_FillType);
                test.reset();
                if (useA) {
                    test.addRect(rectA, aDir);
                    test.addRect(rectB, SkPath::kCW_Direction == aDir ?
                            SkPath::kCCW_Direction : SkPath::kCW_Direction);
                } else {
                    test.addRect(rectB, SkPath::kCW_Direction == aDir ?
                            SkPath::kCCW_Direction : SkPath::kCW_Direction);
                    test.addRect(rectA, aDir);
                }
                REPORTER_ASSERT(reporter, test == result);
                if (test != result) {
                    SkDebugf("");
                }
            }
        }
    }
}

