/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkPathPriv.h"

DEF_TEST(IsClosedSingleContourTest, reporter) {
    SkPath p;
    REPORTER_ASSERT(reporter, !SkPathPriv::IsClosedSingleContour(p));

    p.reset();
    p.close();
    REPORTER_ASSERT(reporter, !SkPathPriv::IsClosedSingleContour(p));

    p.reset();
    p.moveTo(10, 10);
    p.close();
    REPORTER_ASSERT(reporter, SkPathPriv::IsClosedSingleContour(p));

    p.reset();
    p.moveTo(10, 10);
    p.lineTo(20, 20);
    p.close();
    REPORTER_ASSERT(reporter, SkPathPriv::IsClosedSingleContour(p));

    p.reset();
    p.moveTo(10, 10);
    p.lineTo(20, 20);
    p.quadTo(30, 30, 40, 40);
    p.cubicTo(50, 50, 60, 60, 70, 70);
    p.conicTo(30, 30, 40, 40, 0.5);
    p.close();
    REPORTER_ASSERT(reporter, SkPathPriv::IsClosedSingleContour(p));

    p.reset();
    p.moveTo(10, 10);
    p.lineTo(20, 20);
    p.lineTo(20, 30);
    REPORTER_ASSERT(reporter, !SkPathPriv::IsClosedSingleContour(p));

    p.reset();
    p.moveTo(10, 10);
    p.lineTo(20, 20);
    p.moveTo(10, 10);
    p.lineTo(20, 30);
    p.close();
    REPORTER_ASSERT(reporter, !SkPathPriv::IsClosedSingleContour(p));

    p.reset();
    p.moveTo(10, 10);
    p.lineTo(20, 20);
    p.close();
    p.lineTo(20, 30);
    p.close();
    REPORTER_ASSERT(reporter, !SkPathPriv::IsClosedSingleContour(p));
}
