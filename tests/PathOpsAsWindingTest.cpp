/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsThreadedCommon.h"
#include "tests/Test.h"

static SkPath build_squircle(SkPath::Verb verb, const SkRect& rect, SkPath::Direction dir) {
    SkPath path;
    bool reverse = SkPath::kCCW_Direction == dir;
    switch (verb) {
        case SkPath::kLine_Verb:
            path.addRect(rect, dir);
            reverse = false;
            break;
        case SkPath::kQuad_Verb:
            path.moveTo(rect.centerX(), rect.fTop);
            path.quadTo(rect.fRight, rect.fTop, rect.fRight, rect.centerY());
            path.quadTo(rect.fRight, rect.fBottom, rect.centerX(), rect.fBottom);
            path.quadTo(rect.fLeft, rect.fBottom, rect.fLeft, rect.centerY());
            path.quadTo(rect.fLeft, rect.fTop, rect.centerX(), rect.fTop);
            break;
        case SkPath::kConic_Verb:
            path.addCircle(rect.centerX(), rect.centerY(), rect.width() / 2, dir);
            reverse = false;
            break;
        case SkPath::kCubic_Verb: {
            SkScalar aX14 = rect.fLeft + rect.width() * 1 / 4;
            SkScalar aX34 = rect.fLeft + rect.width() * 3 / 4;
            SkScalar aY14 = rect.fTop + rect.height() * 1 / 4;
            SkScalar aY34 = rect.fTop + rect.height() * 3 / 4;
            path.moveTo(rect.centerX(), rect.fTop);
            path.cubicTo(aX34, rect.fTop, rect.fRight, aY14, rect.fRight, rect.centerY());
            path.cubicTo(rect.fRight, aY34, aX34, rect.fBottom, rect.centerX(), rect.fBottom);
            path.cubicTo(aX14, rect.fBottom, rect.fLeft, aY34, rect.fLeft, rect.centerY());
            path.cubicTo(rect.fLeft, aY14, aX14, rect.fTop, rect.centerX(), rect.fTop);
            } break;
        default:
            SkASSERT(0);
    }
    if (reverse) {
        SkPath temp;
        temp.reverseAddPath(path);
        path.swap(temp);
    }
    return path;
}

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
    // test that result may be input
    SkPath copy = test;
    test.setFillType(SkPath::kEvenOdd_FillType);
    REPORTER_ASSERT(reporter, AsWinding(test, &test));
    REPORTER_ASSERT(reporter, !test.isConvex());
    REPORTER_ASSERT(reporter, test == copy);
    // test a in b, b in a, cw/ccw
    constexpr SkRect rectA = {0, 0, 3, 3};
    constexpr SkRect rectB = {1, 1, 2, 2};
    const std::initializer_list<SkPoint> revBccw = {{1, 2}, {2, 2}, {2, 1}, {1, 1}};
    const std::initializer_list<SkPoint> revBcw  = {{2, 1}, {2, 2}, {1, 2}, {1, 1}};
    for (bool aFirst : {false, true}) {
        for (auto dirA : {SkPath::kCW_Direction, SkPath::kCCW_Direction}) {
            for (auto dirB : {SkPath::kCW_Direction, SkPath::kCCW_Direction}) {
                test.reset();
                test.setFillType(SkPath::kEvenOdd_FillType);
                if (aFirst) {
                    test.addRect(rectA, dirA);
                    test.addRect(rectB, dirB);
                } else {
                    test.addRect(rectB, dirB);
                    test.addRect(rectA, dirA);
                }
                SkPath original = test;
                REPORTER_ASSERT(reporter, AsWinding(test, &result));
                REPORTER_ASSERT(reporter, result.getFillType() == SkPath::kWinding_FillType);
                test.reset();
                if (aFirst) {
                    test.addRect(rectA, dirA);
                }
                if (dirA != dirB) {
                    test.addRect(rectB, dirB);
                } else {
                    test.addPoly(SkPath::kCW_Direction == dirA ? revBccw : revBcw, true);
                }
                if (!aFirst) {
                    test.addRect(rectA, dirA);
                }
                REPORTER_ASSERT(reporter, test == result);
                // test that result may be input
                REPORTER_ASSERT(reporter, AsWinding(original, &original));
                REPORTER_ASSERT(reporter, original.getFillType() == SkPath::kWinding_FillType);
                REPORTER_ASSERT(reporter, original == result);
            }
        }
    }
    // Test curve types with donuts. Create a donut with outer and hole in all directions.
    // After converting to winding, all donuts should have a hole in the middle.
    for (bool aFirst : {false, true}) {
        for (auto dirA : {SkPath::kCW_Direction, SkPath::kCCW_Direction}) {
            for (auto dirB : {SkPath::kCW_Direction, SkPath::kCCW_Direction}) {
                for (auto curveA : { SkPath::kLine_Verb, SkPath::kQuad_Verb,
                                     SkPath::kConic_Verb, SkPath::kCubic_Verb } ) {
                    SkPath pathA = build_squircle(curveA, rectA, dirA);
                    for (auto curveB : { SkPath::kLine_Verb, SkPath::kQuad_Verb,
                                     SkPath::kConic_Verb, SkPath::kCubic_Verb } ) {
                        test = aFirst ? pathA : SkPath();
                        test.addPath(build_squircle(curveB, rectB, dirB));
                        if (!aFirst) {
                            test.addPath(pathA);
                        }
                        test.setFillType(SkPath::kEvenOdd_FillType);
                        REPORTER_ASSERT(reporter, AsWinding(test, &result));
                       REPORTER_ASSERT(reporter, result.getFillType() == SkPath::kWinding_FillType);
                        for (SkScalar x = rectA.fLeft - 1; x <= rectA.fRight + 1; ++x) {
                            for (SkScalar y = rectA.fTop - 1; y <= rectA.fBottom + 1; ++y) {
                                bool evenOddContains = test.contains(x, y);
                                bool windingContains = result.contains(x, y);
                                REPORTER_ASSERT(reporter, evenOddContains == windingContains);
                            }
                        }
                    }
                }
            }
        }
    }
}
