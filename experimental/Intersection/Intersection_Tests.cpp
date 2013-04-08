/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CubicIntersection_TestData.h"
#include "Intersection_Tests.h"

void cubecode_test(int test);
void parseSVG();

#define TEST_QUADS_FIRST 0

void Intersection_Tests() {
    int testsRun = 0;
#if 0
    CubicIntersection_OneOffTest();
    CubicIntersection_SelfTest();
    QuadraticIntersection_IntersectionFinder();
    QuadraticIntersection_OneOffTest();
    CubicIntersection_IntersectionFinder();
    CubicUtilities_Test();
#endif
    SimplifyNew_Test();
    CubicsToQuadratics_OneOffTest();
    CubicIntersection_OneOffTest();
//    CubicIntersection_OneOffTests();
  #if 0
    parseSVG();
  #endif
//    QuadraticIntersection_PointFinder();
    ShapeOps4x4CubicsThreaded_Test(testsRun);
    CubicToQuadratics_Test();
    QuadraticIntersection_Test();
    QuarticRoot_Test();
    CubicIntersection_RandTest();
    CubicsToQuadratics_RandTest();
    Simplify4x4RectsThreaded_Test(testsRun);
    Simplify4x4QuadraticsThreaded_Test(testsRun);
    QuadLineIntersectThreaded_Test(testsRun);
    SimplifyNondegenerate4x4TrianglesThreaded_Test(testsRun);
    SimplifyDegenerate4x4TrianglesThreaded_Test(testsRun);
    Simplify4x4QuadralateralsThreaded_Test(testsRun);
    ShapeOps4x4RectsThreaded_Test(testsRun);
    SkDebugf("%s total testsRun=%d\n", __FUNCTION__, testsRun);
    LineQuadraticIntersection_Test();
    MiniSimplify_Test();
    SimplifyAngle_Test();
    QuadraticBezierClip_Test();
    SimplifyFindNext_Test();
    SimplifyFindTop_Test();
    QuadraticReduceOrder_Test();
    SimplifyAddIntersectingTs_Test();

    cubecode_test(1);
    convert_testx();
    // tests are in dependency / complexity order
    Inline_Tests();
    ConvexHull_Test();
    ConvexHull_X_Test();

    LineParameter_Test();
    LineIntersection_Test();
    LineCubicIntersection_Test();

    SimplifyQuadraticPaths_Test();

    SimplifyPolygonPaths_Test();
    SimplifyRectangularPaths_Test();
    SimplifyQuadralateralPaths_Test();

  //  ActiveEdge_Test();

    QuadraticCoincidence_Test();
    QuadraticIntersection_Test();

    CubicParameterization_Test();
    CubicCoincidence_Test();
    CubicReduceOrder_Test();
    CubicBezierClip_Test();
    CubicIntersection_Test();

}
