#include "Test.h"
#include "SkGeometry.h"

static void TestGeometry(skiatest::Reporter* reporter) {
    SkPoint pts[3], dst[5];

    pts[0].set(0, 0);
    pts[1].set(100, 50);
    pts[2].set(0, 100);

    int count = SkChopQuadAtMaxCurvature(pts, dst);
    REPORTER_ASSERT(reporter, count == 1 || count == 2);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Geometry", GeometryTestClass, TestGeometry)
