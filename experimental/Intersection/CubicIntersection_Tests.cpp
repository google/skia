#include "CubicIntersection_Tests.h"

void CubicIntersection_Tests() {
    // tests are in dependency order
    Inline_Tests();
    ConvexHull_Test();
    ConvexHull_X_Test();
    LineParameter_Test();
    LineIntersection_Test();
    QuadraticCoincidence_Test();
    CubicCoincidence_Test();
    ReduceOrder_Test();
 //   BezierClip_Test();
    CubicIntersection_Test();
}
