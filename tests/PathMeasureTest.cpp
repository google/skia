/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkContourMeasure.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "src/core/SkPathPriv.h"
#include "tests/Test.h"

#include <array>
#include <cstddef>
#include <initializer_list>
#include <utility>

static void test_small_segment3() {
    SkPath path;
    const SkPoint pts[] = {
        { 0, 0 },
        { 100000000000.0f, 100000000000.0f }, { 0, 0 }, { 10, 10 },
        { 10, 10 }, { 0, 0 }, { 10, 10 }
    };

    path.moveTo(pts[0]);
    for (size_t i = 1; i < std::size(pts); i += 3) {
        path.cubicTo(pts[i], pts[i + 1], pts[i + 2]);
    }

    SkPathMeasure meas(path, false);
    meas.getLength();
}

static void test_small_segment2() {
    SkPath path;
    const SkPoint pts[] = {
        { 0, 0 },
        { 100000000000.0f, 100000000000.0f }, { 0, 0 },
        { 10, 10 }, { 0, 0 },
    };

    path.moveTo(pts[0]);
    for (size_t i = 1; i < std::size(pts); i += 2) {
        path.quadTo(pts[i], pts[i + 1]);
    }
    SkPathMeasure meas(path, false);
    meas.getLength();
}

static void test_small_segment() {
    SkPath path;
    const SkPoint pts[] = {
        { 100000, 100000},
        // big jump between these points, makes a big segment
        { 1.0005f, 0.9999f },
        // tiny (non-zero) jump between these points
        { SK_Scalar1, SK_Scalar1 },
    };

    path.moveTo(pts[0]);
    for (size_t i = 1; i < std::size(pts); ++i) {
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

    // SkPathMeasure isn't copyable, but it should be move-able
    SkPathMeasure meas2(std::move(meas));
    meas = std::move(meas2);
}

DEF_TEST(PathMeasureConic, reporter) {
    SkPoint stdP, hiP, pts[] = {{0,0}, {100,0}, {100,0}};
    SkPath p;
    p.moveTo(0, 0);
    p.conicTo(pts[1], pts[2], 1);
    SkPathMeasure stdm(p, false);
    REPORTER_ASSERT(reporter, stdm.getPosTan(20, &stdP, nullptr));
    p.reset();
    p.moveTo(0, 0);
    p.conicTo(pts[1], pts[2], 10);
    stdm.setPath(&p, false);
    REPORTER_ASSERT(reporter, stdm.getPosTan(20, &hiP, nullptr));
    REPORTER_ASSERT(reporter, 19.5f < stdP.fX && stdP.fX < 20.5f);
    REPORTER_ASSERT(reporter, 19.5f < hiP.fX && hiP.fX < 20.5f);
}

// Regression test for b/26425223
DEF_TEST(PathMeasure_nextctr, reporter) {
    SkPath path;
    path.moveTo(0, 0); path.lineTo(100, 0);

    SkPathMeasure meas(path, false);
    // only expect 1 contour, even if we didn't explicitly call getLength() ourselves
    REPORTER_ASSERT(reporter, !meas.nextContour());
}

static void test_90_degrees(const sk_sp<SkContourMeasure>& cm, SkScalar radius,
                            skiatest::Reporter* reporter) {
    SkPoint pos;
    SkVector tan;
    SkScalar distance = cm->length() / 4;
    bool success = cm->getPosTan(distance, &pos, &tan);

    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(pos.fX, 0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(pos.fY, radius));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(tan.fX, -1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(tan.fY, 0));
}

static void test_empty_contours(skiatest::Reporter* reporter) {
    SkPath path;

    path.moveTo(0, 0).lineTo(100, 100).lineTo(200, 100);
    path.moveTo(2, 2).moveTo(3, 3);                 // zero-length(s)
    path.moveTo(4, 4).close().close().close();      // zero-length
    path.moveTo(5, 5).lineTo(5, 5);                 // zero-length
    path.moveTo(5, 5).lineTo(5, 5).close();         // zero-length
    path.moveTo(5, 5).lineTo(5, 5).close().close(); // zero-length
    path.moveTo(6, 6).lineTo(7, 7);
    path.moveTo(10, 10);                            // zero-length

    SkContourMeasureIter fact(path, false);

    // given the above construction, we expect only 2 contours (the rest are "empty")

    REPORTER_ASSERT(reporter, fact.next());
    REPORTER_ASSERT(reporter, fact.next());
    REPORTER_ASSERT(reporter, !fact.next());
}

static void test_MLM_contours(skiatest::Reporter* reporter) {
    SkPath path;

    // This odd sequence (with a trailing moveTo) used to return a 2nd contour, which is
    // wrong, since the contract for a measure is to only return non-zero length contours.
    path.moveTo(10, 10).lineTo(20, 20).moveTo(30, 30);

    for (bool forceClosed : {false, true}) {
        SkContourMeasureIter fact(path, forceClosed);
        REPORTER_ASSERT(reporter, fact.next());
        REPORTER_ASSERT(reporter, !fact.next());
    }
}

static void test_shrink(skiatest::Reporter* reporter) {
    SkPath path;
    path.addRect({1, 2, 3, 4});
    path.incReserve(100);   // give shrinkToFit() something to do

    SkContourMeasureIter iter(path, false);

    // shrinks the allocation, possibly relocating the underlying arrays.
    // The contouremasureiter needs to have safely copied path, to be unaffected by this
    // change to "path".
    SkPathPriv::ShrinkToFit(&path);

    // Note, this failed (before the fix) on an ASAN build, which notices that we were
    // using an internal iterator of the passed-in path, not our copy.
    while (iter.next())
        ;
}

DEF_TEST(contour_measure, reporter) {
    SkPath path;
    path.addCircle(0, 0, 100);
    path.addCircle(0, 0, 10);

    SkContourMeasureIter fact(path, false);
    path.reset();   // we should not need the path avert we created the factory

    auto cm0 = fact.next();
    auto cm1 = fact.next();

    REPORTER_ASSERT(reporter, cm0->isClosed());
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(cm0->length(), 200 * SK_ScalarPI, 1.5f));

    test_90_degrees(cm0, 100, reporter);

    REPORTER_ASSERT(reporter, cm1->isClosed());
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(cm1->length(), 20 * SK_ScalarPI, 0.5f));

    test_90_degrees(cm1, 10, reporter);

    auto cm2 = fact.next();
    REPORTER_ASSERT(reporter, !cm2);

    test_empty_contours(reporter);
    test_MLM_contours(reporter);

    test_shrink(reporter);
}
