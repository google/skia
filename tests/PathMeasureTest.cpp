
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkPathMeasure.h"

static void TestPathMeasure(skiatest::Reporter* reporter) {
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

    for (int i = 0; i < 8; i++) {
        SkScalar    d = length * i / 8;
        SkPoint     p;
        SkVector    v;
        meas.getPosTan(d, &p, &v);
#if 0
        SkDebugf("circle arc-length=%g, pos[%g %g] tan[%g %g]\n",
                 d, p.fX, p.fY, v.fX, v.fY);
#endif
    }

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
        SkScalarNearlyEqual(position.fX, -SK_ScalarHalf, SK_Scalar1 * 0.0001));
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
        SkScalarNearlyEqual(position.fX, SK_ScalarHalf, SK_Scalar1 * 0.0001));
    REPORTER_ASSERT(reporter, position.fY == 0);
    REPORTER_ASSERT(reporter, tangent.fX == SK_Scalar1);
    REPORTER_ASSERT(reporter, tangent.fY == 0);
    REPORTER_ASSERT(reporter, meas.getPosTan(SK_Scalar1 * 2.5f, &position, &tangent));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fX, SK_Scalar1, SK_Scalar1 * 0.0001));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fY, SK_Scalar1 * 1.5f));
    REPORTER_ASSERT(reporter, tangent.fX == 0);
    REPORTER_ASSERT(reporter, tangent.fY == SK_Scalar1);
    REPORTER_ASSERT(reporter, meas.getPosTan(SK_Scalar1 * 4.5f, &position, &tangent));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fX, SK_Scalar1 * 2.5f, SK_Scalar1 * 0.0001));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fY, SK_Scalar1 * 2.0f, SK_Scalar1 * 0.0001));
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
        SkScalarNearlyEqual(position.fX, SK_ScalarHalf, SK_Scalar1 * 0.0001));
    REPORTER_ASSERT(reporter, position.fY == 0);
    REPORTER_ASSERT(reporter, tangent.fX == SK_Scalar1);
    REPORTER_ASSERT(reporter, tangent.fY == 0);
    meas.nextContour();
    length = meas.getLength();
    REPORTER_ASSERT(reporter, length == SK_Scalar1);
    REPORTER_ASSERT(reporter, meas.getPosTan(SK_ScalarHalf, &position, &tangent));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fX, SK_Scalar1 * 1.5f, SK_Scalar1 * 0.0001));
    REPORTER_ASSERT(reporter,
        SkScalarNearlyEqual(position.fY, SK_Scalar1 * 2.0f, SK_Scalar1 * 0.0001));
    REPORTER_ASSERT(reporter, tangent.fX == -SK_Scalar1);
    REPORTER_ASSERT(reporter, tangent.fY == 0);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PathMeasure", PathMeasureTestClass, TestPathMeasure)
