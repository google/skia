/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkPathMeasure.h"

static void test_small_segment3() {
#ifdef SK_SCALAR_IS_FLOAT
    SkPath path;
    const SkPoint pts[] = {
        { 0, 0 },
        { 100000000000.0f, 100000000000.0f }, { 0, 0 }, { 10, 10 },
        { 10, 10 }, { 0, 0 }, { 10, 10 }
    };

    path.moveTo(pts[0]);
    for (size_t i = 1; i < SK_ARRAY_COUNT(pts); i += 3) {
        path.cubicTo(pts[i], pts[i + 1], pts[i + 2]);
    }

    SkPathMeasure meas(path, false);
    meas.getLength();
#endif
}

static void test_small_segment2() {
#ifdef SK_SCALAR_IS_FLOAT
    SkPath path;
    const SkPoint pts[] = {
        { 0, 0 },
        { 100000000000.0f, 100000000000.0f }, { 0, 0 },
        { 10, 10 }, { 0, 0 },
    };

    path.moveTo(pts[0]);
    for (size_t i = 1; i < SK_ARRAY_COUNT(pts); i += 2) {
        path.quadTo(pts[i], pts[i + 1]);
    }
    SkPathMeasure meas(path, false);
    meas.getLength();
#endif
}

static void test_small_segment() {
#ifdef SK_SCALAR_IS_FLOAT
    SkPath path;
    const SkPoint pts[] = {
        { 100000, 100000},
        // big jump between these points, makes a big segment
        { 1.0005f, 0.9999f },
        // tiny (non-zero) jump between these points
        { SK_Scalar1, SK_Scalar1 },
    };

    path.moveTo(pts[0]);
    for (size_t i = 1; i < SK_ARRAY_COUNT(pts); ++i) {
        path.lineTo(pts[i]);
    }
    SkPathMeasure meas(path, false);

    /*  this would assert (before a fix) because we added a segment with
        the same length as the prev segment, due to the follow (bad) pattern

        d = distance(pts[0], pts[1]);
        distance += d;
        seg->fDistance = distance;

        SkASSERT(d > 0);    // TRUE
        SkASSERT(seg->fDistance > prevSeg->fDistance);  // FALSE

        This 2nd assert failes because (distance += d) didn't affect distance
        because distance >>> d.
     */
    meas.getLength();
#endif
}

DEF_TEST(PathMeasure, reporter) {
    SkPath  path;

    path.moveTo(0, 0);
    path.lineTo(SK_Scalar1, 0);
    path.lineTo(SK_Scalar1, SK_Scalar1);
    path.lineTo(0, SK_Scalar1);

    SkPathMeasure   meas(path, true);
    SkScalar        length = meas.getLength();
    SkASSERT(length == SK_Scalar1*4);

    path.reset();
    path.moveTo(0, 0);
    path.lineTo(SK_Scalar1*3, SK_Scalar1*4);
    meas.setPath(&path, false);
    length = meas.getLength();
    REPORTER_ASSERT(reporter, length == SK_Scalar1*5);

    path.reset();
    path.addCircle(0, 0, SK_Scalar1);
    meas.setPath(&path, true);
    length = meas.getLength();
//    SkDebugf("circle arc-length = %g\n", length);

    // Test the behavior following a close not followed by a move.
    path.reset();
    path.lineTo(SK_Scalar1, 0);
    path.lineTo(SK_Scalar1, SK_Scalar1);
    path.lineTo(0, SK_Scalar1);
    path.close();
    path.lineTo(-SK_Scalar1, 0);
    meas.setPath(&path, false);
    length = meas.getLength();
    REPORTER_ASSERT(reporter, length == SK_Scalar1 * 4);
    meas.nextContour();
    length = meas.getLength();
    REPORTER_ASSERT(reporter, length == SK_Scalar1);
    SkPoint position;
    SkVector tangent;
    REPORTER_ASSERT(reporter, meas.getPosTan(SK_ScalarHalf, &position, &tangent));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fX,
                            -SK_ScalarHalf,
                            0.0001f));
    REPORTER_ASSERT(reporter, position.fY == 0);
    REPORTER_ASSERT(reporter, tangent.fX == -SK_Scalar1);
    REPORTER_ASSERT(reporter, tangent.fY == 0);

    // Test degenerate paths
    path.reset();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(SK_Scalar1, 0);
    path.quadTo(SK_Scalar1, 0, SK_Scalar1, 0);
    path.quadTo(SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1 * 2);
    path.cubicTo(SK_Scalar1, SK_Scalar1 * 2,
                 SK_Scalar1, SK_Scalar1 * 2,
                 SK_Scalar1, SK_Scalar1 * 2);
    path.cubicTo(SK_Scalar1*2, SK_Scalar1 * 2,
                 SK_Scalar1*3, SK_Scalar1 * 2,
                 SK_Scalar1*4, SK_Scalar1 * 2);
    meas.setPath(&path, false);
    length = meas.getLength();
    REPORTER_ASSERT(reporter, length == SK_Scalar1 * 6);
    REPORTER_ASSERT(reporter, meas.getPosTan(SK_ScalarHalf, &position, &tangent));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fX,
                            SK_ScalarHalf,
                            0.0001f));
    REPORTER_ASSERT(reporter, position.fY == 0);
    REPORTER_ASSERT(reporter, tangent.fX == SK_Scalar1);
    REPORTER_ASSERT(reporter, tangent.fY == 0);
    REPORTER_ASSERT(reporter, meas.getPosTan(2.5f, &position, &tangent));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fX, SK_Scalar1, 0.0001f));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fY, 1.5f));
    REPORTER_ASSERT(reporter, tangent.fX == 0);
    REPORTER_ASSERT(reporter, tangent.fY == SK_Scalar1);
    REPORTER_ASSERT(reporter, meas.getPosTan(4.5f, &position, &tangent));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fX,
                            2.5f,
                            0.0001f));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fY,
                            2.0f,
                            0.0001f));
    REPORTER_ASSERT(reporter, tangent.fX == SK_Scalar1);
    REPORTER_ASSERT(reporter, tangent.fY == 0);

    path.reset();
    path.moveTo(0, 0);
    path.lineTo(SK_Scalar1, 0);
    path.moveTo(SK_Scalar1, SK_Scalar1);
    path.moveTo(SK_Scalar1 * 2, SK_Scalar1 * 2);
    path.lineTo(SK_Scalar1, SK_Scalar1 * 2);
    meas.setPath(&path, false);
    length = meas.getLength();
    REPORTER_ASSERT(reporter, length == SK_Scalar1);
    REPORTER_ASSERT(reporter, meas.getPosTan(SK_ScalarHalf, &position, &tangent));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fX,
                            SK_ScalarHalf,
                            0.0001f));
    REPORTER_ASSERT(reporter, position.fY == 0);
    REPORTER_ASSERT(reporter, tangent.fX == SK_Scalar1);
    REPORTER_ASSERT(reporter, tangent.fY == 0);
    meas.nextContour();
    length = meas.getLength();
    REPORTER_ASSERT(reporter, length == SK_Scalar1);
    REPORTER_ASSERT(reporter, meas.getPosTan(SK_ScalarHalf, &position, &tangent));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fX,
                            1.5f,
                            0.0001f));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fY,
                            2.0f,
                            0.0001f));
    REPORTER_ASSERT(reporter, tangent.fX == -SK_Scalar1);
    REPORTER_ASSERT(reporter, tangent.fY == 0);

    test_small_segment();
    test_small_segment2();
    test_small_segment3();
}
