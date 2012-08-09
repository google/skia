#include "CubicIntersection_TestData.h"
#include "Intersection_Tests.h"

void cubecode_test(int test);

#define TEST_QUADS_FIRST 0

void Intersection_Tests() {
    SimplifyNew_Test();
    Simplify4x4QuadralateralsThreaded_Test();
    Simplify4x4QuadraticsThreaded_Test();
    Simplify4x4RectsThreaded_Test();
    SimplifyNondegenerate4x4TrianglesThreaded_Test();
    SimplifyDegenerate4x4TrianglesThreaded_Test();
    SimplifyFindNext_Test();
    SimplifyFindTop_Test();
    SimplifyAngle_Test();
    QuadraticReduceOrder_Test();
    QuadraticBezierClip_Test();
    QuadraticIntersection_Test();
    SimplifyAddIntersectingTs_Test();
    
    cubecode_test(1);
    convert_testx();
    // tests are in dependency / complexity order
    Inline_Tests();
    ConvexHull_Test();
    ConvexHull_X_Test();

    LineParameter_Test();
    LineIntersection_Test();
    LineQuadraticIntersection_Test();
    LineCubicIntersection_Test();

    SimplifyQuadraticPaths_Test();

    SimplifyPolygonPaths_Test();
    SimplifyRectangularPaths_Test();
    SimplifyQuadralateralPaths_Test();

    ActiveEdge_Test();

    QuadraticCoincidence_Test();
    QuadraticIntersection_Test();

    CubicParameterization_Test();
    CubicCoincidence_Test();
    CubicReduceOrder_Test();
    CubicBezierClip_Test();
    CubicIntersection_Test();

}
