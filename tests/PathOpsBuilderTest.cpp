/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "Test.h"

DEF_TEST(PathOpsBuilder, reporter) {
    SkOpBuilder builder;
    SkPath result;
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isEmpty());

    builder.add(result, kDifference_PathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isEmpty());

    builder.add(result, kUnion_PathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isEmpty());

    SkPath rectPath;
    rectPath.addRect(0, 1, 2, 3, SkPath::kCW_Direction);
    builder.add(rectPath, kUnion_PathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    bool closed;
    SkPath::Direction dir;
    REPORTER_ASSERT(reporter, result.isRect(NULL, &closed, &dir));
    REPORTER_ASSERT(reporter, closed);
    REPORTER_ASSERT(reporter, dir == SkPath::kCW_Direction);
    REPORTER_ASSERT(reporter, rectPath == result);

    rectPath.reset();
    rectPath.addRect(0, 1, 2, 3, SkPath::kCCW_Direction);
    builder.add(rectPath, kUnion_PathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isRect(NULL, &closed, &dir));
    REPORTER_ASSERT(reporter, closed);
    REPORTER_ASSERT(reporter, dir == SkPath::kCCW_Direction);
    REPORTER_ASSERT(reporter, rectPath == result);

    builder.add(rectPath, kDifference_PathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isEmpty());

    SkPath rect2, rect3;
    rect2.addRect(2, 1, 4, 3, SkPath::kCW_Direction);
    rect3.addRect(4, 1, 5, 3, SkPath::kCCW_Direction);
    builder.add(rectPath, kUnion_PathOp);
    builder.add(rect2, kUnion_PathOp);
    builder.add(rect3, kUnion_PathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, result.isRect(NULL, &closed, &dir));
    REPORTER_ASSERT(reporter, closed);
    SkRect expected;
    expected.set(0, 1, 5, 3);
    REPORTER_ASSERT(reporter, result.getBounds() == expected);

    SkPath circle1, circle2, circle3;
    circle1.addCircle(5, 6, 4, SkPath::kCW_Direction);
    circle2.addCircle(7, 4, 8, SkPath::kCCW_Direction);
    circle3.addCircle(6, 5, 6, SkPath::kCW_Direction);
    SkPath opCompare;
    Op(circle1, circle2, kUnion_PathOp, &opCompare);
    Op(opCompare, circle3, kDifference_PathOp, &opCompare);
    builder.add(circle1, kUnion_PathOp);
    builder.add(circle2, kUnion_PathOp);
    builder.add(circle3, kDifference_PathOp);
    REPORTER_ASSERT(reporter, builder.resolve(&result));
    REPORTER_ASSERT(reporter, opCompare == result);
}
